#include "core/utils/magics/MagicsFinder.h"
#include "core/utils/magics/Precomputed.h"
#include "core/utils/Bitboard.h"

#include <bit>
#include <iostream>
#include <random>

uint64_t MagicsFinder::rookAttackMask(int sq, uint64_t occupied) {
    uint64_t attackBitboard = 0ULL;

    // Vertikal nach oben
    for(int i = sq + 8; i < 64; i += 8) {
        attackBitboard |= (1ULL << i);

        if(occupied & (1ULL << i))
            break;
    }

    // Vertikal nach unten
    for(int i = sq - 8; i >= 0; i -= 8) {
        attackBitboard |= (1ULL << i);

        if(occupied & (1ULL << i))
            break;
    }

    // Horizontal nach rechts
    for(int i = sq + 1; i < 64 && i % 8 != 0; i += 1) {
        attackBitboard |= (1ULL << i);

        if(occupied & (1ULL << i))
            break;
    }

    // Horizontal nach links
    for(int i = sq - 1; i >= 0 && i % 8 != 7; i -= 1) {
        attackBitboard |= (1ULL << i);

        if(occupied & (1ULL << i))
            break;
    }

    return attackBitboard;
}

uint64_t MagicsFinder::bishopAttackMask(int sq, uint64_t occupied) {
    uint64_t attackBitboard = 0ULL;

    // Diagonal nach oben rechts
    for(int i = sq + 9; i < 64 && i % 8 != 0; i += 9) {
        attackBitboard |= (1ULL << i);

        if(occupied & (1ULL << i))
            break;
    }

    // Diagonal nach unten links
    for(int i = sq - 9; i >= 0 && i % 8 != 7; i -= 9) {
        attackBitboard |= (1ULL << i);

        if(occupied & (1ULL << i))
            break;
    }

    // Diagonal nach oben links
    for(int i = sq + 7; i < 64 && i % 8 != 7; i += 7) {
        attackBitboard |= (1ULL << i);

        if(occupied & (1ULL << i))
            break;
    }

    // Diagonal nach unten rechts
    for(int i = sq - 7; i >= 0 && i % 8 != 0; i -= 7) {
        attackBitboard |= (1ULL << i);

        if(occupied & (1ULL << i))
            break;
    }

    return attackBitboard;
}

uint64_t MagicsFinder::rookAttackTopMask(int sq) {
    uint64_t attackBitboard = 0ULL;

    // Vertikal nach oben
    for(int i = sq + 8; i < 64; i += 8)
        attackBitboard |= (1ULL << i);

    return attackBitboard;
}

uint64_t MagicsFinder::rookAttackRightMask(int sq) {
    uint64_t attackBitboard = 0ULL;

    // Horizontal nach rechts
    for(int i = sq + 1; i < 64 && i % 8 != 0; i += 1)
        attackBitboard |= (1ULL << i);

    return attackBitboard;
}

uint64_t MagicsFinder::rookAttackBottomMask(int sq) {
    uint64_t attackBitboard = 0ULL;

    // Vertikal nach unten
    for(int i = sq - 8; i >= 0; i -= 8)
        attackBitboard |= (1ULL << i);

    return attackBitboard;
}

uint64_t MagicsFinder::rookAttackLeftMask(int sq) {
    uint64_t attackBitboard = 0ULL;

    // Horizontal nach links
    for(int i = sq - 1; i >= 0 && i % 8 != 7; i -= 1)
        attackBitboard |= (1ULL << i);

    return attackBitboard;
}

uint64_t MagicsFinder::bishopAttackTopLeftMask(int sq) {
    uint64_t attackBitboard = 0ULL;

    // Diagonal nach oben links
    for(int i = sq + 7; i < 64 && i % 8 != 7; i += 7)
        attackBitboard |= (1ULL << i);

    return attackBitboard;
}


uint64_t MagicsFinder::bishopAttackTopRightMask(int sq) {
    uint64_t attackBitboard = 0ULL;

    // Diagonal nach oben rechts
    for(int i = sq + 9; i < 64 && i % 8 != 0; i += 9)
        attackBitboard |= (1ULL << i);

    return attackBitboard;
}

uint64_t MagicsFinder::bishopAttackBottomLeftMask(int sq) {
    uint64_t attackBitboard = 0ULL;

    // Diagonal nach unten links
    for(int i = sq - 9; i >= 0 && i % 8 != 7; i -= 9)
        attackBitboard |= (1ULL << i);

    return attackBitboard;
}

uint64_t MagicsFinder::bishopAttackBottomRightMask(int sq) {
    uint64_t attackBitboard = 0ULL;

    // Diagonal nach unten rechts
    for(int i = sq - 7; i >= 0 && i % 8 != 0; i -= 7)
        attackBitboard |= (1ULL << i);

    return attackBitboard;
}

