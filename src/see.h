#include "board.h"


int Board::see(uint16_t move) {

    #define unset_bit(bb, sq) bb &= ~(1ULL<<sq)
    #define set_bit(bb, sq) bb |= (1ULL<<sq)

    uint64_t wattackers = 0;  // direct atackers
    uint64_t battackers = 0;  // direct atackers

    uint64_t occupied = ~pieces[0];

    int target_square = move & 0b111111;
    int from_square = (move >> 6) & 0b111111;

    uint64_t rook_attack_map = get_rook_attacks(target_square, occupied);
    uint64_t bishop_attack_map = get_bishop_attacks(target_square, occupied);

    uint64_t bitboards[12] = {0};
    memcpy(bitboards, original_bitboards, sizeof(uint64_t)*12);

    uint64_t sliders = bitboards[2] | bitboards[3] | bitboards[4] |
                       bitboards[8] | bitboards[9] | bitboards[10];

    wattackers |= black_pawn_move_lookup_table[target_square] & bitboards[0];
    battackers |= white_pawn_move_lookup_table[target_square] & bitboards[6];

    wattackers |= knight_move_lookup_table[target_square] & bitboards[1];
    battackers |= knight_move_lookup_table[target_square] & bitboards[7];

    wattackers |= king_move_lookup_table[target_square] & bitboards[5];
    battackers |= king_move_lookup_table[target_square] & bitboards[11];

    wattackers |= rook_attack_map & (bitboards[3] | bitboards[4]);
    battackers |= rook_attack_map & (bitboards[9] | bitboards[10]);

    wattackers |= bishop_attack_map & (bitboards[2] | bitboards[4]);
    battackers |= bishop_attack_map & (bitboards[8] | bitboards[10]);

    int sdm = side_to_move;

    int capture_sequence[32] = {0}; // we wont actually need a stop element but anyway 
    char capture_index = 2;

    int capturing_piece_type = (getSquare(from_square) & 0b111) - 1; // we will also use this to toggle the first capturing piece bit
    int captured_piece = (getSquare(target_square) & 0b111);

    capture_sequence[0] = values[captured_piece - 1]; // add the first captured piece to the sequence after offsetting by 1
    capture_sequence[1] = values[capturing_piece_type] - capture_sequence[0];

    if (sdm == WHITE) {
        unset_bit(bitboards[capturing_piece_type], from_square); // unset the first capturing piece bit
    } else {
        unset_bit(bitboards[capturing_piece_type + 6], from_square); // unset the first capturing piece bit
    }

    while (true) {
        char sdm_offset = (sdm == WHITE) ? 0 : 6; // white and black bitboards are offsetted by 6
        uint64_t curr_attackers = (sdm == WHITE) ? wattackers : battackers; // get current attackers
        uint64_t lvattacker = 0; // will contain lowest valued attacker
        int currSq;          // will contain the square for the lowest valued attacker
        for (int i = 0; i < 6; i++) { // loop through the bitboards to find the lowest value piece
            lvattacker = bitboards[i+sdm_offset]; // get the bitboard
            if (lvattacker & curr_attackers) { // if there is a shared bit
                currSq = getlsb(lvattacker & curr_attackers); // get attacker bit index
                unset_bit(bitboards[i+sdm_offset], currSq); // unset the first bit
                if (sdm == WHITE)
                     unset_bit(wattackers, currSq); // unset the attacker in the white bitboard
                else unset_bit(battackers, currSq); // unset the attacker in the black bitboard
                // note that we can't use   &= ~lvattacker here
                unset_bit(occupied, currSq); // unset the occupancy
                capture_sequence[capture_index] = values[i] - capture_sequence[capture_index-1]; // add the capturing piece to the sequence
                capture_index++;
                if (std::max(-capture_sequence[capture_index-1], capture_sequence[capture_index]) < 0) goto prune;
                if (i > 1 && i < 5) { // if a slider moves we will need to calculate sliders again
                    // we can optimize this part by using a switch on i  =  partially completed
                    uint64_t bishop_attack_map;
                    uint64_t rook_attack_map;
                    switch (i) {
                        case 2:
                            bishop_attack_map = get_bishop_attacks(target_square, occupied); // generate bishop attacks
                            wattackers |= bishop_attack_map & (bitboards[2] | bitboards[4]);
                            battackers |= bishop_attack_map & (bitboards[8] | bitboards[10]);
                            break;
                        case 3:
                            rook_attack_map = get_rook_attacks(target_square, occupied); // generate rook attacks
                            wattackers |= rook_attack_map & (bitboards[3] | bitboards[4]);
                            battackers |= rook_attack_map & (bitboards[9] | bitboards[10]);
                            break;                    
                        case 4:
                            rook_attack_map = get_rook_attacks(target_square, occupied); // generate rook attacks
                            bishop_attack_map = get_bishop_attacks(target_square, occupied); // generate bishop attacks
                            wattackers |= rook_attack_map & (bitboards[3] | bitboards[4]);
                            battackers |= rook_attack_map & (bitboards[9] | bitboards[10]);
                            wattackers |= bishop_attack_map & (bitboards[2] | bitboards[4]);
                            battackers |= bishop_attack_map & (bitboards[8] | bitboards[10]);
                            break;
                        default:
                            assert(false);
                    }
                }
                break; 
            }
        }
        if (!curr_attackers) break; // if we run out of attackers we can exit the loop
        sdm = (~sdm) & 0b11000; // change the turn
    }

    prune:

    capture_index -= 2;

    while(capture_index > -1) {
        capture_sequence[capture_index-1]= -std::max(-capture_sequence[capture_index-1], capture_sequence[capture_index]);
        // std::cout << "gain[" << (int)capture_index << "] = " << capture_sequence[capture_index] << std::endl;
        capture_index--;
    }

    return capture_sequence[0];

}
