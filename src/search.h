#pragma once

#include "board.h"

#define EXACT       1
#define LOWERBOUND  2
#define UPPERBOUND  3
#define EMPTY_MOVE  0

typedef struct LINE {
    int cmove = 0;             
    uint16_t argmove[64] = {0};
}   LINE;

typedef struct HISTORY_LINE {
    int cmove = 0;             
    uint16_t argmove[256] = {0};
}   HISTORY_LINE;

typedef struct HASH_HISTORY {
    int cmove = 0;             
    uint64_t arghash[256] = {0};
}   HASH_HISTORY;


class Perft{
    public:
    uint32_t total_nodes = 0;
    void go(Board board, int depth){
        auto moves = board.generateMovesWithoutSorting();
        for (int i = 0; i < 256; i++) {
            uint16_t move = moves[i];
            if (move == 0) continue;
            Board copy(board);
            copy.makeMove((move >> 6) & 0b111111, move & 0b111111);
            copy.makeNullMove();
            if (copy.isInCheck()) continue;
            copy.makeNullMove();
            uint32_t movec = traverse_root_trees(copy, depth-1);
            total_nodes += movec;
            std::cout << moveNotation((move >> 6) & 0b111111, move & 0b111111, board) << "   " << movec << std::endl;
        }
        delete [] moves;
        std::cout << std::endl << "Total: " << total_nodes << std::endl;
    }
    int traverse_root_trees(Board & board, int depth){
        // if (!depth) return 1;
        uint32_t collected = 0;
        auto moves = board.generateMovesWithoutSorting();
        for (int i = 0; i < 256; i++) {
            uint16_t move = moves[i];
            if (move == 0) break;
            Board copy(board);
            copy.makeMove((move >> 6) & 0b111111, move & 0b111111);
            copy.makeNullMove();
            if (copy.isInCheck()) continue;
            copy.makeNullMove();
            if (depth == 1)
                collected++;
            else
                collected += traverse_root_trees(copy, depth-1);
        }
        delete [] moves;
        return collected;
    }
};

class HASH_TABLE {
    public:
    uint32_t*   chsums  = new uint32_t[table_size];
    uint32_t*   hashes  = new uint32_t[table_size]; 
    uint16_t*   moves   = new uint16_t[table_size]; 
    int*        depth   = new int[table_size];      
    int*        score   = new int[table_size];      
    char*       type    = new char[table_size];     

    HASH_TABLE(){ 
        memset(chsums, 0, table_size * sizeof(uint32_t));
        memset(hashes, 0, table_size * sizeof(uint32_t));
        memset(moves,  0, table_size * sizeof(uint16_t));
        memset(depth,  0, table_size * sizeof(int));
        memset(score,  0, table_size * sizeof(int));
        memset(type,   0, table_size * sizeof(char));
    }

    void flush(){
        memset(chsums, 0, table_size * sizeof(uint32_t));
        memset(hashes, 0, table_size * sizeof(uint32_t));
        memset(moves,  0, table_size * sizeof(uint16_t));
        memset(depth,  0, table_size * sizeof(int));
        memset(score,  0, table_size * sizeof(int));
        memset(type,   0, table_size * sizeof(char));
    }

    void add_position(Board board, uint16_t move, int depth, int score, char type){
        uint32_t entry = board.hash % table_size;
        uint32_t chsum = entry ^ move ^ depth ^ score;
        if (this->type[entry]==EXACT && type!=EXACT) return;
        this->chsums[entry] = chsum;
        this->hashes[entry] = (uint32_t)(board.hash >> 32);
        this->moves[entry] = move;
        this->depth[entry] = depth;
        this->score[entry] = score;
        this->type[entry] = type;
    }

    bool contains_entry(uint32_t entry, uint64_t hash, int depth){

        if (this->depth[entry] < depth) return false;
        if ((entry ^ this->moves[entry] ^ this->depth[entry] ^ this->score[entry]) != this->chsums[entry]) return false;
        if (this->hashes[entry] == (uint32_t)(hash >> 32)) return true;
        else return false;
    }

    bool contains_entry_with_move(uint32_t entry, uint64_t hash, uint16_t move){
        if ((entry ^ this->moves[entry] ^ this->depth[entry] ^ this->score[entry]) != this->chsums[entry]) return false;
        if (this->hashes[entry] == (uint32_t)(hash >> 32) && this->moves[entry] == move) return true;
        else return false;
    }

    int get_score(Board board){
        return score[board.hash % table_size];
    }

    int get_move(Board board){
        return moves[board.hash % table_size];
    }

    ~HASH_TABLE(){
        delete[] hashes;
        delete[] moves;
        delete[] depth;
        delete[] score;
        delete[] type;
    }
};

int HISTORY_HEURISTICS[64][64] = {0};
uint16_t KILLER_MOVES[2][128] = {0};
HASH_TABLE TT;

int             PV_LENGTH[128]      = {0};
uint16_t        PV_TABLE [128][128] = {0};
LINE            LOOKUP_LINE;
HISTORY_LINE    HISTORY;
HASH_HISTORY    HASH_HIST;