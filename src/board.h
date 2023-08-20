#pragma once

#include <iostream>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <chrono>

#include <algorithm>
#include <functional>
#include <sstream>

#include <atomic>
#include <cstring>
#include <unordered_map>

#include "ctype.h"


#include "pst.h"
#include "misc.h"
#include "search.h"
#include "magics.cpp"

// #include "nnue/misc.cpp"
// #include "nnue/nnue.cpp"




#define BOARD_SIZE 64
#define BOARD_WIDTH 8
#define BOARD_HEIGHT 8

#define EMPTY       0b00000000
#define WHITE       0b00001000
#define BLACK       0b00010000

#define PAWN        0b00000001
#define KNIGHT      0b00000010
#define BISHOP      0b00000011
#define ROOK        0b00000100
#define QUEEN       0b00000101
#define KING        0b00000110

#define NORTH(sq) ((sq) << 8)
#define SOUTH(sq) ((sq) >> 8)
#define EAST(sq) (((sq) << 1) & 0xfefefefefefefefeULL)
#define WEST(sq) (((sq) >> 1) & 0x7f7f7f7f7f7f7f7fULL)
#define NORTH_WITH_STEP(sq, step) ((sq) << step)
#define SOUTH_WITH_STEP(sq, step) ((sq) >> step)

#define NORTH_EAST(sq) (NORTH(EAST(sq)))
#define NORTH_WEST(sq) (NORTH(WEST(sq)))
#define SOUTH_EAST(sq) (SOUTH(EAST(sq)))
#define SOUTH_WEST(sq) (SOUTH(WEST(sq)))

#define ENCODE_MOVE(fs, ts) (fs << 6 | ts)

#define FLIP(x) (x^63)

#define bitcount __builtin_popcountll
#define getlsb   __builtin_ctzll

#define filled_block_mask 0xFFFFFFFFFFFFFFFFULL
#define empty_block_mask 0x0000000000000000ULL

#define file_mask               0b1000000010000000100000001000000010000000100000001000000010000000
#define rank_mask               0b0000000000000000000000000000000000000000000000000000000011111111
#define right_diagonal_mask     0b0000000100000010000001000000100000010000001000000100000010000000
#define left_diagonal_mask      0b1000000001000000001000000001000000001000000001000000001000000001
#define knight_mask             0b0000000000101000010001000000000001000100001010000000000000000000
#define king_mask               0b0000000000000000001110000010100000111000000000000000000000000000

#define wking_prot_mask         0b0011100001111100011111000000000000000000000000000000000000000000
#define bking_prot_mask         0b0000000000000000011111000111110000111000000000000000000000000000

#define white_short_castling    0b0000000000000000000000000000000000000000000000000000000001100000
#define white_long_castling     0b0000000000000000000000000000000000000000000000000000000000001110

#define black_short_castling    0b0110000000000000000000000000000000000000000000000000000000000000
#define black_long_castling     0b0000111000000000000000000000000000000000000000000000000000000000

#define filled_board            0b1111111111111111111111111111111111111111111111111111111111111111

#define CUT 1
#define ALL 2
#define PV  3

std::atomic_bool stop = { false };

const int entry_bits = 24;
const int table_size = 16777216; 

