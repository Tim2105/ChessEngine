#include "core/utils/magics/Magics.h"
#include "core/utils/magics/MagicsFinder.h"

#include <iostream>

uint64_t Magics::rookAttacks[102400];
uint64_t Magics::bishopAttacks[5248];

uint64_t Magics::rookAttackPtrs[64];
uint64_t Magics::bishopAttackPtrs[64];

uint64_t Magics::rookAttacksTopMask[64];
uint64_t Magics::rookAttacksRightMask[64];
uint64_t Magics::rookAttacksBottomMask[64];
uint64_t Magics::rookAttacksLeftMask[64];

uint64_t Magics::bishopAttacksTopLeftMask[64];
uint64_t Magics::bishopAttacksTopRightMask[64];
uint64_t Magics::bishopAttacksBottomLeftMask[64];
uint64_t Magics::bishopAttacksBottomRightMask[64];

void Magics::initializeMagics() {
    // Fülle die Turm-Tabellen
    uint64_t rookAttackPtr = 0;
    for(int32_t sq = 0; sq < 64; sq++) {
        // Zeigertabelle
        rookAttackPtrs[sq] = rookAttackPtr;

        // Fülle die Zugtabellen
        uint64_t occupied = MagicNumbers::rookMasks[sq];
        int32_t shift = MagicNumbers::rookShifts[sq];
        uint64_t magic = MagicNumbers::rookMagics[sq];
        int32_t numOccupancies = 1 << __builtin_popcountll(occupied);
        uint64_t occupancies[numOccupancies];
        MagicsFinder::generateAllOccupancyCombinations(occupied, occupancies);

        for(int32_t i = 0; i < numOccupancies; i++) {
            uint64_t index = (occupancies[i] * magic) >> shift;
            rookAttacks[rookAttackPtr + index] = MagicsFinder::rookAttackMask(sq, occupancies[i]);
        }

        // Inkrementiere den Zeiger
        rookAttackPtr += numOccupancies;

        // Fülle die Richtungsmasken
        rookAttacksTopMask[sq] = MagicsFinder::rookAttackTopMask(sq);
        rookAttacksRightMask[sq] = MagicsFinder::rookAttackRightMask(sq);
        rookAttacksBottomMask[sq] = MagicsFinder::rookAttackBottomMask(sq);
        rookAttacksLeftMask[sq] = MagicsFinder::rookAttackLeftMask(sq);
    }

    // Fülle die Läufer-Tabellen
    uint64_t bishopAttackPtr = 0;
    for(int32_t sq = 0; sq < 64; sq++) {
        // Zeigertabelle
        bishopAttackPtrs[sq] = bishopAttackPtr;

        // Fülle die Zugtabellen
        uint64_t occupied = MagicNumbers::bishopMasks[sq];
        int32_t shift = MagicNumbers::bishopShifts[sq];
        uint64_t magic = MagicNumbers::bishopMagics[sq];
        int32_t numOccupancies = 1 << __builtin_popcountll(occupied);
        uint64_t occupancies[numOccupancies];
        MagicsFinder::generateAllOccupancyCombinations(occupied, occupancies);

        for(int32_t i = 0; i < numOccupancies; i++) {
            uint64_t index = (occupancies[i] * magic) >> shift;
            bishopAttacks[bishopAttackPtr + index] = MagicsFinder::bishopAttackMask(sq, occupancies[i]);
        }

        // Inkrementiere den Zeiger
        bishopAttackPtr += numOccupancies;

        // Fülle die Richtungsmasken
        bishopAttacksTopLeftMask[sq] = MagicsFinder::bishopAttackTopLeftMask(sq);
        bishopAttacksTopRightMask[sq] = MagicsFinder::bishopAttackTopRightMask(sq);
        bishopAttacksBottomLeftMask[sq] = MagicsFinder::bishopAttackBottomLeftMask(sq);
        bishopAttacksBottomRightMask[sq] = MagicsFinder::bishopAttackBottomRightMask(sq);
    }
}