#include "core/utils/magics/Magics.h"
#include "core/utils/magics/MagicsFinder.h"

#include <iostream>

uint64_t Magics::rookAttacks[64][4096];
uint64_t Magics::bishopAttacks[64][512];

void Magics::initializeMagics() {
    // Fülle die Turm-Tabelle
    for(int32_t sq = 0; sq < 64; sq++) {
        uint64_t occupied = MagicNumbers::rookMasks[sq];
        int32_t shift = MagicNumbers::rookShifts[sq];
        uint64_t magic = MagicNumbers::rookMagics[sq];
        int32_t numOccupancies = 1 << __builtin_popcountll(occupied);
        uint64_t occupancies[numOccupancies];
        MagicsFinder::generateAllOccupancyCombinations(occupied, occupancies);

        for(int32_t i = 0; i < numOccupancies; i++) {
            uint64_t index = (occupancies[i] * magic) >> shift;
            rookAttacks[sq][index] = MagicsFinder::rookAttackMask(sq, occupancies[i]);
        }
    }

    std::cout << "Rook magics initialized!" << std::endl;

    // Fülle die Läufer-Tabelle
    for(int32_t sq = 0; sq < 64; sq++) {
        uint64_t occupied = MagicNumbers::bishopMasks[sq];
        int32_t shift = MagicNumbers::bishopShifts[sq];
        uint64_t magic = MagicNumbers::bishopMagics[sq];
        int32_t numOccupancies = 1 << __builtin_popcountll(occupied);
        uint64_t occupancies[numOccupancies];
        MagicsFinder::generateAllOccupancyCombinations(occupied, occupancies);

        for(int32_t i = 0; i < numOccupancies; i++) {
            uint64_t index = (occupancies[i] * magic) >> shift;
            bishopAttacks[sq][index] = MagicsFinder::bishopAttackMask(sq, occupancies[i]);
        }
    }

    std::cout << "Magics initialized!" << std::endl;
}