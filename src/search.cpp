#include "board.h"
#include "search.h"
#include "moveordering.h"

std::atomic_bool search_complete = { true };

int LMR_MOVES = 32;
int LMR_DEPTH = 5;

class Search {
public:

    std::chrono::high_resolution_clock::time_point start; 

    int go_depth;
    int max_depth;

    int passed_depth = 0;
    uint32_t searchedPositions = 0;
    uint32_t ttHits = 0;

    LINE PVline;

    int root_search(Board board, int depth, int alpha, int beta, int color, int movetime) {
        start = std::chrono::high_resolution_clock::now();
        LINE line;
        LINE discarded_line;
        search_complete = false;
        go_depth = depth;
        max_depth = 0;
        passed_depth = 1;
        board.halfmove_clock = 0;
        int alphaOriginal = alpha;
        int bestScore = INT_MIN;
        auto moves = board.generateMoves(depth, 0);
        uint16_t bestMove = moves[0];
        if (!bestMove && !board.isInCheck()){
            return 0;
        }
        // bool bSearchPv = true;
        for (int i = 0; i < 256; i++) {
            if (stop) {
                delete[] moves;
                return INT_MAX;
            }
            uint16_t move = moves[i];
            if (move == 0) break;
            Board copy(board);
            copy.makeMove((move >> 6) & 0b111111, move & 0b111111);
            Board nullCheck(copy);
            nullCheck.makeNullMove();
            if (nullCheck.isInCheck()) continue;
            int score = -alphaBeta(&line, copy, depth - 1, -beta, -alpha, -color, 1, movetime, true);
            // else {
            //     score = -alphaBeta(&line, copy, depth - 1, -alpha-1, -alpha, -color, 1, movetime, true);
            //     if (score > alpha && score < beta) { 
            //         score = -alphaBeta(&line, copy, depth - 1, -beta, -alpha, -color, 1, movetime, true);

            //     }
            // }

            if (stop) {
                delete[] moves;
                return INT_MAX;
            }
            if (score > bestScore){
                bestScore = score;
                bestMove = move;
            }

            if (score > alpha) {
                alpha = score;
                // bSearchPv = false;

                PV_TABLE[0][0] = bestMove;
                for (int next_depth = 1; next_depth < PV_LENGTH[0]+1; next_depth++) {
                    PV_TABLE[0][next_depth] = PV_TABLE[1][next_depth];
                } PV_LENGTH[0] = PV_LENGTH[1];

                PVline.argmove[0] = bestMove;
                memcpy(PVline.argmove + 1, line.argmove, line.cmove * 2);
                PVline.cmove = line.cmove + 1;

                if (!board.isCapture((move >> 6) & 0b111111, move & 0b111111)){
                    HISTORY_HEURISTICS[(move >> 6) & 0b111111][move & 0b111111] += depth;
                }
                if (score >= beta) {
                    delete[] moves;
                    if (!board.isCapture((move >> 6) & 0b111111, move & 0b111111)){
                        KILLER_MOVES[1][depth] = KILLER_MOVES[0][depth];
                        KILLER_MOVES[0][depth] = move;
                    }
                    TT.add_position(board, bestMove, depth, bestScore, LOWERBOUND);
                    searchedPositions += ttHits;
                    search_complete = true;
                    return beta;
                }
            }
        }
        delete[] moves;
        if (bestScore <= alphaOriginal) {
            TT.add_position(board, bestMove, depth, bestScore, UPPERBOUND);
        } else {
            TT.add_position(board, bestMove, depth, bestScore, EXACT);
        }
        searchedPositions += ttHits;
        search_complete = true;
        return bestScore;
    }