const uint64_t zobrist_keys[781] = {
    0x9D39247E33776D41ULL, 0x2AF7398005AAA5C7ULL, 0x44DB015024623547ULL, 0x9C15F73E62A76AE2ULL,
    0x75834465489C0C89ULL, 0x3290AC3A203001BFULL, 0x0FBBAD1F61042279ULL, 0xE83A908FF2FB60CAULL,
    0x0D7E765D58755C10ULL, 0x1A083822CEAFE02DULL, 0x9605D5F0E25EC3B0ULL, 0xD021FF5CD13A2ED5ULL,
    0x40BDF15D4A672E32ULL, 0x011355146FD56395ULL, 0x5DB4832046F3D9E5ULL, 0x239F8B2D7FF719CCULL,
    0x05D1A1AE85B49AA1ULL, 0x679F848F6E8FC971ULL, 0x7449BBFF801FED0BULL, 0x7D11CDB1C3B7ADF0ULL,
    0x82C7709E781EB7CCULL, 0xF3218F1C9510786CULL, 0x331478F3AF51BBE6ULL, 0x4BB38DE5E7219443ULL,
    0xAA649C6EBCFD50FCULL, 0x8DBD98A352AFD40BULL, 0x87D2074B81D79217ULL, 0x19F3C751D3E92AE1ULL,
    0xB4AB30F062B19ABFULL, 0x7B0500AC42047AC4ULL, 0xC9452CA81A09D85DULL, 0x24AA6C514DA27500ULL,
    0x4C9F34427501B447ULL, 0x14A68FD73C910841ULL, 0xA71B9B83461CBD93ULL, 0x03488B95B0F1850FULL,
    0x637B2B34FF93C040ULL, 0x09D1BC9A3DD90A94ULL, 0x3575668334A1DD3BULL, 0x735E2B97A4C45A23ULL,
    0x18727070F1BD400BULL, 0x1FCBACD259BF02E7ULL, 0xD310A7C2CE9B6555ULL, 0xBF983FE0FE5D8244ULL,
    0x9F74D14F7454A824ULL, 0x51EBDC4AB9BA3035ULL, 0x5C82C505DB9AB0FAULL, 0xFCF7FE8A3430B241ULL,
    0x3253A729B9BA3DDEULL, 0x8C74C368081B3075ULL, 0xB9BC6C87167C33E7ULL, 0x7EF48F2B83024E20ULL,
    0x11D505D4C351BD7FULL, 0x6568FCA92C76A243ULL, 0x4DE0B0F40F32A7B8ULL, 0x96D693460CC37E5DULL,
    0x42E240CB63689F2FULL, 0x6D2BDCDAE2919661ULL, 0x42880B0236E4D951ULL, 0x5F0F4A5898171BB6ULL,
    0x39F890F579F92F88ULL, 0x93C5B5F47356388BULL, 0x63DC359D8D231B78ULL, 0xEC16CA8AEA98AD76ULL,
    0x5355F900C2A82DC7ULL, 0x07FB9F855A997142ULL, 0x5093417AA8A7ED5EULL, 0x7BCBC38DA25A7F3CULL,
    0x19FC8A768CF4B6D4ULL, 0x637A7780DECFC0D9ULL, 0x8249A47AEE0E41F7ULL, 0x79AD695501E7D1E8ULL,
    0x14ACBAF4777D5776ULL, 0xF145B6BECCDEA195ULL, 0xDABF2AC8201752FCULL, 0x24C3C94DF9C8D3F6ULL,
    0xBB6E2924F03912EAULL, 0x0CE26C0B95C980D9ULL, 0xA49CD132BFBF7CC4ULL, 0xE99D662AF4243939ULL,
    0x27E6AD7891165C3FULL, 0x8535F040B9744FF1ULL, 0x54B3F4FA5F40D873ULL, 0x72B12C32127FED2BULL,
    0xEE954D3C7B411F47ULL, 0x9A85AC909A24EAA1ULL, 0x70AC4CD9F04F21F5ULL, 0xF9B89D3E99A075C2ULL,
    0x87B3E2B2B5C907B1ULL, 0xA366E5B8C54F48B8ULL, 0xAE4A9346CC3F7CF2ULL, 0x1920C04D47267BBDULL,
    0x87BF02C6B49E2AE9ULL, 0x092237AC237F3859ULL, 0xFF07F64EF8ED14D0ULL, 0x8DE8DCA9F03CC54EULL,
    0x9C1633264DB49C89ULL, 0xB3F22C3D0B0B38EDULL, 0x390E5FB44D01144BULL, 0x5BFEA5B4712768E9ULL,
    0x1E1032911FA78984ULL, 0x9A74ACB964E78CB3ULL, 0x4F80F7A035DAFB04ULL, 0x6304D09A0B3738C4ULL,
    0x2171E64683023A08ULL, 0x5B9B63EB9CEFF80CULL, 0x506AACF489889342ULL, 0x1881AFC9A3A701D6ULL,
    0x6503080440750644ULL, 0xDFD395339CDBF4A7ULL, 0xEF927DBCF00C20F2ULL, 0x7B32F7D1E03680ECULL,
    0xB9FD7620E7316243ULL, 0x05A7E8A57DB91B77ULL, 0xB5889C6E15630A75ULL, 0x4A750A09CE9573F7ULL,
    0xCF464CEC899A2F8AULL, 0xF538639CE705B824ULL, 0x3C79A0FF5580EF7FULL, 0xEDE6C87F8477609DULL,
    0x799E81F05BC93F31ULL, 0x86536B8CF3428A8CULL, 0x97D7374C60087B73ULL, 0xA246637CFF328532ULL,
    0x043FCAE60CC0EBA0ULL, 0x920E449535DD359EULL, 0x70EB093B15B290CCULL, 0x73A1921916591CBDULL,
    0x56436C9FE1A1AA8DULL, 0xEFAC4B70633B8F81ULL, 0xBB215798D45DF7AFULL, 0x45F20042F24F1768ULL,
    0x930F80F4E8EB7462ULL, 0xFF6712FFCFD75EA1ULL, 0xAE623FD67468AA70ULL, 0xDD2C5BC84BC8D8FCULL,
    0x7EED120D54CF2DD9ULL, 0x22FE545401165F1CULL, 0xC91800E98FB99929ULL, 0x808BD68E6AC10365ULL,
    0xDEC468145B7605F6ULL, 0x1BEDE3A3AEF53302ULL, 0x43539603D6C55602ULL, 0xAA969B5C691CCB7AULL,
    0xA87832D392EFEE56ULL, 0x65942C7B3C7E11AEULL, 0xDED2D633CAD004F6ULL, 0x21F08570F420E565ULL,
    0xB415938D7DA94E3CULL, 0x91B859E59ECB6350ULL, 0x10CFF333E0ED804AULL, 0x28AED140BE0BB7DDULL,
    0xC5CC1D89724FA456ULL, 0x5648F680F11A2741ULL, 0x2D255069F0B7DAB3ULL, 0x9BC5A38EF729ABD4ULL,
    0xEF2F054308F6A2BCULL, 0xAF2042F5CC5C2858ULL, 0x480412BAB7F5BE2AULL, 0xAEF3AF4A563DFE43ULL,
    0x19AFE59AE451497FULL, 0x52593803DFF1E840ULL, 0xF4F076E65F2CE6F0ULL, 0x11379625747D5AF3ULL,
    0xBCE5D2248682C115ULL, 0x9DA4243DE836994FULL, 0x066F70B33FE09017ULL, 0x4DC4DE189B671A1CULL,
    0x51039AB7712457C3ULL, 0xC07A3F80C31FB4B4ULL, 0xB46EE9C5E64A6E7CULL, 0xB3819A42ABE61C87ULL,
    0x21A007933A522A20ULL, 0x2DF16F761598AA4FULL, 0x763C4A1371B368FDULL, 0xF793C46702E086A0ULL,
    0xD7288E012AEB8D31ULL, 0xDE336A2A4BC1C44BULL, 0x0BF692B38D079F23ULL, 0x2C604A7A177326B3ULL,
    0x4850E73E03EB6064ULL, 0xCFC447F1E53C8E1BULL, 0xB05CA3F564268D99ULL, 0x9AE182C8BC9474E8ULL,
    0xA4FC4BD4FC5558CAULL, 0xE755178D58FC4E76ULL, 0x69B97DB1A4C03DFEULL, 0xF9B5B7C4ACC67C96ULL,
    0xFC6A82D64B8655FBULL, 0x9C684CB6C4D24417ULL, 0x8EC97D2917456ED0ULL, 0x6703DF9D2924E97EULL,
    0xC547F57E42A7444EULL, 0x78E37644E7CAD29EULL, 0xFE9A44E9362F05FAULL, 0x08BD35CC38336615ULL,
    0x9315E5EB3A129ACEULL, 0x94061B871E04DF75ULL, 0xDF1D9F9D784BA010ULL, 0x3BBA57B68871B59DULL,
    0xD2B7ADEEDED1F73FULL, 0xF7A255D83BC373F8ULL, 0xD7F4F2448C0CEB81ULL, 0xD95BE88CD210FFA7ULL,
    0x336F52F8FF4728E7ULL, 0xA74049DAC312AC71ULL, 0xA2F61BB6E437FDB5ULL, 0x4F2A5CB07F6A35B3ULL,
    0x87D380BDA5BF7859ULL, 0x16B9F7E06C453A21ULL, 0x7BA2484C8A0FD54EULL, 0xF3A678CAD9A2E38CULL,
    0x39B0BF7DDE437BA2ULL, 0xFCAF55C1BF8A4424ULL, 0x18FCF680573FA594ULL, 0x4C0563B89F495AC3ULL,
    0x40E087931A00930DULL, 0x8CFFA9412EB642C1ULL, 0x68CA39053261169FULL, 0x7A1EE967D27579E2ULL,
    0x9D1D60E5076F5B6FULL, 0x3810E399B6F65BA2ULL, 0x32095B6D4AB5F9B1ULL, 0x35CAB62109DD038AULL,
    0xA90B24499FCFAFB1ULL, 0x77A225A07CC2C6BDULL, 0x513E5E634C70E331ULL, 0x4361C0CA3F692F12ULL,
    0xD941ACA44B20A45BULL, 0x528F7C8602C5807BULL, 0x52AB92BEB9613989ULL, 0x9D1DFA2EFC557F73ULL,
    0x722FF175F572C348ULL, 0x1D1260A51107FE97ULL, 0x7A249A57EC0C9BA2ULL, 0x04208FE9E8F7F2D6ULL,
    0x5A110C6058B920A0ULL, 0x0CD9A497658A5698ULL, 0x56FD23C8F9715A4CULL, 0x284C847B9D887AAEULL,
    0x04FEABFBBDB619CBULL, 0x742E1E651C60BA83ULL, 0x9A9632E65904AD3CULL, 0x881B82A13B51B9E2ULL,
    0x506E6744CD974924ULL, 0xB0183DB56FFC6A79ULL, 0x0ED9B915C66ED37EULL, 0x5E11E86D5873D484ULL,
    0xF678647E3519AC6EULL, 0x1B85D488D0F20CC5ULL, 0xDAB9FE6525D89021ULL, 0x0D151D86ADB73615ULL,
    0xA865A54EDCC0F019ULL, 0x93C42566AEF98FFBULL, 0x99E7AFEABE000731ULL, 0x48CBFF086DDF285AULL,
    0x7F9B6AF1EBF78BAFULL, 0x58627E1A149BBA21ULL, 0x2CD16E2ABD791E33ULL, 0xD363EFF5F0977996ULL,
    0x0CE2A38C344A6EEDULL, 0x1A804AADB9CFA741ULL, 0x907F30421D78C5DEULL, 0x501F65EDB3034D07ULL,
    0x37624AE5A48FA6E9ULL, 0x957BAF61700CFF4EULL, 0x3A6C27934E31188AULL, 0xD49503536ABCA345ULL,
    0x088E049589C432E0ULL, 0xF943AEE7FEBF21B8ULL, 0x6C3B8E3E336139D3ULL, 0x364F6FFA464EE52EULL,
    0xD60F6DCEDC314222ULL, 0x56963B0DCA418FC0ULL, 0x16F50EDF91E513AFULL, 0xEF1955914B609F93ULL,
    0x565601C0364E3228ULL, 0xECB53939887E8175ULL, 0xBAC7A9A18531294BULL, 0xB344C470397BBA52ULL,
    0x65D34954DAF3CEBDULL, 0xB4B81B3FA97511E2ULL, 0xB422061193D6F6A7ULL, 0x071582401C38434DULL,
    0x7A13F18BBEDC4FF5ULL, 0xBC4097B116C524D2ULL, 0x59B97885E2F2EA28ULL, 0x99170A5DC3115544ULL,
    0x6F423357E7C6A9F9ULL, 0x325928EE6E6F8794ULL, 0xD0E4366228B03343ULL, 0x565C31F7DE89EA27ULL,
    0x30F5611484119414ULL, 0xD873DB391292ED4FULL, 0x7BD94E1D8E17DEBCULL, 0xC7D9F16864A76E94ULL,
    0x947AE053EE56E63CULL, 0xC8C93882F9475F5FULL, 0x3A9BF55BA91F81CAULL, 0xD9A11FBB3D9808E4ULL,
    0x0FD22063EDC29FCAULL, 0xB3F256D8ACA0B0B9ULL, 0xB03031A8B4516E84ULL, 0x35DD37D5871448AFULL,
    0xE9F6082B05542E4EULL, 0xEBFAFA33D7254B59ULL, 0x9255ABB50D532280ULL, 0xB9AB4CE57F2D34F3ULL,
    0x693501D628297551ULL, 0xC62C58F97DD949BFULL, 0xCD454F8F19C5126AULL, 0xBBE83F4ECC2BDECBULL,
    0xDC842B7E2819E230ULL, 0xBA89142E007503B8ULL, 0xA3BC941D0A5061CBULL, 0xE9F6760E32CD8021ULL,
    0x09C7E552BC76492FULL, 0x852F54934DA55CC9ULL, 0x8107FCCF064FCF56ULL, 0x098954D51FFF6580ULL,
    0x23B70EDB1955C4BFULL, 0xC330DE426430F69DULL, 0x4715ED43E8A45C0AULL, 0xA8D7E4DAB780A08DULL,
    0x0572B974F03CE0BBULL, 0xB57D2E985E1419C7ULL, 0xE8D9ECBE2CF3D73FULL, 0x2FE4B17170E59750ULL,
    0x11317BA87905E790ULL, 0x7FBF21EC8A1F45ECULL, 0x1725CABFCB045B00ULL, 0x964E915CD5E2B207ULL,
    0x3E2B8BCBF016D66DULL, 0xBE7444E39328A0ACULL, 0xF85B2B4FBCDE44B7ULL, 0x49353FEA39BA63B1ULL,
    0x1DD01AAFCD53486AULL, 0x1FCA8A92FD719F85ULL, 0xFC7C95D827357AFAULL, 0x18A6A990C8B35EBDULL,
    0xCCCB7005C6B9C28DULL, 0x3BDBB92C43B17F26ULL, 0xAA70B5B4F89695A2ULL, 0xE94C39A54A98307FULL,
    0xB7A0B174CFF6F36EULL, 0xD4DBA84729AF48ADULL, 0x2E18BC1AD9704A68ULL, 0x2DE0966DAF2F8B1CULL,
    0xB9C11D5B1E43A07EULL, 0x64972D68DEE33360ULL, 0x94628D38D0C20584ULL, 0xDBC0D2B6AB90A559ULL,
    0xD2733C4335C6A72FULL, 0x7E75D99D94A70F4DULL, 0x6CED1983376FA72BULL, 0x97FCAACBF030BC24ULL,
    0x7B77497B32503B12ULL, 0x8547EDDFB81CCB94ULL, 0x79999CDFF70902CBULL, 0xCFFE1939438E9B24ULL,
    0x829626E3892D95D7ULL, 0x92FAE24291F2B3F1ULL, 0x63E22C147B9C3403ULL, 0xC678B6D860284A1CULL,
    0x5873888850659AE7ULL, 0x0981DCD296A8736DULL, 0x9F65789A6509A440ULL, 0x9FF38FED72E9052FULL,
    0xE479EE5B9930578CULL, 0xE7F28ECD2D49EECDULL, 0x56C074A581EA17FEULL, 0x5544F7D774B14AEFULL,
    0x7B3F0195FC6F290FULL, 0x12153635B2C0CF57ULL, 0x7F5126DBBA5E0CA7ULL, 0x7A76956C3EAFB413ULL,
    0x3D5774A11D31AB39ULL, 0x8A1B083821F40CB4ULL, 0x7B4A38E32537DF62ULL, 0x950113646D1D6E03ULL,
    0x4DA8979A0041E8A9ULL, 0x3BC36E078F7515D7ULL, 0x5D0A12F27AD310D1ULL, 0x7F9D1A2E1EBE1327ULL,
    0xDA3A361B1C5157B1ULL, 0xDCDD7D20903D0C25ULL, 0x36833336D068F707ULL, 0xCE68341F79893389ULL,
    0xAB9090168DD05F34ULL, 0x43954B3252DC25E5ULL, 0xB438C2B67F98E5E9ULL, 0x10DCD78E3851A492ULL,
    0xDBC27AB5447822BFULL, 0x9B3CDB65F82CA382ULL, 0xB67B7896167B4C84ULL, 0xBFCED1B0048EAC50ULL,
    0xA9119B60369FFEBDULL, 0x1FFF7AC80904BF45ULL, 0xAC12FB171817EEE7ULL, 0xAF08DA9177DDA93DULL,
    0x1B0CAB936E65C744ULL, 0xB559EB1D04E5E932ULL, 0xC37B45B3F8D6F2BAULL, 0xC3A9DC228CAAC9E9ULL,
    0xF3B8B6675A6507FFULL, 0x9FC477DE4ED681DAULL, 0x67378D8ECCEF96CBULL, 0x6DD856D94D259236ULL,
    0xA319CE15B0B4DB31ULL, 0x073973751F12DD5EULL, 0x8A8E849EB32781A5ULL, 0xE1925C71285279F5ULL,
    0x74C04BF1790C0EFEULL, 0x4DDA48153C94938AULL, 0x9D266D6A1CC0542CULL, 0x7440FB816508C4FEULL,
    0x13328503DF48229FULL, 0xD6BF7BAEE43CAC40ULL, 0x4838D65F6EF6748FULL, 0x1E152328F3318DEAULL,
    0x8F8419A348F296BFULL, 0x72C8834A5957B511ULL, 0xD7A023A73260B45CULL, 0x94EBC8ABCFB56DAEULL,
    0x9FC10D0F989993E0ULL, 0xDE68A2355B93CAE6ULL, 0xA44CFE79AE538BBEULL, 0x9D1D84FCCE371425ULL,
    0x51D2B1AB2DDFB636ULL, 0x2FD7E4B9E72CD38CULL, 0x65CA5B96B7552210ULL, 0xDD69A0D8AB3B546DULL,
    0x604D51B25FBF70E2ULL, 0x73AA8A564FB7AC9EULL, 0x1A8C1E992B941148ULL, 0xAAC40A2703D9BEA0ULL,
    0x764DBEAE7FA4F3A6ULL, 0x1E99B96E70A9BE8BULL, 0x2C5E9DEB57EF4743ULL, 0x3A938FEE32D29981ULL,
    0x26E6DB8FFDF5ADFEULL, 0x469356C504EC9F9DULL, 0xC8763C5B08D1908CULL, 0x3F6C6AF859D80055ULL,
    0x7F7CC39420A3A545ULL, 0x9BFB227EBDF4C5CEULL, 0x89039D79D6FC5C5CULL, 0x8FE88B57305E2AB6ULL,
    0xA09E8C8C35AB96DEULL, 0xFA7E393983325753ULL, 0xD6B6D0ECC617C699ULL, 0xDFEA21EA9E7557E3ULL,
    0xB67C1FA481680AF8ULL, 0xCA1E3785A9E724E5ULL, 0x1CFC8BED0D681639ULL, 0xD18D8549D140CAEAULL,
    0x4ED0FE7E9DC91335ULL, 0xE4DBF0634473F5D2ULL, 0x1761F93A44D5AEFEULL, 0x53898E4C3910DA55ULL,
    0x734DE8181F6EC39AULL, 0x2680B122BAA28D97ULL, 0x298AF231C85BAFABULL, 0x7983EED3740847D5ULL,
    0x66C1A2A1A60CD889ULL, 0x9E17E49642A3E4C1ULL, 0xEDB454E7BADC0805ULL, 0x50B704CAB602C329ULL,
    0x4CC317FB9CDDD023ULL, 0x66B4835D9EAFEA22ULL, 0x219B97E26FFC81BDULL, 0x261E4E4C0A333A9DULL,
    0x1FE2CCA76517DB90ULL, 0xD7504DFA8816EDBBULL, 0xB9571FA04DC089C8ULL, 0x1DDC0325259B27DEULL,
    0xCF3F4688801EB9AAULL, 0xF4F5D05C10CAB243ULL, 0x38B6525C21A42B0EULL, 0x36F60E2BA4FA6800ULL,
    0xEB3593803173E0CEULL, 0x9C4CD6257C5A3603ULL, 0xAF0C317D32ADAA8AULL, 0x258E5A80C7204C4BULL,
    0x8B889D624D44885DULL, 0xF4D14597E660F855ULL, 0xD4347F66EC8941C3ULL, 0xE699ED85B0DFB40DULL,
    0x2472F6207C2D0484ULL, 0xC2A1E7B5B459AEB5ULL, 0xAB4F6451CC1D45ECULL, 0x63767572AE3D6174ULL,
    0xA59E0BD101731A28ULL, 0x116D0016CB948F09ULL, 0x2CF9C8CA052F6E9FULL, 0x0B090A7560A968E3ULL,
    0xABEEDDB2DDE06FF1ULL, 0x58EFC10B06A2068DULL, 0xC6E57A78FBD986E0ULL, 0x2EAB8CA63CE802D7ULL,
    0x14A195640116F336ULL, 0x7C0828DD624EC390ULL, 0xD74BBE77E6116AC7ULL, 0x804456AF10F5FB53ULL,
    0xEBE9EA2ADF4321C7ULL, 0x03219A39EE587A30ULL, 0x49787FEF17AF9924ULL, 0xA1E9300CD8520548ULL,
    0x5B45E522E4B1B4EFULL, 0xB49C3B3995091A36ULL, 0xD4490AD526F14431ULL, 0x12A8F216AF9418C2ULL,
    0x001F837CC7350524ULL, 0x1877B51E57A764D5ULL, 0xA2853B80F17F58EEULL, 0x993E1DE72D36D310ULL,
    0xB3598080CE64A656ULL, 0x252F59CF0D9F04BBULL, 0xD23C8E176D113600ULL, 0x1BDA0492E7E4586EULL,
    0x21E0BD5026C619BFULL, 0x3B097ADAF088F94EULL, 0x8D14DEDB30BE846EULL, 0xF95CFFA23AF5F6F4ULL,
    0x3871700761B3F743ULL, 0xCA672B91E9E4FA16ULL, 0x64C8E531BFF53B55ULL, 0x241260ED4AD1E87DULL,
    0x106C09B972D2E822ULL, 0x7FBA195410E5CA30ULL, 0x7884D9BC6CB569D8ULL, 0x0647DFEDCD894A29ULL,
    0x63573FF03E224774ULL, 0x4FC8E9560F91B123ULL, 0x1DB956E450275779ULL, 0xB8D91274B9E9D4FBULL,
    0xA2EBEE47E2FBFCE1ULL, 0xD9F1F30CCD97FB09ULL, 0xEFED53D75FD64E6BULL, 0x2E6D02C36017F67FULL,
    0xA9AA4D20DB084E9BULL, 0xB64BE8D8B25396C1ULL, 0x70CB6AF7C2D5BCF0ULL, 0x98F076A4F7A2322EULL,
    0xBF84470805E69B5FULL, 0x94C3251F06F90CF3ULL, 0x3E003E616A6591E9ULL, 0xB925A6CD0421AFF3ULL,
    0x61BDD1307C66E300ULL, 0xBF8D5108E27E0D48ULL, 0x240AB57A8B888B20ULL, 0xFC87614BAF287E07ULL,
    0xEF02CDD06FFDB432ULL, 0xA1082C0466DF6C0AULL, 0x8215E577001332C8ULL, 0xD39BB9C3A48DB6CFULL,
    0x2738259634305C14ULL, 0x61CF4F94C97DF93DULL, 0x1B6BACA2AE4E125BULL, 0x758F450C88572E0BULL,
    0x959F587D507A8359ULL, 0xB063E962E045F54DULL, 0x60E8ED72C0DFF5D1ULL, 0x7B64978555326F9FULL,
    0xFD080D236DA814BAULL, 0x8C90FD9B083F4558ULL, 0x106F72FE81E2C590ULL, 0x7976033A39F7D952ULL,
    0xA4EC0132764CA04BULL, 0x733EA705FAE4FA77ULL, 0xB4D8F77BC3E56167ULL, 0x9E21F4F903B33FD9ULL,
    0x9D765E419FB69F6DULL, 0xD30C088BA61EA5EFULL, 0x5D94337FBFAF7F5BULL, 0x1A4E4822EB4D7A59ULL,
    0x6FFE73E81B637FB3ULL, 0xDDF957BC36D8B9CAULL, 0x64D0E29EEA8838B3ULL, 0x08DD9BDFD96B9F63ULL,
    0x087E79E5A57D1D13ULL, 0xE328E230E3E2B3FBULL, 0x1C2559E30F0946BEULL, 0x720BF5F26F4D2EAAULL,
    0xB0774D261CC609DBULL, 0x443F64EC5A371195ULL, 0x4112CF68649A260EULL, 0xD813F2FAB7F5C5CAULL,
    0x660D3257380841EEULL, 0x59AC2C7873F910A3ULL, 0xE846963877671A17ULL, 0x93B633ABFA3469F8ULL,
    0xC0C0F5A60EF4CDCFULL, 0xCAF21ECD4377B28CULL, 0x57277707199B8175ULL, 0x506C11B9D90E8B1DULL,
    0xD83CC2687A19255FULL, 0x4A29C6465A314CD1ULL, 0xED2DF21216235097ULL, 0xB5635C95FF7296E2ULL,
    0x22AF003AB672E811ULL, 0x52E762596BF68235ULL, 0x9AEBA33AC6ECC6B0ULL, 0x944F6DE09134DFB6ULL,
    0x6C47BEC883A7DE39ULL, 0x6AD047C430A12104ULL, 0xA5B1CFDBA0AB4067ULL, 0x7C45D833AFF07862ULL,
    0x5092EF950A16DA0BULL, 0x9338E69C052B8E7BULL, 0x455A4B4CFE30E3F5ULL, 0x6B02E63195AD0CF8ULL,
    0x6B17B224BAD6BF27ULL, 0xD1E0CCD25BB9C169ULL, 0xDE0C89A556B9AE70ULL, 0x50065E535A213CF6ULL,
    0x9C1169FA2777B874ULL, 0x78EDEFD694AF1EEDULL, 0x6DC93D9526A50E68ULL, 0xEE97F453F06791EDULL,
    0x32AB0EDB696703D3ULL, 0x3A6853C7E70757A7ULL, 0x31865CED6120F37DULL, 0x67FEF95D92607890ULL,
    0x1F2B1D1F15F6DC9CULL, 0xB69E38A8965C6B65ULL, 0xAA9119FF184CCCF4ULL, 0xF43C732873F24C13ULL,
    0xFB4A3D794A9A80D2ULL, 0x3550C2321FD6109CULL, 0x371F77E76BB8417EULL, 0x6BFA9AAE5EC05779ULL,
    0xCD04F3FF001A4778ULL, 0xE3273522064480CAULL, 0x9F91508BFFCFC14AULL, 0x049A7F41061A9E60ULL,
    0xFCB6BE43A9F2FE9BULL, 0x08DE8A1C7797DA9BULL, 0x8F9887E6078735A1ULL, 0xB5B4071DBFC73A66ULL,
    0x230E343DFBA08D33ULL, 0x43ED7F5A0FAE657DULL, 0x3A88A0FBBCB05C63ULL, 0x21874B8B4D2DBC4FULL,
    0x1BDEA12E35F6A8C9ULL, 0x53C065C6C8E63528ULL, 0xE34A1D250E7A8D6BULL, 0xD6B04D3B7651DD7EULL,
    0x5E90277E7CB39E2DULL, 0x2C046F22062DC67DULL, 0xB10BB459132D0A26ULL, 0x3FA9DDFB67E2F199ULL,
    0x0E09B88E1914F7AFULL, 0x10E8B35AF3EEAB37ULL, 0x9EEDECA8E272B933ULL, 0xD4C718BC4AE8AE5FULL,
    0x81536D601170FC20ULL, 0x91B534F885818A06ULL, 0xEC8177F83F900978ULL, 0x190E714FADA5156EULL,
    0xB592BF39B0364963ULL, 0x89C350C893AE7DC1ULL, 0xAC042E70F8B383F2ULL, 0xB49B52E587A1EE60ULL,
    0xFB152FE3FF26DA89ULL, 0x3E666E6F69AE2C15ULL, 0x3B544EBE544C19F9ULL, 0xE805A1E290CF2456ULL,
    0x24B33C9D7ED25117ULL, 0xE74733427B72F0C1ULL, 0x0A804D18B7097475ULL, 0x57E3306D881EDB4FULL,
    0x4AE7D6A36EB5DBCBULL, 0x2D8D5432157064C8ULL, 0xD1E649DE1E7F268BULL, 0x8A328A1CEDFE552CULL,
    0x07A3AEC79624C7DAULL, 0x84547DDC3E203C94ULL, 0x990A98FD5071D263ULL, 0x1A4FF12616EEFC89ULL,
    0xF6F7FD1431714200ULL, 0x30C05B1BA332F41CULL, 0x8D2636B81555A786ULL, 0x46C9FEB55D120902ULL,
    0xCCEC0A73B49C9921ULL, 0x4E9D2827355FC492ULL, 0x19EBB029435DCB0FULL, 0x4659D2B743848A2CULL,
    0x963EF2C96B33BE31ULL, 0x74F85198B05A2E7DULL, 0x5A0F544DD2B1FB18ULL, 0x03727073C2E134B1ULL,
    0xC7F6AA2DE59AEA61ULL, 0x352787BAA0D7C22FULL, 0x9853EAB63B5E0B35ULL, 0xABBDCDD7ED5C0860ULL,
    0xCF05DAF5AC8D77B0ULL, 0x49CAD48CEBF4A71EULL, 0x7A4C10EC2158C4A6ULL, 0xD9E92AA246BF719EULL,
    0x13AE978D09FE5557ULL, 0x730499AF921549FFULL, 0x4E4B705B92903BA4ULL, 0xFF577222C14F0A3AULL,
    0x55B6344CF97AAFAEULL, 0xB862225B055B6960ULL, 0xCAC09AFBDDD2CDB4ULL, 0xDAF8E9829FE96B5FULL,
    0xB5FDFC5D3132C498ULL, 0x310CB380DB6F7503ULL, 0xE87FBB46217A360EULL, 0x2102AE466EBB1148ULL,
    0xF8549E1A3AA5E00DULL, 0x07A69AFDCC42261AULL, 0xC4C118BFE78FEAAEULL, 0xF9F4892ED96BD438ULL,
    0x1AF3DBE25D8F45DAULL, 0xF5B4B0B0D2DEEEB4ULL, 0x962ACEEFA82E1C84ULL, 0x046E3ECAAF453CE9ULL,
    0xF05D129681949A4CULL, 0x964781CE734B3C84ULL, 0x9C2ED44081CE5FBDULL, 0x522E23F3925E319EULL,
    0x177E00F9FC32F791ULL, 0x2BC60A63A6F3B3F2ULL, 0x222BBFAE61725606ULL, 0x486289DDCC3D6780ULL,
    0x7DC7785B8EFDFC80ULL, 0x8AF38731C02BA980ULL, 0x1FAB64EA29A2DDF7ULL, 0xE4D9429322CD065AULL,
    0x9DA058C67844F20CULL, 0x24C0E332B70019B0ULL, 0x233003B5A6CFE6ADULL, 0xD586BD01C5C217F6ULL,
    0x5E5637885F29BC2BULL, 0x7EBA726D8C94094BULL, 0x0A56A5F0BFE39272ULL, 0xD79476A84EE20D06ULL,
    0x9E4C1269BAA4BF37ULL, 0x17EFEE45B0DEE640ULL, 0x1D95B0A5FCF90BC6ULL, 0x93CBE0B699C2585DULL,
    0x65FA4F227A2B6D79ULL, 0xD5F9E858292504D5ULL, 0xC2B5A03F71471A6FULL, 0x59300222B4561E00ULL,
    0xCE2F8642CA0712DCULL, 0x7CA9723FBB2E8988ULL, 0x2785338347F2BA08ULL, 0xC61BB3A141E50E8CULL,
    0x150F361DAB9DEC26ULL, 0x9F6A419D382595F4ULL, 0x64A53DC924FE7AC9ULL, 0x142DE49FFF7A7C3DULL,
    0x0C335248857FA9E7ULL, 0x0A9C32D5EAE45305ULL, 0xE6C42178C4BBB92EULL, 0x71F1CE2490D20B07ULL,
    0xF1BCC3D275AFE51AULL, 0xE728E8C83C334074ULL, 0x96FBF83A12884624ULL, 0x81A1549FD6573DA5ULL,
    0x5FA7867CAF35E149ULL, 0x56986E2EF3ED091BULL, 0x917F1DD5F8886C61ULL, 0xD20D8C88C8FFE65FULL,
    0x31D71DCE64B2C310ULL, 0xF165B587DF898190ULL, 0xA57E6339DD2CF3A0ULL, 0x1EF6E6DBB1961EC9ULL,
    0x70CC73D90BC26E24ULL, 0xE21A6B35DF0C3AD7ULL, 0x003A93D8B2806962ULL, 0x1C99DED33CB890A1ULL,
    0xCF3145DE0ADD4289ULL, 0xD0E4427A5514FB72ULL, 0x77C621CC9FB3A483ULL, 0x67A34DAC4356550BULL,
    0xF8D626AAAF278509ULL
};