void MagicsFinder::findRookMasks(std::ofstream& resultFile) {
    resultFile << "static constexpr uint64_t rookMasks[64] = {\n";
    for(int sq = 0; sq < 64; sq++) {
        uint64_t mask = 0ULL;

        int rank = sq / 8;
        int file = sq % 8;

        // Vertikal nach oben
        for(int r = rank + 1; r < 7; r++) {
            mask |= (1ULL << (r * 8 + file));
        }

        // Vertikal nach unten
        for(int r = rank - 1; r > 0; r--) {
            mask |= (1ULL << (r * 8 + file));
        }

        // Horizontal nach rechts
        for(int f = file + 1; f < 7; f++) {
            mask |= (1ULL << (rank * 8 + f));
        }

        // Horizontal nach links
        for(int f = file - 1; f > 0; f--) {
            mask |= (1ULL << (rank * 8 + f));
        }

        resultFile << "0x" << std::hex << mask << "ULL,";
    }

    resultFile << "\n};\n";
}

void MagicsFinder::findBishopMasks(std::ofstream& resultFile) {
    resultFile << "static constexpr uint64_t bishopMasks[64] = {\n";
    for(int sq = 0; sq < 64; sq++) {
        uint64_t mask = 0ULL;

        int rank = sq / 8;
        int file = sq % 8;

        // Diagonal nach oben rechts
        for(int r = rank + 1, f = file + 1; r < 7 && f < 7; r++, f++) {
            mask |= (1ULL << (r * 8 + f));
        }

        // Diagonal nach unten links
        for(int r = rank - 1, f = file - 1; r > 0 && f > 0; r--, f--) {
            mask |= (1ULL << (r * 8 + f));
        }

        // Diagonal nach oben links
        for(int r = rank + 1, f = file - 1; r < 7 && f > 0; r++, f--) {
            mask |= (1ULL << (r * 8 + f));
        }

        // Diagonal nach unten rechts
        for(int r = rank - 1, f = file + 1; r > 0 && f < 7; r--, f++) {
            mask |= (1ULL << (r * 8 + f));
        }

        resultFile << "0x" << std::hex << mask << "ULL,";
    }

    resultFile << "\n};\n";
}

void MagicsFinder::generateAllOccupancyCombinations(uint64_t mask, uint64_t* occupancies) {
    int numMaskBits = std::popcount(mask);

    // Generiere alle möglichen Belegungen
    for(int i = 0; i < (1 << numMaskBits); i++) {
        uint64_t occupancy = 0ULL;
        
        for(int j = 0; j < numMaskBits; j++) {
            uint64_t bit = (1ULL << j);

            if(i & bit) {
                uint64_t nthBit = mask;

                for(int k = 0; k < j; k++)
                    // Entferne das LSB
                    nthBit &= (nthBit - 1);

                // Behalte nur das LSB
                nthBit &= -nthBit;

                occupancy |= nthBit;
            }
        }

        occupancies[i] = occupancy;
    }
}

bool validateRookMagicNumber(uint64_t magic, int sq, int shift, uint64_t* occupancies, int numMaskBits) {
    int numIndices = 1 << shift;

    uint64_t* attacks = new uint64_t[numIndices];

    for(int j = 0; j < numIndices; j++)
        attacks[j] = 0;

    for(int j = 0; j < (1 << numMaskBits); j++) {
        uint64_t occupancy = occupancies[j];
        uint64_t magicIndex = (occupancy * magic) >> (64 - shift);

        if(attacks[magicIndex] == 0) {
            attacks[magicIndex] = MagicsFinder::rookAttackMask(sq, occupancy);
        } else if(attacks[magicIndex] != MagicsFinder::rookAttackMask(sq, occupancy)) {
            delete[] attacks;
            return false;
        }
    }

    delete[] attacks;
    return true;
}

bool validateBishopMagicNumber(uint64_t magic, int sq, int shift, uint64_t* occupancies, int numMaskBits) {
    int numIndices = 1 << shift;

    uint64_t* attacks = new uint64_t[numIndices];

    for(int j = 0; j < numIndices; j++)
        attacks[j] = 0;

    for(int j = 0; j < (1 << numMaskBits); j++) {
        uint64_t occupancy = occupancies[j];
        uint64_t magicIndex = (occupancy * magic) >> (64 - shift);

        if(attacks[magicIndex] == 0) {
            attacks[magicIndex] = MagicsFinder::bishopAttackMask(sq, occupancy);
        } else if(attacks[magicIndex] != MagicsFinder::bishopAttackMask(sq, occupancy)) {
            delete[] attacks;
            return false;
        }
    }

    delete[] attacks;
    return true;
}