    int alphaBeta(LINE * pline, Board board, int depth, int alpha, int beta, int color, int pdepth, int movetime, bool doNull = false, bool collectPV = true) {
        LINE line;
        LINE discarded_line;
        PV_LENGTH[pdepth] = pdepth;
        if (!board.original_bitboards[5] || !board.original_bitboards[11]) {
            if (board.side_to_move == WHITE) {
                if (!board.original_bitboards[5]) {
                    return -99000 + pdepth*1000;
                } else {
                    return 99000 - pdepth*1000;
                }
            } else {
                if (!board.original_bitboards[11]) {
                    return -99000 + pdepth*1000;
                } else {
                    return 99000 - pdepth*1000;
                }
            }
        }
        uint64_t entry_key = board.hash % table_size;
        if (TT.contains_entry(entry_key, board.hash, depth)){
            ttHits++;
            int ttScore = TT.score[entry_key];
            switch(TT.type[entry_key]){
                case EXACT:
                    return ttScore;
                    break;
                case UPPERBOUND:
                    if (ttScore <= alpha) { return ttScore; }
                    break;
                case LOWERBOUND:
                    if (ttScore >= beta) { return ttScore; }
                    break;
                default:
                    ttHits--;
                    break;
            }                
        }

        int root_extensions = 0;
        int root_reductions = 0;

        bool isInCheck = false;

        if (board.isInCheck()){
            isInCheck = true;
            root_extensions++;
        }
        // else if (doNull && depth > 3 && bitcount(~board.pieces[0]) > 14){
        //     int reduction = depth > 6 ? 3 : 2;
        //     Board copy(board);
        //     copy.makeNullMove();
        //     LINE discard_line;
        //     int score = -alphaBeta(&discard_line, copy, std::max(depth - 1 - reduction, 1), -beta, 1-beta, -color, pdepth+1, movetime, false, false); // -AlphaBeta (0-beta, 1-beta, depth-R-1)
        //     if (score >= beta ) {
        //         return beta;
        //         // depth = std::max(1, depth-3);
        //         // if ( depth <= 0 ){
        //         //     pline->cmove = 0;
        //         //     return quiescenceSearch(board, alpha, beta, color, pdepth+1, movetime);
        //         // }
        //     }
        // }

        if (depth <= 0) {
            pline->cmove = 0;
            return quiescenceSearch(board, alpha, beta, color, pdepth+1, movetime);
        }

        int alphaOriginal = alpha;
        int bestScore = INT_MIN;

        auto moves = board.generateMoves(depth, pdepth);
        uint16_t bestMove = moves[0];

        if (!bestMove){
            if (!isInCheck) {
                delete[] moves;
                return 0;
            } else {
                delete[] moves;
                return - 99000 + pdepth*1000;
            }
        }

        int legalMoves = 0;

        // bool bSearchPv = true;

        for (int i = 0; i < 256; i++) {
            if (stop) return 0;
            uint16_t move = moves[i];
            if (move == 0) break;
            int score;
            Board copy(board);
            copy.makeMove((move >> 6) & 0b111111, move & 0b111111);
            Board nullCheck(copy);
            nullCheck.makeNullMove();
            if (nullCheck.isInCheck()) continue;
            legalMoves++;

            bool do_full_search = false;

            int repetitions = 0;
            for (int j = 0; j < 256; j++) {
                repetitions += HASH_HIST.arghash[j] == board.hash;
                if (repetitions >= 2) {
                    score = 0;
                    goto repetition;
                }
            }

            HASH_HIST.arghash[HISTORY.cmove+pdepth+1] = board.hash;
            
            
            // score = -alphaBeta(&line, copy, depth - 1 - root_reductions + root_extensions, -beta, -alpha, -color, pdepth+1, movetime, doNull);
            
            if (i > 12 && depth >= 3 && !board.isCapture((move >> 6) & 0b111111, move & 0b111111) && !board.isInCheck()){
                const int lmr = 1;
                score = -alphaBeta(&line, copy, depth - 1 - lmr, -alpha-1, -alpha, -color, pdepth+1, movetime, doNull);
                if (score > alpha)
                    do_full_search = true;
            } else {
                do_full_search = true;
            }

            if (do_full_search) {
                score = -alphaBeta(&line, copy, depth - 1, -beta, -alpha, -color, pdepth+1, movetime, doNull);
            }

            repetition:

            if (stop) return 0;

            if (score > bestScore){
                bestScore = score;
                bestMove = move;
            }

            if (score >= beta) {
                delete[] moves;
                if (!board.isCapture((move >> 6) & 0b111111, move & 0b111111)){
                    KILLER_MOVES[1][depth] = KILLER_MOVES[0][depth];
                    KILLER_MOVES[0][depth] = move;
                }
                TT.add_position(board, bestMove, depth, bestScore, LOWERBOUND);
                return beta;
            }

            if (score > alpha) {
                alpha = score;
                // bSearchPv = false;

                PV_TABLE[pdepth][pdepth] = move;
                for (int next_depth = pdepth+1; next_depth < PV_LENGTH[pdepth]+1; next_depth++) {
                    PV_TABLE[pdepth][next_depth] = PV_TABLE[pdepth+1][next_depth];
                } PV_LENGTH[pdepth] = PV_LENGTH[pdepth+1];

                pline->argmove[0] = move;
                memcpy(pline->argmove + 1, line.argmove, line.cmove * 2);
                pline->cmove = line.cmove + 1;

                if (!board.isCapture((move >> 6) & 0b111111, move & 0b111111)){
                    HISTORY_HEURISTICS[(move >> 6) & 0b111111][move & 0b111111] += depth;
                }
            }

        }
        if (!legalMoves){
            if (!isInCheck) {
                delete[] moves;
                return 0;
            } else {
                delete[] moves;
                return - 99000 + pdepth*1000;
            }
        }
        if (bestScore <= alphaOriginal) {
            TT.add_position(board, bestMove, depth, bestScore, UPPERBOUND);
        } else {
            TT.add_position(board, bestMove, depth, bestScore, EXACT);
        }
        delete[] moves;
        return alpha;

    }