const uint64_t isolated_pawns[8] = {
     0b0000001000000010000000100000001000000010000000100000001000000010 ,
     0b0000010100000101000001010000010100000101000001010000010100000101 ,
     0b0000101000001010000010100000101000001010000010100000101000001010 ,
     0b0001010000010100000101000001010000010100000101000001010000010100 ,
     0b0010100000101000001010000010100000101000001010000010100000101000 ,
     0b0101000001010000010100000101000001010000010100000101000001010000 ,
     0b1010000010100000101000001010000010100000101000001010000010100000 ,
     0b0100000001000000010000000100000001000000010000000100000001000000 ,
};

const uint64_t double_pawns[8] = {
     0b0000001000000010000000100000001000000010000000100000001000000010 ,
     0b0000010100000101000001010000010100000101000001010000010100000101 ,
     0b0000101000001010000010100000101000001010000010100000101000001010 ,
     0b0001010000010100000101000001010000010100000101000001010000010100 ,
     0b0010100000101000001010000010100000101000001010000010100000101000 ,
     0b0101000001010000010100000101000001010000010100000101000001010000 ,
     0b1010000010100000101000001010000010100000101000001010000010100000 ,
     0b0100000001000000010000000100000001000000010000000100000001000000 ,
};

const int mg_values[5]       = { 100, 337, 365, 500, 1025 };
const int eg_values[5]       = { 125, 295, 320, 550,  936 };
const int phase_scores[5]    = { 0,  1  ,  1 ,  2 ,  4   };

