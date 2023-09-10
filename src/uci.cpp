#include <iostream>
#include <iterator>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <locale>
#include <chrono>
#include <atomic>
#include <thread>

#include "search.h"
#include "search.cpp"

int MAX_THREADS = 1;

const int WindowScores[]    = {40, 80, 160, 240, 320, 560, 2600};
const int WindowPerScore[]  = {160, 200, 280, 420, 880, 4400, 120000};

std::unordered_map<std::string, std::vector<std::string>> options = {
    {"name", {"Alephv0.0.1", "noedit"}},
    {"author", {"kerbal_galactic", "noedit"}},
    {"Threads", {"1", "edit"}},
    {"Hash", {"64", "edit"}},
};

std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;

    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

int getIndex(std::vector<std::string> vector, std::string K) {
    auto it = find(vector.begin(), vector.end(), K);
    if (it != vector.end()) {
        return it - vector.begin();;
    }
    else { return -1; }
}

bool doesContain(std::vector<std::string> vector, std::string K) {
    return std::find(vector.begin(), vector.end(), K) != vector.end();
}

bool is_number(const std::string &s) {
  return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

struct GoOptions {
    int movetime    = INT_MAX;
    int wtime       = -1;
    int btime       = -1;
    int winc        = -1;
    int binc        = -1;
    int depth       = 30;
    uint64_t nodes  = 1000000000;
};

std::string input;

bool stop_break = false;

bool debug_mode = true;

void Stop() {
    while (true) {
        std::string sinput;
        std::getline(std::cin, sinput);

        if (sinput == "stop") {
            stop = true;
            sinput = "";
            break;
        } else {
            input = sinput;
            stop_break = true;
            if (search_complete) break;
        }
    }
}

void EngineGo(Board & board, int depth, int movetime, int wtime, int btime, int winc, int binc, uint64_t nodes) {

    using std::chrono::high_resolution_clock;
    using std::chrono::duration;
    using std::chrono::microseconds;

    auto t1 = high_resolution_clock::now();
    
    bool movetime_specified = false;

    int safety_overhead = 50;
    int max_time = movetime;
    int panic_time = 0;
    int moves_to_go = ((winc && board.side_to_move == WHITE) || (binc && board.side_to_move == BLACK) ? 20 : 45);
    if (movetime != INT_MAX) {
        movetime_specified = true;
        goto movetime_calc;
    }
    else {
        if (wtime > 0 || btime > 0) {
            int base_time = 0;
            if (board.side_to_move == WHITE)
                base_time = wtime / moves_to_go + winc * 3 / 4;
            else
                base_time = btime / moves_to_go + binc * 3 / 4;
            max_time = std::max(5, 2*base_time-safety_overhead);
            movetime = (int)std::max((double)5, 1.35*((float)base_time)-safety_overhead);
            panic_time = (max_time - movetime) / 8;
        }
    }

    movetime_calc:

    Search search;
    std::string bestMove;

    int nps;

    uint64_t total_time = 0;

    int alpha = -120000;
    int beta  =  120000;
    const int MATE_SCORE = 75000;
    int previous_score = 0;
    int window = 45;
    int panic_window = 0;

    int side_to_move = (board.side_to_move == WHITE ? 1 : -1);

    for (int i = 1; i < depth+1; i++) {

        int score = search.root_search(board, i, alpha, beta, side_to_move, movetime);

        LOOKUP_LINE = search.PVline;

        if (stop) {
            stop = false;
            break;
        }

        if (!movetime_specified && i > 2 && (previous_score - score) > 50 && (movetime+panic_time-safety_overhead) < max_time) {
            movetime += panic_time;
            if (debug_mode)
                std::cout << "info string Using panic time due to eval score drop." << "\n";
        }

        previous_score = score;

        if (stop) { stop = false; break; }

        bestMove = moveNotation((search.PVline.argmove[0] >> 6) & 0b111111, search.PVline.argmove[0] & 0b111111, board);

        long double ms = std::chrono::duration_cast<microseconds>(high_resolution_clock::now() - t1).count();

        auto t1 = high_resolution_clock::now();

        total_time += ms;

        movetime -= (total_time/1000);

        if (ms) { nps = (int)(search.searchedPositions*(1000000/ms)); }
        else { nps = 0; }

        std::cout << std::fixed << "info depth " << i << " seldepth " << search.max_depth << " score cp " << score << " nodes " << search.searchedPositions << " nps " << nps << " tthits "<< search.ttHits << " time " << (int)(total_time/1000) << " pv "; 

        Board PVcopy(board);
        for (int i = 0; i < 64; i++) {
            if (!search.PVline.argmove[i]) { break; }
            else {
                if (!PVcopy.getSquare((search.PVline.argmove[i] >> 6) & 0b111111)) break;
                PVcopy.makeMove((search.PVline.argmove[i] >> 6) & 0b111111, search.PVline.argmove[i] & 0b111111);
                Board nullCheck(PVcopy);
                nullCheck.makeNullMove();
                if (nullCheck.isInCheck()) break;
                std::cout << moveNotation((search.PVline.argmove[i] >> 6) & 0b111111, search.PVline.argmove[i] & 0b111111, PVcopy) << " " ;
            }
        }
        std::cout << std::endl;
        if (score >= MATE_SCORE) {
            alpha = score;
            if (alpha >= beta) break;
        }
    }
    std::cout << "bestmove " << bestMove << std::endl;
    search_complete = true;

    std::fill(
        &KILLER_MOVES[0][0],
        &KILLER_MOVES[0][0] + sizeof(KILLER_MOVES) / sizeof(KILLER_MOVES[0][0]),
    0);
    std::fill(
        &HISTORY_HEURISTICS[0][0],
        &HISTORY_HEURISTICS[0][0] + sizeof(HISTORY_HEURISTICS) / sizeof(HISTORY_HEURISTICS[0][0]),
    0);

    TT.flush();

}

int main() {

    init_move_lookup_tables();

    std::cout << "info string Initialized slider attack tables." << "\n";

    GoOptions goOptions;

    std::string fen = "";
    std::vector<std::string> movelist;

    Board board;

    while (true) {

        if (stop_break) {
            stop_break = false;
        } else {
            std::getline(std::cin, input);
        }

        std::vector<std::string> substrings = splitString(input, ' ');

        int scanIndex = 0;

        for (const auto& substring : substrings) {
            if (scanIndex == 0 && substring == "uci" && substrings.size() == 1) {
                std::cout << "\n";
                std::cout << "id name Alephv0.0.1a" << "\n";
                std::cout << "id author kerbal_galactic" << "\n";
                std::cout << "\n";
                std::cout << "option name Threads type spin default 1 min 1 max 8" << "\n";
                std::cout << "option name Hash type spin default 64 min 1 max 128" << "\n";
                std::cout << "uciok" << "\n";
            } else if (scanIndex == 0 && substring == "setoption") {
                if (substrings.size() < scanIndex + 3) {
                    std::cout << "Option not found." << "\n";
                    break;
                } else if (substrings[scanIndex + 1] != "name") {
                    std::cout << "Option not found." << "\n";
                    break;
                } else if (options.find(substrings[scanIndex + 2]) != options.end()) {
                    if (options[substrings[scanIndex + 2]][1] == "noedit") {
                        std::cout << "No such option: " << substrings[scanIndex + 2] << "\n";
                    } else {
                        if (substrings.size() == 5) {
                            options[substrings[scanIndex + 2]][0] = substrings[scanIndex + 4];
                        }
                    }
                } else {
                    std::string lastSubstring = substrings.back();
                    std::cout << "No such option: " << lastSubstring << "\n";
                }
            } else if (scanIndex == 0 && substring == "isready") {
                std::cout << "readyok" << "\n";
            } else if (scanIndex == 0 && substring == "ucinewgame") {
                fen = "";
                movelist.clear();
                board.reset();
            } else if (scanIndex == 0 && substring == "position") {
                if (substrings.size() < 2) {
                    continue;
                }
                fen = "";
                movelist.clear();
                if (substrings[scanIndex + 1] == "startpos") {
                    board.reset();
                    substrings.erase(substrings.begin() + scanIndex + 1);

                }
                if (substrings[scanIndex + 1] == "fen") {
                    int moves_index = getIndex(substrings, "moves");
                    auto end_index = substrings.end() - substrings.size();
                    if (moves_index < 0) { end_index = substrings.end(); }
                    else { end_index += moves_index; }
                    std::vector<std::string> fen_constructed_vector = std::vector<std::string>(substrings.begin() + scanIndex + 2, end_index);
                    const char* const delim = " ";
                    std::ostringstream fen_constructed_stream;
                    std::copy(fen_constructed_vector.begin(), fen_constructed_vector.end(),
                            std::ostream_iterator<std::string>(fen_constructed_stream, delim));
                    std::string fen_constructed(fen_constructed_stream.str());

                    board.parseFEN(fen_constructed);
                    if (moves_index > -1) {
                        for (auto move : std::vector<std::string>(end_index + 1, substrings.end())) {
                            board.makeUCIMove(move);
                        }
                    }
                } else if (substrings[scanIndex + 1] == "moves") {
                    for (auto move : std::vector<std::string>(substrings.begin() + scanIndex + 2, substrings.end())) {
                        int return_code = board.makeUCIMove(move);
                        HISTORY.argmove[HISTORY.cmove] = board.numerizeUCIMove(move);
                        HASH_HIST.arghash[HISTORY.cmove] = board.hash;
                        if (board.getSquare((HISTORY.argmove[HISTORY.cmove]>>6)&0b111111) & 0b111 == PAWN || return_code == 1) {
                            memset(HISTORY.argmove, 0, 256*sizeof(uint16_t));
                            memset(HASH_HIST.arghash, 0, 256*sizeof(uint64_t));
                            HISTORY.cmove = 0;
                            continue;
                        }
                        HISTORY.cmove++;
                    }
                }
            } else if (scanIndex == 0 && substring == "go") {
                goOptions = GoOptions();
                int current_parse_index = 1;
                while (true) {

                    if (current_parse_index < substrings.size() && substrings[current_parse_index] == "infinite") {
                        goOptions.nodes     = UINT64_MAX;
                        goOptions.depth     = 128;
                        goOptions.wtime     = -1;
                        goOptions.btime     = -1;
                        goOptions.movetime  = INT_MAX;
                        break;
                    }

                    if (current_parse_index >= substrings.size()-1) break;

                    if (substrings[current_parse_index] == "wtime") {
                        if (is_number(substrings[current_parse_index+1])) {
                            goOptions.wtime = stoi(substrings[current_parse_index+1]);
                            current_parse_index += 2;
                            continue;
                        } else { goto incAndSkip; }
                    }
                    if (substrings[current_parse_index] == "btime") {
                        if (is_number(substrings[current_parse_index+1])) {
                            goOptions.btime = stoi(substrings[current_parse_index+1]);
                            current_parse_index += 2;
                            continue;
                        } else { goto incAndSkip; }
                    }
                    if (substrings[current_parse_index] == "winc") {
                        if (is_number(substrings[current_parse_index+1])) {
                            goOptions.winc = stoi(substrings[current_parse_index+1]);
                            current_parse_index += 2;
                            continue;
                        } else { goto incAndSkip; }
                    }
                    if (substrings[current_parse_index] == "binc") {
                        if (is_number(substrings[current_parse_index+1])) {
                            goOptions.binc = stoi(substrings[current_parse_index+1]);
                            current_parse_index += 2;
                            continue;
                        } else { goto incAndSkip; }
                    }
                    if (substrings[current_parse_index] == "depth") {
                        if (is_number(substrings[current_parse_index+1])) {
                            goOptions.depth = stoi(substrings[current_parse_index+1]);
                            current_parse_index += 2;
                            continue;
                        } else { goto incAndSkip; }
                    }
                    if (substrings[current_parse_index] == "nodes") {
                        if (is_number(substrings[current_parse_index+1])) {
                            goOptions.nodes = stoull(substrings[current_parse_index+1]);
                            current_parse_index += 2;
                            continue;
                        } else { goto incAndSkip; }
                    }
                    if (substrings[current_parse_index] == "movetime") {
                        if (is_number(substrings[current_parse_index+1])) {
                            goOptions.movetime = stoi(substrings[current_parse_index+1]);
                            current_parse_index += 2;
                            continue;
                        } else { goto incAndSkip; }
                    }
                    incAndSkip:
                    current_parse_index++;
                }

                std::thread goThread(EngineGo, std::ref(board), goOptions.depth, goOptions.movetime, goOptions.wtime, goOptions.btime, goOptions.winc, goOptions.binc, goOptions.nodes);
                std::thread StopThread(Stop);
                goThread.join();
                StopThread.join();
                search_complete = false;
            }
        }

        if (input == "board" || input == "d") {
            board.printChessBoard();
        }

        if (input == "exit" || input == "quit") {
            break;
        }
    }

    return 0;
}