    int quiescenceSearch(Board board, int alpha, int beta, int color, int pdepth, int movetime, int depth=0) {
        if (!board.original_bitboards[5] || !board.original_bitboards[11]) {
            if (board.side_to_move == WHITE) {
                if (!board.original_bitboards[5]) {
                    return -99000 + pdepth*1000;
                } else {
                    return 99000 - pdepth*1000;
                }
            } else {
                if (!board.original_bitboards[11]) {
                    return -99000 + pdepth*1000;
                } else {
                    return 99000 - pdepth*1000;
                }
            }
        }

        max_depth = __max(go_depth+depth, max_depth);
        int stand_pat = color * board.getScore();

        if(!(searchedPositions & 4095)) {
            int ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
            if (ms > movetime) {
                stop = true;
            }
        }

        if (stand_pat >= beta) {
            searchedPositions++;
            return beta;
        }

        int BIG_DELTA = 900;

        if ( stand_pat < alpha - BIG_DELTA ) {
            return alpha;
        }

        if (alpha < stand_pat) {
            alpha = stand_pat;
        }

        bool check = board.isInCheck();

        const auto moves = board.generateMoves(depth, pdepth);

        if (!moves[0]){
            if (!board.isInCheck()) {
                delete[] moves;
                return 0;
            }
            else {
                delete[] moves;
                return - 99000 + pdepth*1000;
            }
        }

        for (int i = 0; i < 256; i++) {
            uint16_t move = moves[i];
            if (stop || move == 0) break;

            Board copy(board);
            copy.makeMove((move >> 6) & 0b111111, move & 0b111111);

            if (board.isCapture((move >> 6) & 0b111111, move & 0b111111) || copy.isInCheck()) {

                int score = -quiescenceSearch(copy, -beta, -alpha, -color, pdepth+1, movetime, depth+1);

                if (score > alpha) {
                    if (score >= beta) {
                        searchedPositions++;
                        delete[] moves;
                        return beta;
                    }
                    alpha = score;
                }
            }
        }
        searchedPositions++;
        delete[] moves;
        return alpha;
    }
};