const int knight_mobility = 2;
const int bishop_mobility = 2;
const int rook_mobility   = 4;
const int queen_mobility  = 2;

uint64_t wking_defence_table[64]            = {0};
uint64_t bking_defence_table[64]            = {0};
uint64_t king_move_lookup_table[64]         = {0};
uint64_t knight_move_lookup_table[64]       = {0};
uint64_t white_pawn_move_lookup_table[64]   = {0};
uint64_t black_pawn_move_lookup_table[64]   = {0};


std::unordered_map<int, std::string> PIECE_NAMES = {
        {0b00000000, " ."},
        {0b00001001, u8"wP"},
        {0b00001010, u8"wN"},
        {0b00001011, u8"wB"},
        {0b00001100, u8"wR"},
        {0b00001101, u8"wQ"},
        {0b00001110, u8"wK"},
        {0b00010001, u8"bP"},
        {0b00010010, u8"bN"},
        {0b00010011, u8"bB"},
        {0b00010100, u8"bR"},
        {0b00010101, u8"bQ"},
        {0b00010110, u8"bK"}
};

std::unordered_map<int, std::string> RANKS = {
        {0, "8"},
        {1, "7"},
        {2, "6"},
        {3, "5"},
        {4, "4"},
        {5, "3"},
        {6, "2"},
        {7, "1"},
};

std::unordered_map<int, std::string> FILES = {
        {0, "a"},
        {1, "b"},
        {2, "c"},
        {3, "d"},
        {4, "e"},
        {5, "f"},
        {6, "g"},
        {7, "h"},
};

void init_move_lookup_tables() {

    for (int square=0; square<64; square++) {

        int file = (square & 7) - 4;
        int rank = (square >> 3) - 4;

        uint64_t knight_mask_to_shift = knight_mask;
        uint64_t king_mask_to_shift = king_mask;
        uint64_t wking_defence_bb = wking_prot_mask;
        uint64_t bking_defence_bb = bking_prot_mask;

        if (file > 0) {
            for (int i = 0; i < file; i++) {
                knight_mask_to_shift    = EAST(knight_mask_to_shift);
                king_mask_to_shift      = EAST(king_mask_to_shift);
                wking_defence_bb        = EAST(wking_defence_bb);
                bking_defence_bb        = EAST(bking_defence_bb);
            }
        } else {
            for (int i = 0; i > file; i--) {
                knight_mask_to_shift    = WEST(knight_mask_to_shift);
                king_mask_to_shift      = WEST(king_mask_to_shift);
                wking_defence_bb        = WEST(wking_defence_bb);
                bking_defence_bb        = WEST(bking_defence_bb);
            }
        }
        if (rank > 0) {
            for (int i = 0; i < rank; i++) {
                knight_mask_to_shift    = NORTH(knight_mask_to_shift);
                king_mask_to_shift      = NORTH(king_mask_to_shift);
                wking_defence_bb        = NORTH(wking_defence_bb);
                bking_defence_bb        = NORTH(bking_defence_bb);
            }
        } else {
            for (int i = 0; i > rank; i--) {
                knight_mask_to_shift    = SOUTH(knight_mask_to_shift);
                king_mask_to_shift      = SOUTH(king_mask_to_shift);
                wking_defence_bb        = SOUTH(wking_defence_bb);
                bking_defence_bb        = SOUTH(bking_defence_bb);
            }
        }

        uint64_t white_pawn = (1ULL << square);

        white_pawn |= (NORTH_EAST(white_pawn) | NORTH_WEST(white_pawn));
        white_pawn ^= (1ULL << square);

        uint64_t black_pawn = (1ULL << square);

        black_pawn |= (SOUTH_EAST(black_pawn) | SOUTH_WEST(black_pawn));
        black_pawn ^= (1ULL << square);

        white_pawn_move_lookup_table[square] = white_pawn;
        black_pawn_move_lookup_table[square] = black_pawn;

        knight_move_lookup_table[square] = knight_mask_to_shift;
        king_move_lookup_table[square] = king_mask_to_shift;

        wking_defence_table[square] = wking_defence_bb;
        bking_defence_table[square] = bking_defence_bb;
    }

    enum { rook, bishop };

    init_sliders_attacks(bishop);
    init_sliders_attacks(rook);
}

class Board {
    private:
    void updateHash() {
        hash = 0;

        for (int i=0; i<6; i++) {
            uint64_t white_bitboard = original_bitboards[i];
            while (white_bitboard) {
                int square = getlsb(white_bitboard);
                hash ^= zobrist_keys[(i<<6)+square];
                white_bitboard &= white_bitboard - 1;
            }
            uint64_t black_bitboard = original_bitboards[i+6];
            while (black_bitboard) {
                int square = getlsb(black_bitboard);
                hash ^= zobrist_keys[((i+6)<<6)+square];
                black_bitboard &= black_bitboard - 1;
            }
        }
        if (white_castling_right & 0b01) {
            hash ^= zobrist_keys[768]; }
        if (white_castling_right & 0b10) {
            hash ^= zobrist_keys[768 + 1]; }
        if (black_castling_right & 0b01) {
            hash ^= zobrist_keys[768 + 2]; }
        if (black_castling_right & 0b10) {
            hash ^= zobrist_keys[768 + 3]; }
        if (side_to_move == WHITE) {
            hash ^= zobrist_keys[780];
        }

    }

    void putPiece(int square, int piece, int color) {
        uint64_t mask = 1ULL << square;
        if (mask & ~pieces[0]) {
            assert(false);
        }
        int offset = (color == WHITE) ? 0 : 6;
        original_bitboards[offset] |= mask;
        updateBoard();
    }

