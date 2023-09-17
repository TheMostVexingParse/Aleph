#include "board.h"
#include "search.h"
#include "moveordering.h"
#include "parameters.h"

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
    uint32_t fprunedNodes = 0;

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

        HASH_HIST.arghash[HISTORY.cmove+1] = board.hash;
        HISTORY.cmove++;

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

        int repetitions = 0;
        for (int j = 0; j < 256; j++) {
            repetitions += HASH_HIST.arghash[j] == board.hash;
            if (repetitions > 2) {
                return 0;
            }
        }

        HASH_HIST.arghash[HISTORY.cmove+pdepth+1] = board.hash;

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

        if (depth <= 0) {
            pline->cmove = 0;
            return quiescenceSearch(board, alpha, beta, color, pdepth+1, movetime);
        }

        int root_extensions = 0;
        int root_reductions = 0;

        bool isInCheck = false;

        int pieceCount = bitcount(board.original_bitboards[1] | board.original_bitboards[2] | board.original_bitboards[3] | board.original_bitboards[4] |
                                  board.original_bitboards[7] | board.original_bitboards[8] | board.original_bitboards[9] | board.original_bitboards[10] );

        if (board.isInCheck()){
            isInCheck = true;
            root_extensions++;
        }

        // AEL - Pruning beginning

        // int fmax = 0;
        // int fscore = 0;
        // int fpruned_moves = 0;

        bool isPVNode = beta - alpha != 1;

        // bool fprune = false;

        // int material_balance = board.matBalance();

        // fscore = material_balance + razoringMargin; // razoring at pre pre frontier nodes
        // if (!root_extensions && (depth == PRE_PRE_FRONTIER_NODE) && (fscore <= alpha)) {
        //     fprune = true;
        //     fmax = fscore;
        // }
        // fscore = material_balance + extendedFutilityMargin; // extended futility pruining at pre frontier nodes
        // if (!root_extensions && (depth == PRE_FRONTIER_NODE) && (fscore <= alpha)) {
        //     fprune = true;
        //     fmax = fscore;
        // }
        // fscore = material_balance + futilityMargin; // futility pruining at frontier nodes
        // if (!root_extensions && (depth == FRONTIER_NODE) && (fscore <= alpha)) {
        //     fprune = true;
        //     fmax = fscore;
        // }

        // if (!isInCheck && abs(beta) < 70000) {
        //     int static_score = board.getScore();
        //     int score_margin = staticNullMovePruningBaseMargin * depth;
        //     if (static_score - score_margin >= beta) {
        //         return static_score - score_margin;
        //     }
        // }

        // // if depth <= 2 && !isPVNode && !inCheck {
        // //     staticScore := EvaluatePos(&search.Pos)
        // //     if staticScore+FutilityMargins[depth]*3 < alpha {
        // //         score := search.Qsearch(alpha, beta, ply, &PVLine{}, 0)
        // //         if score < alpha {
        // //             return alpha
        // //         }
        // //     }
        // // }

        // if (depth <= PRE_PRE_FRONTIER_NODE && !isInCheck && !isPVNode) {
        //     int static_score = board.getScore();
        //     if (50 + static_score + futilityMargins[depth] * 3 < alpha) {
        //         int qscore = quiescenceSearch(board, alpha-1, alpha, color, pdepth+1, movetime);
        //         if (qscore < alpha)
        //             return alpha;
        //     }
        // }

        if (doNull && !isInCheck && depth >= 3 && pieceCount > 0 && bitcount(~board.pieces[0]) > 14){
            // int reduction = 2;
            int reduction = 2 + depth / 6;
            // if (depth <= 6 || (depth <= 8 && pieceCount < 5)) reduction = 2;
            // else if ( depth > 8 || (depth > 6 && pieceCount >= 5)) reduction = 3;
            Board copy(board);
            copy.makeNullMove();
            LINE discard_line;
            int score = -alphaBeta(&discard_line, copy, std::max(depth - 1 - reduction + root_extensions - root_reductions, 1), -beta, 1-beta, -color, pdepth+1, movetime, false, false); // -AlphaBeta (0-beta, 1-beta, depth-R-1)
            if (score >= beta ) return beta;

        } 
        else if (!doNull) {
            doNull = true;
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

        for (int i = 0; i < 256; i++) {
            if (stop) return 0;
            uint16_t move = moves[i];
            if (move == 0) break;
            int score;
            Board copy(board);
            copy.makeMove((move >> 6) & 0b111111, move & 0b111111);

            // if (fprune && (fmax + board.matGain(move) <= alpha) && !copy.isInCheck()) {
            //     // std::cout << "Pruned: " << board.matGain(move) << "     Alpha: " << alpha << "     Fmax: " << fmax << "\n";
            //     continue;
            // }
            
            Board nullCheck(copy); // it is somehow faster ??? wtf ???
            nullCheck.makeNullMove();
            if (nullCheck.isInCheck()) continue;

            legalMoves++;

            bool do_full_search = false;
            
            if (i > 12 && depth >= 3 && !isInCheck && !board.isCapture((move >> 6) & 0b111111, move & 0b111111)){
                constexpr int lmr = 1;
                score = -alphaBeta(&line, copy, depth - 1 - lmr, -alpha-1, -alpha, -color, pdepth+1, movetime, doNull);
                if (score > alpha)
                    do_full_search = true;
            } else {
                do_full_search = true;
            }

            if (do_full_search) {
                score = -alphaBeta(&line, copy, depth - 1, -beta, -alpha, -color, pdepth+1, movetime, doNull);
            }

            if (stop) return 0;

            if (score > bestScore){
                bestScore = score;
                bestMove = move;
            }

            if (score >= beta) {
                delete[] moves;
                int fromsq = (move >> 6) & 0b111111;
                int tosq = move & 0b111111;
                int prevfrsq = (board.last_move >> 6) & 0b111111;
                int prevtosq = board.last_move & 0b111111;
                if (!board.isCapture(fromsq, tosq)){
                    COUNTER_MOVE_HEURISTICS[prevfrsq][prevtosq] = move;
                    KILLER_MOVES[1][depth] = KILLER_MOVES[0][depth];
                    KILLER_MOVES[0][depth] = move;
                }
                TT.add_position(board, bestMove, depth, bestScore, LOWERBOUND);
                return beta;
            }

            if (score > alpha) {
                alpha = score;
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
        return bestScore;

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

        // int fmax = 0;
        // int fscore = 0;
        // int fpruned_moves = 0;

        // bool fprune = 0;

        // int material_balance = board.matBalance();

        // fscore = material_balance + razoringMargin; // razoring at pre pre frontier nodes
        // if ((depth == PRE_PRE_FRONTIER_NODE) && (fscore <= alpha)) {
        //     fprune = true;
        //     fmax = fscore;
        // }
        // fscore = material_balance + extendedFutilityMargin; // extended futility pruining at pre frontier nodes
        // if ((depth == PRE_FRONTIER_NODE) && (fscore <= alpha)) {
        //     fprune = true;
        //     fmax = fscore;
        // }
        // fscore = material_balance + futilityMargin; // futility pruining at frontier nodes
        // if ((depth == FRONTIER_NODE) && (fscore <= alpha)) {
        //     fprune = true;
        //     fmax = fscore;
        // }

        // bool check = board.isInCheck();

        const auto moves = board.generateQuiescence();

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

            // if (fprune && (fmax + board.matGain(move) <= alpha)) {
            //     fprunedNodes++;
            //     continue;
            // }

            if (board.isCapture((move >> 6) & 0b111111, move & 0b111111)) { //|| copy.isInCheck()) {

                int see_score = board.see(move);

                if (see_score < 0) continue;

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