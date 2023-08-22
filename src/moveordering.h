#include "board.h"
#include "search.h"


int Board::scoreMove(uint16_t move, uint32_t entry, int depth, int pdepth=0){
    int fromsq = (move >> 6) & 0b111111;
    int tosq = move & 0b111111;
    int fromPiece = getSquare(fromsq);
    int toPiece = getSquare(tosq);

    int score = 0;

    if (PV_TABLE[0][pdepth] == move || LOOKUP_LINE.argmove[pdepth] == move){
        score += 32768;
    } else if (TT.contains_entry_with_move(entry, this->hash, move)) {
        score += 16384;
    }
    if (isCapture(fromsq, tosq)){
        score += 8192 + (5 * (toPiece&0b111) - (fromPiece&0b111)) * 50;
    } else if (KILLER_MOVES[0][depth] == move) {
        score += 2048;
    } else if (KILLER_MOVES[1][depth] == move) {
        score += 1024;
    }
    score += HISTORY_HEURISTICS[fromsq][tosq];
    
    return score;
}



int Board::scoreThreadMove(uint16_t move, uint32_t entry){
    int fromsq = (move >> 6) & 0b111111;
    int tosq = move & 0b111111;
    int fromPiece = getSquare(fromsq);
    int toPiece = getSquare(tosq);

    int score = 0;

    if (TT.contains_entry_with_move(entry, this->hash, move)) {
        score += 16384;
    }
    if (isCapture(fromsq, tosq)){
        score += 8192 + (5 * (toPiece&0b111) - (fromPiece&0b111)) * 50;
    }
    
    return score;
}