    bool isSquareAttacked(int sq){
        int current_offset;
        int opponent_offset;
        uint64_t pawn_mask;

        if (side_to_move == WHITE) {
            opponent_offset = 6;
            current_offset  = 0;
            pawn_mask       = white_pawn_move_lookup_table[sq];
        } else {
            opponent_offset = 0;
            current_offset  = 6;
            pawn_mask       = black_pawn_move_lookup_table[sq];
        }

        if (pawn_mask & original_bitboards[opponent_offset])
            return true;

        uint64_t occ_bb = (~pieces[0]) ^ original_bitboards[5 + current_offset];

        uint64_t knight_bb = original_bitboards[1 + opponent_offset];
        uint64_t bishop_bb = original_bitboards[2 + opponent_offset];
        uint64_t rook_bb = original_bitboards[3 + opponent_offset];
        uint64_t queen_bb = original_bitboards[4 + opponent_offset];

        uint64_t bishop_rays = get_bishop_attacks(sq, occ_bb);
        if (bishop_rays & bishop_bb || bishop_rays & queen_bb) return true;

        uint64_t rook_rays = get_rook_attacks(sq, occ_bb);
        if (rook_rays & rook_bb || rook_rays & queen_bb) return true;

        if (knight_bb & knight_move_lookup_table[sq]) return true;

        int king_square = getlsb(original_bitboards[5 + opponent_offset]);

        if ((1ULL << sq) & king_move_lookup_table[king_square]) return true;

        return false;
    }

    public:
        uint64_t pieces[3]; 
        uint8_t side_to_move; 
        uint8_t white_castling_right; 
        uint8_t black_castling_right; 
        uint8_t halfmove_clock; 
        uint16_t fullmove_number; 
        uint64_t original_bitboards[12];

        int en_passant_square;

        uint64_t hash = 0;

        Board() {
            side_to_move = WHITE;
            white_castling_right = 3;
            black_castling_right = 3;
            halfmove_clock = 0;
            fullmove_number = 0;
            en_passant_square = -1;
            original_bitboards[0] = 0b0000000000000000000000000000000000000000000000001111111100000000;  
            original_bitboards[1] = 0b0000000000000000000000000000000000000000000000000000000001000010;  
            original_bitboards[2] = 0b0000000000000000000000000000000000000000000000000000000000100100;  
            original_bitboards[3] = 0b0000000000000000000000000000000000000000000000000000000010000001;  
            original_bitboards[4] = 0b0000000000000000000000000000000000000000000000000000000000001000;  
            original_bitboards[5] = 0b0000000000000000000000000000000000000000000000000000000000010000;  
            original_bitboards[6] = original_bitboards[0] << 40;
            original_bitboards[7] = original_bitboards[1] << 56;
            original_bitboards[8] = original_bitboards[2] << 56;
            original_bitboards[9] = original_bitboards[3] << 56;
            original_bitboards[10] = original_bitboards[4] << 56;
            original_bitboards[11] = original_bitboards[5] << 56;

            updateBoard();
            updateHash();

        }

        Board(const Board& other) {

            side_to_move = other.side_to_move;
            white_castling_right = other.white_castling_right;
            black_castling_right = other.black_castling_right;
            halfmove_clock = other.halfmove_clock;
            fullmove_number = other.fullmove_number;
            en_passant_square = other.en_passant_square;
            hash = other.hash;

            memcpy(original_bitboards, other.original_bitboards, sizeof(uint64_t)*12);

            updateBoard();
        }

        void reset() {
            side_to_move = WHITE;
            white_castling_right = 3;
            black_castling_right = 3;
            halfmove_clock = 0;
            fullmove_number = 0;
            en_passant_square = -1;
            original_bitboards[0] = 0b0000000000000000000000000000000000000000000000001111111100000000;  
            original_bitboards[1] = 0b0000000000000000000000000000000000000000000000000000000001000010;  
            original_bitboards[2] = 0b0000000000000000000000000000000000000000000000000000000000100100;  
            original_bitboards[3] = 0b0000000000000000000000000000000000000000000000000000000010000001;  
            original_bitboards[4] = 0b0000000000000000000000000000000000000000000000000000000000001000;  
            original_bitboards[5] = 0b0000000000000000000000000000000000000000000000000000000000010000;  
            original_bitboards[6] = original_bitboards[0] << 40;
            original_bitboards[7] = original_bitboards[1] << 56;
            original_bitboards[8] = original_bitboards[2] << 56;
            original_bitboards[9] = original_bitboards[3] << 56;
            original_bitboards[10] = original_bitboards[4] << 56;
            original_bitboards[11] = original_bitboards[5] << 56;

            pieces[1] = original_bitboards[0] | original_bitboards[1] | original_bitboards[2] | original_bitboards[3] | original_bitboards[4] | original_bitboards[5];
            pieces[2] = original_bitboards[6] | original_bitboards[7] | original_bitboards[8] | original_bitboards[9] | original_bitboards[10] | original_bitboards[11];
            pieces[0] = ~(pieces[1] | pieces[2]);

            hash = 0;
        }

        void printChessBoard() {
            std::string b1 = (side_to_move == WHITE ? "=====================================" : "=============== move ================");
            std::string b2 = (side_to_move == WHITE ? "=============== move ================" : "=====================================");
            std::cout << b1 << std::endl << "||                                 ||" << std::endl;
            char pieceChars[7] = {' ', 'P', 'N', 'B', 'R', 'Q', 'K'};
            char sideChars[3] = {' ', 'W', 'B'};
            std::cout << "||      a  b  c  d  e  f  g  h     ||\n";
            for (int rank = 7; rank >= 0; rank--) {
                std::cout << "||   " << rank + 1 << "  ";
                for (int file = 0; file < 8; file++) {
                    int square = rank * 8 + file;
                    int piece = getSquare(square);
                    std::cout << PIECE_NAMES[piece] << " ";
                }
                std::cout << "   ||" << std::endl << "||                                 ||" << std::endl;
            }
            std::cout << "||      a  b  c  d  e  f  g  h     ||\n"<< "||                                 ||" << std::endl;
            std::cout << b2 << std::endl << std::endl;
        }

        int getSquare(int square) {
            uint64_t mask = 1ULL << square;
            if (pieces[0] & mask) { return EMPTY; } 

            for (int i=1; i<7; i++) {   
                if          (original_bitboards[i-1] & mask)    { return WHITE | i ; }
                else if     (original_bitboards[i+5] & mask)    { return BLACK | i ; }
            }
            std::cout << "GET SQUARE FAILED:" << square << std::endl;
            assert(false); 
            return EMPTY;
        }

        bool isInCheck(){
            int sq;
            int current_offset;
            int opponent_offset;
            uint64_t pawn_mask;

            if (side_to_move == WHITE) {
                opponent_offset = 6;
                current_offset  = 0;
                sq              = getlsb(original_bitboards[5 + current_offset]);
                pawn_mask       = white_pawn_move_lookup_table[sq];
            } else {
                opponent_offset = 0;
                current_offset  = 6;
                sq              = getlsb(original_bitboards[5 + current_offset]);
                pawn_mask       = black_pawn_move_lookup_table[sq];
            }

            if (pawn_mask & original_bitboards[opponent_offset])
                return true;

            uint64_t occ_bb = (~pieces[0]) ^ original_bitboards[5 + current_offset];

            uint64_t knight_bb  = original_bitboards[1 + opponent_offset];
            uint64_t bishop_bb  = original_bitboards[2 + opponent_offset];
            uint64_t rook_bb    = original_bitboards[3 + opponent_offset];
            uint64_t queen_bb   = original_bitboards[4 + opponent_offset];

            uint64_t bishop_rays = get_bishop_attacks(sq, occ_bb);
            if (bishop_rays & bishop_bb || bishop_rays & queen_bb) return true;

            uint64_t rook_rays = get_rook_attacks(sq, occ_bb);
            if (rook_rays & rook_bb || rook_rays & queen_bb) return true;

            if (knight_bb & knight_move_lookup_table[sq]) return true;

            int king_square = getlsb(original_bitboards[5 + opponent_offset]);

            if ((1ULL << sq) & king_move_lookup_table[king_square]) return true;

            return false;

        }

        int scoreMove(uint16_t move, uint32_t entry, int depth, int pdepth=0);
        int scoreCaptureMove(uint16_t move, uint32_t entry);
        int scoreThreadMove(uint16_t move, uint32_t entry);

        int getScore() {

            int score = 0;

            uint64_t white_pawns = original_bitboards[0];
            uint64_t black_pawns = original_bitboards[6];
            uint64_t white_pawns_copy = white_pawns;
            uint64_t black_pawns_copy = black_pawns;
            uint64_t wpawn_space = 0;
            uint64_t bpawn_space = 0;
            while (white_pawns){
                int sq = getlsb(white_pawns);
                int file = sq & 7;
                white_pawns &= white_pawns - 1;
                uint64_t adjacent_pawns = white_pawns_copy & isolated_pawns[file];
                uint64_t pawn_tower = bitcount(white_pawns_copy & double_pawns[file]);
                bool passed_pawn = !bitcount(black_pawns_copy & ((double_pawns[file] | isolated_pawns[file])));
                if (passed_pawn) score += 45;
                if (!adjacent_pawns) score -= 20;
                if (pawn_tower > 1) score -= 10 * pawn_tower;
                wpawn_space |= white_pawn_move_lookup_table[sq];
            }
            while (black_pawns){
                int sq = getlsb(black_pawns);
                int file = sq & 7;
                black_pawns &= black_pawns - 1;
                uint64_t adjacent_pawns = black_pawns_copy & isolated_pawns[file];
                uint64_t pawn_tower = bitcount(black_pawns_copy & double_pawns[file]);
                bool passed_pawn = !bitcount(white_pawns_copy & ((double_pawns[file] | isolated_pawns[file])));
                if (passed_pawn) score -= 45;
                if (!adjacent_pawns) score += 20;
                if (pawn_tower > 1) score += 10 * pawn_tower;
                bpawn_space |= black_pawn_move_lookup_table[sq];
            }
            score += bitcount(wpawn_space) - bitcount(bpawn_space);

            int game_phase = 0;

            uint64_t white_king = original_bitboards[5];
            uint64_t black_king = original_bitboards[11];

            int wking_square = getlsb(white_king);
            uint64_t wking_defence_bb = wking_defence_table[wking_square];
            score -= bitcount(wking_defence_bb & pieces[0]) * 8;
            score -= bitcount(wking_defence_bb & pieces[2]) * 16;
            score += bitcount(wking_defence_bb & pieces[1]) * 12;

            int bking_square = getlsb(black_king);
            uint64_t bking_defence_bb = bking_defence_table[bking_square];
            score += bitcount(bking_defence_bb & pieces[0]) * 8;
            score += bitcount(bking_defence_bb & pieces[1]) * 16;
            score -= bitcount(bking_defence_bb & pieces[2]) * 12;

            int mg_score = king_pos_middle[FLIP(wking_square)];
            int eg_score = king_pos_end[FLIP(wking_square)];
            mg_score -= king_pos_middle[bking_square];
            eg_score -= king_pos_end[bking_square];

            for (int i=0; i<5; i++) {
                int wbitcount = bitcount(original_bitboards[i]);
                int bbitcount = bitcount(original_bitboards[6+i]);
                score += pst(original_bitboards[i], white_pst_table[i]) ;
                score -= pst(original_bitboards[6+i], black_pst_table[i]) ;
                game_phase  += phase_scores[i];
                mg_score += mg_values[i] * (wbitcount - bbitcount);
                eg_score += eg_values[i] * (wbitcount - bbitcount);
            }

            uint64_t occupied = ~pieces[0];

            uint64_t wknights = original_bitboards[1];
            uint64_t bknights = original_bitboards[7];
            uint64_t wbishops = original_bitboards[2];
            uint64_t bbishops = original_bitboards[8];
            uint64_t wrooks = original_bitboards[3];
            uint64_t brooks = original_bitboards[9];
            uint64_t wqueen = original_bitboards[4];
            uint64_t bqueen = original_bitboards[10];

            // uint64_t wvalp = wknights | wbishops | wrooks | white_king;
            // uint64_t bvalp = bknights | bbishops | brooks | black_king;

            wpawn_space = ~wpawn_space;
            bpawn_space = ~bpawn_space;

            while (wknights) {
                int sq = getlsb(wknights);
                uint64_t moves = knight_move_lookup_table[sq] & (~pieces[1]) & bpawn_space;
                mg_score += bitcount(moves) * knight_mobility / bitcount(wknights);
                // score += bitcount(moves & bvalp) * 6;
                wknights ^= (1ULL << sq);
            }
            while (bknights) {
                int sq = getlsb(bknights);
                uint64_t moves = knight_move_lookup_table[sq] & (~pieces[2]) & wpawn_space;
                mg_score -= bitcount(moves) * knight_mobility / bitcount(bknights);
                // score -= bitcount(moves & wvalp) * 6;
                bknights ^= (1ULL << sq);
            }
            while (wbishops) {
                int sq = getlsb(wbishops);
                uint64_t moves = get_bishop_attacks(sq, occupied) & (~pieces[1]) & bpawn_space;
                mg_score += bitcount(moves) * bishop_mobility / bitcount(wbishops);
                // score += bitcount(moves & bvalp) * 6;
                wbishops ^= (1ULL << sq);
            }
            while (bbishops) {
                int sq = getlsb(bbishops);
                uint64_t moves = get_bishop_attacks(sq, occupied) & (~pieces[2]) & wpawn_space;
                mg_score -= bitcount(moves) * bishop_mobility / bitcount(bbishops);
                // score -= bitcount(moves & wvalp) * 6;
                bbishops ^= (1ULL << sq);
            }
            while (wrooks) {
                int sq = getlsb(wrooks);
                uint64_t moves = get_rook_attacks(sq, occupied) & (~pieces[1]) & bpawn_space;
                mg_score += bitcount(moves) * rook_mobility / bitcount(wrooks);
                if (!(double_pawns[sq & 7] & pieces[1])) score += 15;
                // score += bitcount(moves & bvalp) * 4;
                wrooks ^= (1ULL << sq);
            }
            while (brooks) {
                int sq = getlsb(brooks);
                uint64_t moves = get_rook_attacks(sq, occupied) & (~pieces[2]) & wpawn_space;
                mg_score -= bitcount(moves) * rook_mobility / bitcount(brooks);
                if (!(double_pawns[sq & 7] & pieces[3])) score -= 15;
                // score -= bitcount(moves & wvalp) * 4;
                brooks ^= (1ULL << sq);
            }
            while (wqueen) {
                int sq = getlsb(wqueen);
                wqueen ^= (1ULL << sq);
                uint64_t moves = (get_rook_attacks(sq, occupied) | get_bishop_attacks(sq, occupied)) & (~pieces[1]) & bpawn_space;
                mg_score += bitcount(moves) * queen_mobility;
                // score += bitcount(moves & bvalp) * 2;
            }
            while (bqueen) {
                int sq = getlsb(bqueen);
                bqueen ^= (1ULL << sq);
                uint64_t moves = (get_rook_attacks(sq, occupied) | get_bishop_attacks(sq, occupied)) & (~pieces[2]) & wpawn_space;
                mg_score -= bitcount(moves) * queen_mobility;
                // score -= bitcount(moves & wvalp) * 2;
            }

            if (side_to_move == WHITE)
                 mg_score += 18;
            else mg_score -= 18;

            game_phase = std::min(game_phase, 24);
            score += ((mg_score * game_phase) + (eg_score * (24-game_phase)))/24;

            return score;

        }

