#include "core/utils/magics/MagicsFinder.h"
#include "core/utils/magics/Precomputed.h"

#include "core/utils/Bitboard.h"

#include <random>

void MagicsFinder::findRookMasks(std::ofstream& resultFile) {
    resultFile << "static constexpr uint64_t rookMasks[64] = {\n";
    for(int32_t sq = 0; sq < 64; sq++) {
        uint64_t mask = 0ULL;

        int32_t rank = sq / 8;
        int32_t file = sq % 8;

        for(int32_t r = rank + 1; r < 7; r++)
            mask |= (1ULL << (r * 8 + file));

        for(int32_t r = rank - 1; r > 0; r--)
            mask |= (1ULL << (r * 8 + file));

        for(int32_t f = file + 1; f < 7; f++)
            mask |= (1ULL << (rank * 8 + f));

        for(int32_t f = file - 1; f > 0; f--)
            mask |= (1ULL << (rank * 8 + f));

        resultFile << "0x" << std::hex << mask << "ULL,";
    }

    resultFile << "\n};\n";
}

void MagicsFinder::findBishopMasks(std::ofstream& resultFile) {
    resultFile << "static constexpr uint64_t bishopMasks[64] = {\n";
    for(int32_t sq = 0; sq < 64; sq++) {
        uint64_t mask = 0ULL;

        int32_t rank = sq / 8;
        int32_t file = sq % 8;

        for(int32_t r = rank + 1, f = file + 1; r < 7 && f < 7; r++, f++)
            mask |= (1ULL << (r * 8 + f));

        for(int32_t r = rank - 1, f = file - 1; r > 0 && f > 0; r--, f--)
            mask |= (1ULL << (r * 8 + f));

        for(int32_t r = rank + 1, f = file - 1; r < 7 && f > 0; r++, f--)
            mask |= (1ULL << (r * 8 + f));

        for(int32_t r = rank - 1, f = file + 1; r > 0 && f < 7; r--, f++)
            mask |= (1ULL << (r * 8 + f));

        resultFile << "0x" << std::hex << mask << "ULL,";
    }

    resultFile << "\n};\n";
}

uint64_t MagicsFinder::findRookMagic(int32_t sq, int32_t shift) {
    uint64_t magic = 0ULL;
    uint64_t mask = MagicNumbers::rookMasks[sq];
    int32_t numMaskBits = __builtin_popcountll(mask);
    uint64_t occupancies[4096];
    uint64_t attacks[4096];

    // Generiere alle möglichen Belegungen
    for(int32_t i = 0; i < (1 << numMaskBits); i++) {
        uint64_t occupancy = 0ULL;
        
        for(int32_t j = 0; j < numMaskBits; j++) {
            uint64_t bit = (1ULL << j);

            if(i & bit) {
                uint64_t nthBit = mask;

                for(int32_t k = 0; k < j; k++)
                    // Entferne das LSB
                    nthBit &= (nthBit - 1);

                // Behalte nur das LSB
                nthBit &= -nthBit;

                occupancy |= nthBit;
            }
        }

        occupancies[i] = occupancy;
    }

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0ULL, 0xFFFFFFFFFFFFFFFFULL);

    bool valid = false;

    for(uint64_t i = 0; i < maxTries; i++) {
        // Generiere einen zufälligen (niedrigen) Magic-Number-Kandidaten
        magic = dis(gen) & dis(gen) & dis(gen);

        // Überprüfe, ob der Kandidat funktioniert
        valid = true;

        for(int32_t j = 0; j < 4096; j++)
            attacks[j] = 0;

        for(int32_t j = 0; j < (1 << numMaskBits); j++) {
            uint64_t occupancy = occupancies[j];
            uint64_t magicIndex = (occupancy * magic) >> (64 - shift);

            if(attacks[magicIndex] == 0) {
                attacks[magicIndex] = straightAttackBitboard(sq, occupancy);
            } else if(attacks[magicIndex] != (uint64_t)straightAttackBitboard(sq, occupancy)) {
                valid = false;
                break;
            }
        }

        if(valid)
            break;
    }

    if(!valid)
        throw NoMagicNumberFoundException();

    return magic;
}

void MagicsFinder::searchForRookMagics(std::ofstream& resultFile, std::chrono::seconds time) {
    int32_t shifts[64];
    uint64_t magics[64];

    for(int32_t i = 0; i < 64; i++) {
        shifts[i] = 13;
        magics[i] = 0ULL;
    }

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end = begin + time;
    (void) end;
    int32_t sq = 0;

    while(std::chrono::steady_clock::now() < end) {
        int32_t currentBestShift = shifts[sq];

        // Suche nach einer Magic-Number für das Feld
        // mit einer besseren Shift-Anzahl
        int32_t shift = currentBestShift - 1;
        try {
            uint64_t magic = findRookMagic(sq, shift);
            shifts[sq] = shift;
            magics[sq] = magic;
            std::cout << "Found magic number with Shift " << shift
                << " for Square " << sq << std::endl;
        } catch(NoMagicNumberFoundException& e) {}

        sq++;
        sq %= 64;
    }

    for(int32_t sq = 0; sq < 64; sq++) {
        if(shifts[sq] >= 13) {
            resultFile << "Magic Numbers are missing" << std::endl;
            resultFile.close();
            return;
        }
    }

    resultFile << "int32_t rookShifts[64] = {\n";
    for(int32_t sq = 0; sq < 64; sq++)
        resultFile << 64 - shifts[sq] << ",";

    resultFile << "\n}\n";

    resultFile << "uint64_t rookMagics[64] = {\n";
    for(int32_t sq = 0; sq < 64; sq++)
        resultFile << "0x" << std::hex << magics[sq] << "ULL,";

    resultFile << "\n}\n";

    resultFile.close();
}