uint64_t MagicsFinder::findRookMagic(int sq, int shift) {
    uint64_t magic = 0ULL;
    uint64_t mask = MagicNumbers::rookMasks[sq];
    int numMaskBits = std::popcount(mask);
    uint64_t occupancies[4096];

    generateAllOccupancyCombinations(mask, occupancies);

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0ULL, 0xFFFFFFFFFFFFFFFFULL);

    bool valid = false;

    for(uint64_t i = 0; i < maxTries; i++) {
        // Generiere einen zufälligen (niedrigen) Magic-Number-Kandidaten
        magic = dis(gen) & dis(gen) & dis(gen);

        // Überprüfe, ob der Kandidat funktioniert
        if(validateRookMagicNumber(magic, sq, shift, occupancies, numMaskBits)) {
            valid = true;
            break;
        }
    }

    if(!valid)
        throw NoMagicNumberFoundException();

    return magic;
}

void MagicsFinder::searchForRookMagics(std::ofstream& resultFile, std::chrono::seconds time) {
    int shifts[64];
    uint64_t magics[64];

    for(int i = 0; i < 64; i++) {
        shifts[i] = 13;
        magics[i] = 0ULL;
    }

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end = begin + time;
    int sq = 0;

    while(std::chrono::steady_clock::now() < end) {
        int currentBestShift = shifts[sq];

        // Suche nach einer Magic-Number für das Feld
        // mit einer besseren Shift-Anzahl
        int shift = currentBestShift - 1;
        try {
            uint64_t magic = findRookMagic(sq, shift);
            shifts[sq] = shift;
            magics[sq] = magic;
            std::cout << "Found rook magic number with Shift " << shift
                << " for Square " << sq << std::endl;
        } catch(NoMagicNumberFoundException& e) {}

        sq++;
        sq %= 64;
    }

    for(int sq = 0; sq < 64; sq++) {
        if(shifts[sq] >= 13) {
            resultFile << "Magic Numbers are missing" << std::endl;
            resultFile.close();
            return;
        }
    }

    resultFile << "int rookShifts[64] = {\n";
    for(int sq = 0; sq < 64; sq++)
        resultFile << 64 - shifts[sq] << ",";

    resultFile << "\n}\n";

    resultFile << "uint64_t rookMagics[64] = {\n";
    for(int sq = 0; sq < 64; sq++)
        resultFile << "0x" << std::hex << magics[sq] << "ULL,";

    resultFile << "\n}\n";

    resultFile.close();
}

uint64_t MagicsFinder::findBishopMagic(int sq, int shift) {
    uint64_t magic = 0ULL;
    uint64_t mask = MagicNumbers::bishopMasks[sq];
    int numMaskBits = std::popcount(mask);
    uint64_t occupancies[4096];

    generateAllOccupancyCombinations(mask, occupancies);

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0ULL, 0xFFFFFFFFFFFFFFFFULL);

    bool valid = false;

    for(uint64_t i = 0; i < maxTries; i++) {
        // Generiere einen zufälligen (niedrigen) Magic-Number-Kandidaten
        magic = dis(gen) & dis(gen) & dis(gen);

        // Überprüfe, ob der Kandidat funktioniert
        if(validateBishopMagicNumber(magic, sq, shift, occupancies, numMaskBits)) {
            valid = true;
            break;
        }
    }

    if(!valid)
        throw NoMagicNumberFoundException();

    return magic;
}

void MagicsFinder::searchForBishopMagics(std::ofstream& resultFile, std::chrono::seconds time) {
    int shifts[64];
    uint64_t magics[64];

    for(int i = 0; i < 64; i++) {
        shifts[i] = 10;
        magics[i] = 0ULL;
    }

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end = begin + time;
    int sq = 0;

    while(std::chrono::steady_clock::now() < end) {
        int currentBestShift = shifts[sq];

        // Suche nach einer Magic-Number für das Feld
        // mit einer besseren Shift-Anzahl
        int shift = currentBestShift - 1;
        try {
            uint64_t magic = findBishopMagic(sq, shift);
            shifts[sq] = shift;
            magics[sq] = magic;
            std::cout << "Found bishop magic number with Shift " << shift
                << " for Square " << sq << std::endl;
        } catch(NoMagicNumberFoundException& e) {}

        sq++;
        sq %= 64;
    }

    for(int sq = 0; sq < 64; sq++) {
        if(shifts[sq] >= 13) {
            resultFile << "Magic Numbers are missing" << std::endl;
            resultFile.close();
            return;
        }
    }

    resultFile << "int bishopShifts[64] = {\n";
    for(int sq = 0; sq < 64; sq++)
        resultFile << 64 - shifts[sq] << ",";

    resultFile << "\n}\n";

    resultFile << "uint64_t bishopMagics[64] = {\n";
    for(int sq = 0; sq < 64; sq++)
        resultFile << "0x" << std::hex << magics[sq] << "ULL,";

    resultFile << "\n}\n";

    resultFile.close();
}