        int pawnStructureScore() {
            int score = 0;
            uint64_t white_pawns = original_bitboards[0];
            uint64_t black_pawns = original_bitboards[6];
            uint64_t white_pawns_copy = white_pawns;
            uint64_t black_pawns_copy = black_pawns;
            uint64_t wpawn_space = 0;
            uint64_t bpawn_space = 0;
            while (white_pawns){
                int sq = getlsb(white_pawns);
                int file = sq & 7;
                white_pawns &= white_pawns - 1;
                uint64_t adjacent_pawns = white_pawns_copy & isolated_pawns[file];
                uint64_t pawn_tower = bitcount(white_pawns_copy & double_pawns[file]);
                bool passed_pawn = !bitcount(black_pawns_copy & ((double_pawns[file] | isolated_pawns[file])));
                if (passed_pawn) score += 45;
                if (!adjacent_pawns) score -= 20;
                if (pawn_tower > 1) score -= 10 * pawn_tower;
                wpawn_space |= white_pawn_move_lookup_table[sq];
            }
            while (black_pawns){
                int sq = getlsb(black_pawns);
                int file = sq & 7;
                black_pawns &= black_pawns - 1;
                uint64_t adjacent_pawns = black_pawns_copy & isolated_pawns[file];
                uint64_t pawn_tower = bitcount(black_pawns_copy & double_pawns[file]);
                bool passed_pawn = !bitcount(white_pawns_copy & ((double_pawns[file] | isolated_pawns[file])));
                if (passed_pawn) score -= 45;
                if (!adjacent_pawns) score += 20;
                if (pawn_tower > 1) score += 10 * pawn_tower;
                bpawn_space |= black_pawn_move_lookup_table[sq];
            }
            score += 2 * (bitcount(wpawn_space) - bitcount(bpawn_space));
            return score;
        }

        void pawnStructureScoreDebug() {
            uint64_t white_pawns = original_bitboards[0];
            uint64_t black_pawns = original_bitboards[6];
            uint64_t white_pawns_copy = white_pawns;
            uint64_t black_pawns_copy = black_pawns;
            uint64_t wpawn_space = 0;
            uint64_t bpawn_space = 0;
            uint64_t white_passed_pawns = 0;
            uint64_t black_passed_pawns = 0;
            uint64_t white_doubled_pawns = 0;
            uint64_t black_doubled_pawns = 0;
            uint64_t white_isolated_pawns = 0;
            uint64_t black_isolated_pawns = 0;
            uint64_t white_connected_pawns = 0;
            uint64_t black_connected_pawns = 0;
            while (white_pawns){
                int sq = getlsb(white_pawns);
                int file = sq & 7;
                int rank = sq >> 3;
                white_pawns &= white_pawns - 1;
                uint64_t adjacent_pawns = white_pawns_copy & isolated_pawns[file];

                if (adjacent_pawns)
                    white_connected_pawns |= (1ULL << sq) | adjacent_pawns;
                else
                    white_isolated_pawns |= (1ULL << sq);

                uint64_t pawn_tower = white_pawns_copy & double_pawns[file];
                if (bitcount(pawn_tower) >= 2)
                    white_doubled_pawns |= pawn_tower;

                std::cout << "White passed pawn mask\n";
                printBitboard((double_pawns[file] | isolated_pawns[file]) & (filled_board << ((rank + 1) << 3)));

                uint64_t unpassed_pawn = black_pawns_copy & ((double_pawns[file] | isolated_pawns[file]) & (filled_board << ((rank + 1) << 3)));
                if (!bitcount(unpassed_pawn))
                    white_passed_pawns |= (1ULL << sq);

                wpawn_space |= white_pawn_move_lookup_table[sq];
            }
            while (black_pawns){
                int sq = getlsb(black_pawns);
                int file = sq & 7;
                int rank = sq >> 3;
                black_pawns &= black_pawns - 1;
                uint64_t adjacent_pawns = black_pawns_copy & isolated_pawns[file];

                if (adjacent_pawns)
                    black_connected_pawns |= (1ULL << sq) | adjacent_pawns;
                else
                    black_isolated_pawns |= (1ULL << sq);

                uint64_t pawn_tower = black_pawns_copy & double_pawns[file];
                if (bitcount(pawn_tower) >= 2)
                    black_doubled_pawns |= pawn_tower;

                std::cout << "Black passed pawn mask\n";
                printBitboard((double_pawns[file] | isolated_pawns[file]) & (filled_board >> ((8 - rank) << 3)));

                uint64_t unpassed_pawn = white_pawns_copy & ((double_pawns[file] | isolated_pawns[file]) & (filled_board >> ((8 - rank) << 3)));
                if (!bitcount(unpassed_pawn))
                    black_passed_pawns |= (1ULL << sq);

                bpawn_space |= black_pawn_move_lookup_table[sq];
            }

            std::cout << "White pawn space:\n";
            printBitboard(wpawn_space);
            std::cout << "Black pawn space:\n";
            printBitboard(bpawn_space);
            std::cout << "White passed pawns:\n";
            printBitboard(white_passed_pawns);
            std::cout << "Black passed pawns:\n";
            printBitboard(black_passed_pawns);
            std::cout << "White doubled pawns:\n";
            printBitboard(white_doubled_pawns);
            std::cout << "Black doubled pawns:\n";
            printBitboard(black_doubled_pawns);
            std::cout << "White isolated pawns:\n";
            printBitboard(white_isolated_pawns);
            std::cout << "Black isolated pawns:\n";
            printBitboard(black_isolated_pawns);
            std::cout << "White connected pawns:\n";
            printBitboard(white_connected_pawns);
            std::cout << "Black connected pawns:\n";
            printBitboard(black_connected_pawns);
        }

        void updateBoard() {
            pieces[1] = original_bitboards[0] | original_bitboards[1] | original_bitboards[2] | original_bitboards[3] | original_bitboards[4] | original_bitboards[5];
            pieces[2] = original_bitboards[6] | original_bitboards[7] | original_bitboards[8] | original_bitboards[9] | original_bitboards[10] | original_bitboards[11];
            pieces[0] = ~(pieces[1] | pieces[2]);
        }

