#include "board.h"
#include "search.h"
#include "see.h"

int Board::scoreMove(uint16_t move, uint32_t entry, int depth, int pdepth=0){
    if (move == EMPTY) return INT_MIN;
    int fromsq = (move >> 6) & 0b111111;
    int tosq = move & 0b111111;
    int prevfrsq = (last_move >> 6) & 0b111111;
    int prevtosq = last_move & 0b111111;

    int score = 0;

    if (PV_TABLE[0][pdepth] == move || LOOKUP_LINE.argmove[pdepth] == move){
        score += 32768;
    } else if (TT.contains_entry_with_move(entry, this->hash, move)) {
        score += 16384;
    } else if (KILLER_MOVES[0][depth] == move) {
        score += 2048;
    } else if (KILLER_MOVES[1][depth] == move) {
        score += 1024;
    } else if (isCapture(fromsq, tosq)){
        int see_score = see(move);
        if (see_score > -135) score += 8192 + see_score;
        else score += see_score;
        // score += 8192 + (9 * (toPiece&0b111) - (fromPiece&0b111)) * 100;
    } else if (COUNTER_MOVE_HEURISTICS[prevfrsq][prevtosq] == move) {
        score += 512;
    } 
    
    return score;
}


int Board::scoreQuiescence(uint16_t move){
    if (move == EMPTY) return INT_MIN;
    int fromsq = (move >> 6) & 0b111111;
    int tosq = move & 0b111111;
    int fromPiece = getSquare(fromsq);
    int toPiece = getSquare(tosq);
    if (isCapture(fromsq, tosq)){
        return (9 * (toPiece&0b111) - (fromPiece&0b111)) * 12 + HISTORY_HEURISTICS[fromsq][tosq];
    } else return INT_MIN + 1;
    
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