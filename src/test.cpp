#include <iostream>
#include <iterator>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <locale>
#include <chrono>

#include "search.h"

int main() {

    init_move_lookup_tables();

    Board board;

    board.parseFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    board.printChessBoard();


    Perft perft;

    perft.go(board, 7);

    return 0;
}