        uint16_t * generateMoves(int depth, int pdepth=0) {
            updateBoard();

            uint16_t* moves = new uint16_t[256];
            memset(moves, 0, 256*sizeof(*moves));

            int append_index = 0;

            int curr_color = (side_to_move >> 3) & 3;

            uint64_t occp_curr_side = pieces[curr_color];
            uint64_t occp_opp_side = pieces[~(curr_color) & 3];

            int piece_index_offset = (side_to_move == WHITE ? 0 : 6);

            uint64_t pawns_bb = original_bitboards[0 + piece_index_offset];
            uint64_t knights_bb = original_bitboards[1 + piece_index_offset];
            uint64_t king_bb = original_bitboards[5 + piece_index_offset];

            while (pawns_bb) {

                int square = getlsb(pawns_bb);
                pawns_bb &= pawns_bb - 1;
                int shift_multiplier = ((side_to_move == WHITE) ? 1 : -1);
                int next_square = square + shift_multiplier * 8;
                int left_capture_square = square + shift_multiplier * 7;
                int right_capture_square = square + shift_multiplier * 9;

                if (left_capture_square >= 0 && left_capture_square < 64 && (abs((square>>3) - (left_capture_square>>3)) == 1)) {
                        uint64_t target_mask = 1ULL << left_capture_square;

                        if (occp_opp_side & target_mask) {
                            moves[append_index] = ENCODE_MOVE(square, left_capture_square);
                            append_index++;
                        }
                }

                if (right_capture_square >= 0 && right_capture_square < 64 && (abs((square>>3) - (right_capture_square>>3)) == 1)) {
                        uint64_t target_mask = 1ULL << right_capture_square;

                        if (occp_opp_side & target_mask) {
                            moves[append_index] = ENCODE_MOVE(square, right_capture_square);
                            append_index++;
                        }
                }

                if (en_passant_square != -1) {
                    int file = (square & 7);
                    int rank = (square >> 3);
                    int en_passant_file = en_passant_square & 7;
                    int en_passant_rank = (en_passant_square >> 3) & 7;

                    if (en_passant_rank == (rank + shift_multiplier) && ((en_passant_file == file - 1 && file != 0) || en_passant_file == file + 1 && file != 7)) {

                        moves[append_index] = ENCODE_MOVE(square, en_passant_square);
                        append_index++;
                    }
                }

                if (!(pieces[0] & leftShift(1ULL, shift_multiplier * (square + shift_multiplier * 8)))) { continue; }
                moves[append_index] = ENCODE_MOVE(square, square + shift_multiplier * 8);
                append_index++;
                if ((square < 16 && square > 7 && shift_multiplier == 1) || (square > 47 && square < 56 && shift_multiplier == -1)) {
                    if (!(pieces[0] & leftShift(1ULL, shift_multiplier * (square + shift_multiplier * 16)))) { continue; }
                    moves[append_index] = ENCODE_MOVE(square, square + shift_multiplier * 16);
                    append_index++; 
                }
            }

            while (knights_bb) {

                int square = getlsb(knights_bb);
                knights_bb &= knights_bb - 1;
                uint64_t mask = knight_move_lookup_table[square];
                while (mask != 0) {

                    int target_square = getlsb(mask);
                    mask &= mask - 1;
                    if (occp_curr_side & (1ULL << target_square)) { continue; }
                    moves[append_index] = ENCODE_MOVE(square, target_square);
                    append_index++;
                }
            }

            if(king_bb == 0) {
                memset(moves, 0, 256*sizeof(*moves));
                return moves;
            }
            int king_square = getlsb(king_bb);
            uint64_t mask = king_move_lookup_table[king_square];
            while (mask != 0) {
                int target_king_square = getlsb(mask);
                mask &= mask - 1;
                if (occp_curr_side & (1ULL << target_king_square)) { continue; }
                if (isSquareAttacked(target_king_square)) continue;
                moves[append_index] = ENCODE_MOVE(king_square, target_king_square);
                append_index++;
            }
            if (side_to_move == WHITE) {
                if (white_castling_right) {
                    if ((white_castling_right == 1 || white_castling_right == 3) && ((white_short_castling & pieces[0]) == white_short_castling)) {
                        if (!isSquareAttacked(6) && !isSquareAttacked(5) && !isInCheck()){
                            moves[append_index] = ENCODE_MOVE(king_square, 6);
                            append_index++;
                        }
                    }
                    if ((white_castling_right == 2 || white_castling_right == 3) && ((white_long_castling & pieces[0]) == white_long_castling)) {
                        if (!isSquareAttacked(2) && !isSquareAttacked(3) && !isInCheck()){
                            moves[append_index] = ENCODE_MOVE(king_square, 2);
                            append_index++;
                        }
                    }                    
                }                                              
            } else {
                if (black_castling_right) {
                    if ((black_castling_right == 1 || black_castling_right == 3) && ((black_short_castling & pieces[0]) == black_short_castling)) {
                        if (!isSquareAttacked(62) && !isSquareAttacked(61) && !isInCheck()){
                            moves[append_index] = ENCODE_MOVE(king_square, 62);
                            append_index++;
                        }
                    }
                    if ((black_castling_right == 2 || black_castling_right == 3) && ((black_long_castling & pieces[0]) == black_long_castling)) {
                        if (!isSquareAttacked(58) && !isSquareAttacked(59) && !isInCheck()){
                            moves[append_index] = ENCODE_MOVE(king_square, 58);
                            append_index++;
                        }
                    }
                }  
            }

            uint64_t bishop_bb = original_bitboards[2 + piece_index_offset];
            uint64_t rook_bb = original_bitboards[3 + piece_index_offset];
            uint64_t queen_bb = original_bitboards[4 + piece_index_offset];

            while (bishop_bb) {

                int square = getlsb(bishop_bb);
                bishop_bb &= bishop_bb - 1;
                uint64_t b_attacks = get_bishop_attacks(square, ~pieces[0]);
                while (b_attacks) {

                    int target_square = getlsb(b_attacks);
                    b_attacks &= b_attacks - 1;
                    if (pieces[side_to_move >> 3] & (1ULL << target_square)) {
                        continue;
                    } else {
                        moves[append_index] = ENCODE_MOVE(square, target_square);
                        append_index++;
                    }
                }
            }

            while (rook_bb) {

                int square = getlsb(rook_bb);
                rook_bb &= rook_bb - 1;
                uint64_t r_attacks = get_rook_attacks(square, ~pieces[0]);
                while (r_attacks) {

                    int target_square = getlsb(r_attacks);
                    r_attacks &= r_attacks - 1;
                    if (pieces[side_to_move >> 3] & (1ULL << target_square)) {
                        continue;
                    } else {
                        moves[append_index] = ENCODE_MOVE(square, target_square);
                        append_index++;
                    }

                }
            }

            while (queen_bb) {

                int square = getlsb(queen_bb);
                queen_bb &= queen_bb - 1;
                uint64_t q_attacks = get_bishop_attacks(square, ~pieces[0]) | get_rook_attacks(square, ~pieces[0]);
                while (q_attacks) {

                    int target_square = getlsb(q_attacks);
                    q_attacks &= q_attacks - 1;
                    if (pieces[side_to_move >> 3] & (1ULL << target_square)) {
                        continue;
                    } else {
                        moves[append_index] = ENCODE_MOVE(square, target_square);
                        append_index++;
                    }
                }
            }

            struct ScoredMove {
                uint16_t move;
                int score;
            };
            ScoredMove scored_moves[256];
            memset(scored_moves, 0, 256*sizeof(*scored_moves));
            uint32_t entry = this->hash % table_size;
            for (int i = 0; i < append_index; i++) {
                uint16_t move = moves[i];
                scored_moves[i] = { move, scoreMove(move, entry, depth, pdepth) };
            }
            std::sort(scored_moves, scored_moves + append_index,
                [](const ScoredMove& a, const ScoredMove& b) {
                    return a.score > b.score;
                });
            for (int i = 0; i < append_index; i++) {
                moves[i] = scored_moves[i].move;
            }

            return moves;
        }

        uint16_t * generateMovesWithoutSorting() {
            updateBoard();

            uint16_t* moves = new uint16_t[256];
            memset(moves, 0, 256*sizeof(*moves));

            int append_index = 0;

            int curr_color = (side_to_move >> 3) & 3;

            uint64_t occp_curr_side = pieces[curr_color];
            uint64_t occp_opp_side = pieces[~(curr_color) & 3];

            int piece_index_offset = (side_to_move == WHITE ? 0 : 6);

            uint64_t pawns_bb = original_bitboards[0 + piece_index_offset];
            uint64_t knights_bb = original_bitboards[1 + piece_index_offset];
            uint64_t king_bb = original_bitboards[5 + piece_index_offset];

            while (pawns_bb) {

                int square = getlsb(pawns_bb);
                pawns_bb &= pawns_bb - 1;
                int shift_multiplier = ((side_to_move == WHITE) ? 1 : -1);
                int next_square = square + shift_multiplier * 8;
                int left_capture_square = square + shift_multiplier * 7;
                int right_capture_square = square + shift_multiplier * 9;

                if (left_capture_square >= 0 && left_capture_square < 64 && (abs((square>>3) - (left_capture_square>>3)) == 1)) {
                        uint64_t target_mask = 1ULL << left_capture_square;

                        if (occp_opp_side & target_mask) {
                            moves[append_index] = ENCODE_MOVE(square, left_capture_square);
                            append_index++;
                        }
                }

                if (right_capture_square >= 0 && right_capture_square < 64 && (abs((square>>3) - (right_capture_square>>3)) == 1)) {
                        uint64_t target_mask = 1ULL << right_capture_square;

                        if (occp_opp_side & target_mask) {
                            moves[append_index] = ENCODE_MOVE(square, right_capture_square);
                            append_index++;
                        }
                }

                if (en_passant_square != -1) {
                    int file = (square & 7);
                    int rank = (square >> 3);
                    int en_passant_file = en_passant_square & 7;
                    int en_passant_rank = (en_passant_square >> 3) & 7;

                    if (en_passant_rank == (rank + shift_multiplier) && ((en_passant_file == file - 1 && file != 0) || en_passant_file == file + 1 && file != 7)) {

                        moves[append_index] = ENCODE_MOVE(square, en_passant_square);
                        append_index++;
                    }
                }

                if (!(pieces[0] & leftShift(1ULL, shift_multiplier * (square + shift_multiplier * 8)))) { continue; }
                moves[append_index] = ENCODE_MOVE(square, square + shift_multiplier * 8);
                append_index++;
                if ((square < 16 && square > 7 && shift_multiplier == 1) || (square > 47 && square < 56 && shift_multiplier == -1)) {
                    if (!(pieces[0] & leftShift(1ULL, shift_multiplier * (square + shift_multiplier * 16)))) { continue; }
                    moves[append_index] = ENCODE_MOVE(square, square + shift_multiplier * 16);
                    append_index++; 
                }
            }

            while (knights_bb) {

                int square = getlsb(knights_bb);
                knights_bb &= knights_bb - 1;
                uint64_t mask = knight_move_lookup_table[square];
                while (mask != 0) {

                    int target_square = getlsb(mask);
                    mask &= mask - 1;
                    if (occp_curr_side & (1ULL << target_square)) { continue; }
                    moves[append_index] = ENCODE_MOVE(square, target_square);
                    append_index++;
                }
            }

            if(king_bb == 0) {
                memset(moves, 0, 256*sizeof(*moves));
                return moves;
            }
            int king_square = getlsb(king_bb);
            uint64_t mask = king_move_lookup_table[king_square];
            while (mask != 0) {
                int target_king_square = getlsb(mask);
                mask &= mask - 1;
                if (occp_curr_side & (1ULL << target_king_square)) { continue; }
                if (isSquareAttacked(target_king_square)) continue;
                moves[append_index] = ENCODE_MOVE(king_square, target_king_square);
                append_index++;
            }
            if (side_to_move == WHITE) {
                if (white_castling_right) {
                    if ((white_castling_right == 1 || white_castling_right == 3) && ((white_short_castling & pieces[0]) == white_short_castling)) {
                        if (!isSquareAttacked(6) && !isSquareAttacked(5) && !isInCheck()){
                            moves[append_index] = ENCODE_MOVE(king_square, 6);
                            append_index++;
                        }
                    }
                    if ((white_castling_right == 2 || white_castling_right == 3) && ((white_long_castling & pieces[0]) == white_long_castling)) {
                        if (!isSquareAttacked(2) && !isSquareAttacked(3) && !isInCheck()){
                            moves[append_index] = ENCODE_MOVE(king_square, 2);
                            append_index++;
                        }
                    }                    
                }                                              
            } else {
                if (black_castling_right) {
                    if ((black_castling_right == 1 || black_castling_right == 3) && ((black_short_castling & pieces[0]) == black_short_castling)) {
                        if (!isSquareAttacked(62) && !isSquareAttacked(61) && !isInCheck()){
                            moves[append_index] = ENCODE_MOVE(king_square, 62);
                            append_index++;
                        }
                    }
                    if ((black_castling_right == 2 || black_castling_right == 3) && ((black_long_castling & pieces[0]) == black_long_castling)) {
                        if (!isSquareAttacked(58) && !isSquareAttacked(59) && !isInCheck()){
                            moves[append_index] = ENCODE_MOVE(king_square, 58);
                            append_index++;
                        }
                    }
                }  
            }

            uint64_t bishop_bb = original_bitboards[2 + piece_index_offset];
            uint64_t rook_bb = original_bitboards[3 + piece_index_offset];
            uint64_t queen_bb = original_bitboards[4 + piece_index_offset];

            while (bishop_bb) {

                int square = getlsb(bishop_bb);
                bishop_bb &= bishop_bb - 1;
                uint64_t b_attacks = get_bishop_attacks(square, ~pieces[0]);
                while (b_attacks) {

                    int target_square = getlsb(b_attacks);
                    b_attacks &= b_attacks - 1;
                    if (pieces[side_to_move >> 3] & (1ULL << target_square)) {
                        continue;
                    } else {
                        moves[append_index] = ENCODE_MOVE(square, target_square);
                        append_index++;
                    }
                }
            }

            while (rook_bb) {

                int square = getlsb(rook_bb);
                rook_bb &= rook_bb - 1;
                uint64_t r_attacks = get_rook_attacks(square, ~pieces[0]);
                while (r_attacks) {

                    int target_square = getlsb(r_attacks);
                    r_attacks &= r_attacks - 1;
                    if (pieces[side_to_move >> 3] & (1ULL << target_square)) {
                        continue;
                    } else {
                        moves[append_index] = ENCODE_MOVE(square, target_square);
                        append_index++;
                    }

                }
            }

            while (queen_bb) {

                int square = getlsb(queen_bb);
                queen_bb &= queen_bb - 1;
                uint64_t q_attacks = get_bishop_attacks(square, ~pieces[0]) | get_rook_attacks(square, ~pieces[0]);
                while (q_attacks) {

                    int target_square = getlsb(q_attacks);
                    q_attacks &= q_attacks - 1;
                    if (pieces[side_to_move >> 3] & (1ULL << target_square)) {
                        continue;
                    } else {
                        moves[append_index] = ENCODE_MOVE(square, target_square);
                        append_index++;
                    }
                }
            }

            struct ScoredMove {
                uint16_t move;
                int score;
            };
            ScoredMove scored_moves[256];
            memset(scored_moves, 0, 256*sizeof(*scored_moves));
            uint32_t entry = this->hash % table_size;
            for (int i = 0; i < append_index; i++) {
                uint16_t move = moves[i];
                scored_moves[i] = { move, scoreThreadMove(move, entry) };
            }
            std::sort(scored_moves, scored_moves + append_index,
                [](const ScoredMove& a, const ScoredMove& b) {
                    return a.score > b.score;
                });
            for (int i = 0; i < append_index; i++) {
                moves[i] = scored_moves[i].move;
            }

            return moves;
        }

