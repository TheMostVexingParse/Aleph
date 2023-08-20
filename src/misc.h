#include <vector>
#include <cassert>
#include <iostream>
#include <algorithm>

#define EMPTY       0b00000000
#define WHITE       0b00001000
#define BLACK       0b00010000

#define PAWN        0b00000001
#define KNIGHT      0b00000010
#define BISHOP      0b00000011
#define ROOK        0b00000100
#define QUEEN       0b00000101
#define KING        0b00000110


const int arrCenterManhattanDistance[64] = { 
  6, 5, 4, 3, 3, 4, 5, 6,
  5, 4, 3, 2, 2, 3, 4, 5,
  4, 3, 2, 1, 1, 2, 3, 4,
  3, 2, 1, 0, 0, 1, 2, 3,
  3, 2, 1, 0, 0, 1, 2, 3,
  4, 3, 2, 1, 1, 2, 3, 4,
  5, 4, 3, 2, 2, 3, 4, 5,
  6, 5, 4, 3, 3, 4, 5, 6
};

const int index64[64] = {
    0, 47,  1, 56, 48, 27,  2, 60,
    57, 49, 41, 37, 28, 16,  3, 61,
    54, 58, 35, 52, 50, 42, 21, 44,
    38, 32, 29, 23, 17, 11,  4, 62,
    46, 55, 26, 59, 40, 36, 15, 53,
    34, 51, 20, 43, 31, 22, 10, 45,
    25, 39, 14, 33, 19, 30,  9, 24,
    13, 18,  8, 12,  7,  6,  5, 63
};

int bitScanForward(uint64_t bb) {
    const uint64_t debruijn64 = 0x03f79d71b4cb0a89;
    assert(bb != 0);
    return index64[((bb ^ (bb - 1)) * debruijn64) >> 58];
}

int hamming_weight(uint64_t x)
{
    x -= (x >> 1) & 0x5555555555555555;           
    x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333); 
    x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0f;        
    return (x * 0x0101010101010101) >> 56;  
}

inline int pst(uint64_t bitboard, const int* pieceSquareTables) {
    int sum = 0;
    while (bitboard) {
        int square = bitScanForward(bitboard);
        sum += pieceSquareTables[square];
        bitboard &= bitboard - 1;
    }
    return sum;
}


inline uint64_t leftShift(uint64_t left, int right) {
    if (right < 0) { return left << -right; }
    else { return left << right; }
}

int manhattanDistance(int sq1, int sq2) {
   int file1, file2, rank1, rank2;
   int rankDistance, fileDistance;
   file1 = sq1  & 7;
   file2 = sq2  & 7;
   rank1 = sq1 >> 3;
   rank2 = sq2 >> 3;
   rankDistance = abs (rank2 - rank1);
   fileDistance = abs (file2 - file1);
   return rankDistance + fileDistance;
}


void printBitboard(uint64_t bitboard) {
    for (int rank = 7; rank >= 0; rank--) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            int bit = (bitboard >> square) & 1;
            std::cout << bit << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}