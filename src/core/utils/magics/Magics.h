#ifndef MAGICS_H
#define MAGICS_H

#include "core/utils/magics/Precomputed.h"

#include <stdint.h>

class Magics {
    private:
        static uint64_t rookAttacks[64][4096];
        static uint64_t bishopAttacks[64][512];

    public:
        static void initializeMagics();

        static inline uint64_t lookupRookAttacks(int32_t sq, uint64_t occupied) {
            occupied &= MagicNumbers::rookMasks[sq];
            int32_t shift = MagicNumbers::rookShifts[sq];
            uint64_t magic = MagicNumbers::rookMagics[sq];
            uint64_t index = (occupied * magic) >> shift;
            return rookAttacks[sq][index];
        }

        static inline uint64_t lookupBishopAttacks(int32_t sq, uint64_t occupied) {
            occupied &= MagicNumbers::bishopMasks[sq];
            int32_t shift = MagicNumbers::bishopShifts[sq];
            uint64_t magic = MagicNumbers::bishopMagics[sq];
            uint64_t index = (occupied * magic) >> shift;
            return bishopAttacks[sq][index];
        }

};

#endif