        void makeMove(int fromSquare, int toSquare) {

                 if (toSquare == 0)  white_castling_right &= ~0b10;
            else if (toSquare == 7)  white_castling_right &= ~0b01;
            else if (toSquare == 56) black_castling_right &= ~0b10;
            else if (toSquare == 63) black_castling_right &= ~0b01;

                 if (fromSquare == 0)  white_castling_right &= ~0b10;
            else if (fromSquare == 7)  white_castling_right &= ~0b01;
            else if (fromSquare == 56) black_castling_right &= ~0b10;
            else if (fromSquare == 63) black_castling_right &= ~0b01;

            uint8_t piece = getSquare(fromSquare);
            uint8_t piece_color = piece & 0b11000;
            if(piece_color != side_to_move) { return; }
            uint8_t piece_type = (piece & 0b00000111) - 1;
            uint8_t target_square = getSquare(toSquare);
            uint8_t target_piece_type = target_square & 0b00000111;

            if ((piece & 0b00000111) == PAWN && (toSquare < 8 || toSquare > 55)) {
                original_bitboards[piece_type + (side_to_move == WHITE ? 0 : 6)] &= ~(1ULL << fromSquare);
                original_bitboards[4 + (piece_color == WHITE ? 0 : 6)] |= (1ULL << toSquare);
                if (target_piece_type) {
                    original_bitboards[target_piece_type + (side_to_move == WHITE ? 5 : -1)] &= ~(1ULL << toSquare);
                }
                updateHash();
                goto jump_label;
            }

            if(side_to_move == WHITE) {
                original_bitboards[piece_type] &= ~(1ULL << fromSquare);
                original_bitboards[piece_type] |= (1ULL << toSquare);
                hash ^= zobrist_keys[(piece_type<<6)+fromSquare];
                hash ^= zobrist_keys[(piece_type<<6)+toSquare];
                if (target_piece_type) {
                    original_bitboards[target_piece_type + 5] &= ~(1ULL << toSquare);
                    hash ^= zobrist_keys[(target_piece_type<<6)+toSquare];
                }

            } else { 
                original_bitboards[piece_type + 6] &= ~(1ULL << fromSquare);
                original_bitboards[piece_type + 6] |= (1ULL << toSquare);
                hash ^= zobrist_keys[(piece_type<<6)+fromSquare];
                hash ^= zobrist_keys[(piece_type<<6)+toSquare];
                if (target_piece_type) {
                    original_bitboards[target_piece_type - 1] &= ~(1ULL << toSquare);
                    hash ^= zobrist_keys[(target_piece_type<<6)+toSquare];
                }
            }

            en_passant_square = -1;

            if ((piece & 0b00000111) == KING) {

                if (abs(toSquare - fromSquare) == 2 || abs(toSquare - fromSquare) == 3) {
                    if (side_to_move == WHITE) {
                        if (toSquare == 2) {

                            original_bitboards[3] &= ~(1ULL << 0); 
                            original_bitboards[3] |= (1ULL << 3); 
                        } else {

                            original_bitboards[3] &= ~(1ULL << 7); 
                            original_bitboards[3] |= (1ULL << 5); 
                        }
                        white_castling_right = 0;
                    } else {
                        if (toSquare == 58) {

                            original_bitboards[9] &= ~(1ULL << 56); 
                            original_bitboards[9] |= (1ULL << 59); 
                        } else {
                            original_bitboards[9] &= ~(1ULL << 63); 
                            original_bitboards[9] |= (1ULL << 61); 
                        }
                        black_castling_right = 0;
                    }
                    updateHash();
                } else {
                    if (side_to_move == WHITE) {
                        white_castling_right = 0;
                    } else {
                        black_castling_right = 0;
                    }
                }
            }  else if ((piece & 0b00000111) == PAWN) {
                int pawn_distance = abs(toSquare - fromSquare);
                if (pawn_distance == 16) {
                    if(toSquare > fromSquare) {
                        en_passant_square = toSquare - 8;
                    } else {
                        en_passant_square = toSquare + 8;
                    }
                } else if ((pawn_distance == 7 || pawn_distance == 9) && pieces[0] & (1ULL << toSquare)) {
                    if(side_to_move == WHITE) {
                        original_bitboards[6] &= ~(1ULL << (toSquare - 8 * ((piece_color == WHITE) ? 1 : -1)));
                    } else { 
                        original_bitboards[0] &= ~(1ULL << (toSquare - 8 * ((piece_color == WHITE) ? 1 : -1)));
                    }
                }
            } 

            jump_label:

            updateBoard();

            halfmove_clock++;

            side_to_move = (~side_to_move) & 0b11000;
            fullmove_number = (int)(halfmove_clock / 2);

            hash ^= zobrist_keys[780];

        }

        void makeNullMove() {
            side_to_move = (~side_to_move) & 0b11000;
            hash ^= zobrist_keys[780];
        }

        void makeUCIMove(std::string uciMove) {
            static int square[2];
            const std::string files = "abcdefgh";
            const std::string ranks = "12345678";
            char fileChar = uciMove[0];
            char rankChar = uciMove[1];
            int fileIndex = files.find(fileChar);
            int rankIndex = ranks.find(rankChar);
            square[0] = (rankIndex * 8) + fileIndex;
            fileChar = uciMove[2];
            rankChar = uciMove[3];
            fileIndex = files.find(fileChar);
            rankIndex = ranks.find(rankChar);
            square[1] = (rankIndex * 8) + fileIndex;
            makeMove(square[0], square[1]);
        }

        void parseFEN(const std::string& fen) {
            for (int i = 0; i < 12; i++) {
                original_bitboards[i] = 0;
            }

            std::vector<std::string> fenParts;
            std::istringstream iss(fen);
            std::string part;
            while (iss >> part) {
                fenParts.push_back(part);
            }

            std::string boardSetup = fenParts[0];
            int squarePtr = 63;
            for (char c : boardSetup) {
                if (isdigit(c)) {
                    squarePtr -= (c - '0');
                } else if (c == '/') {
                    continue;  
                } else {

                    int pieceType = -1;
                    int offset = (islower(c)) ? 6 : 0;
                    switch (tolower(c)) {
                        case 'p': pieceType = PAWN; break;
                        case 'n': pieceType = KNIGHT; break;
                        case 'b': pieceType = BISHOP; break;
                        case 'r': pieceType = ROOK; break;
                        case 'q': pieceType = QUEEN; break;
                        case 'k': pieceType = KING; break;
                    }

                    original_bitboards[pieceType+offset-1] |= (1ULL << ((squarePtr & ~0b111) - (squarePtr & 0b111) + 7));                    

                    squarePtr--;
                }
            }

            std::string turn = fenParts[1];
            if (turn == "b") {
                side_to_move = BLACK;
            } else {
                side_to_move = WHITE;
            }

            std::string castlingRights = fenParts[2];
            white_castling_right = 0;
            black_castling_right = 0;
            for (char c : castlingRights) {
                switch (c) {
                    case 'K': white_castling_right |= 0b01; break;
                    case 'Q': white_castling_right |= 0b10; break;
                    case 'k': black_castling_right |= 0b01; break;
                    case 'q': black_castling_right |= 0b10; break;
                }
            }

            std::string enPassantData = fenParts[3];
            if (enPassantData != "-") {
                int file = enPassantData[0] - 'a';
                int rank = enPassantData[1] - '1';
                en_passant_square = rank * 8 + file;
            } else {
                en_passant_square = -1;
            }

            if (fenParts.size() >= 5) {
                halfmove_clock = std::stoi(fenParts[4]);
            } else {
                halfmove_clock = 0;
            }

            if (fenParts.size() >= 6) {
                fullmove_number = std::stoi(fenParts[5]);
            } else {
                fullmove_number = 0;
            }
            updateHash();
            updateBoard();
        }

        bool isCapture(int from, int to) {
            if (pieces[0] & (1ULL << to)) {
                return false;
            } return true;
        }

};

std::string moveNotation(int from, int to, Board board) {
    std::string fromSquareRank(1, "12345678"[from >> 3]);
    std::string fromSquareFile(1, "abcdefgh"[from & 7]);
    std::string toSquareRank(1, "12345678"[to >> 3]);
    std::string toSquareFile(1, "abcdefgh"[to & 7]);

    if ((board.getSquare(from) & 0b111) == PAWN && (to < 8 || to > 55)){

        return fromSquareFile + fromSquareRank + toSquareFile + toSquareRank + "q";
    } else {
        return fromSquareFile + fromSquareRank + toSquareFile + toSquareRank;
    }
}