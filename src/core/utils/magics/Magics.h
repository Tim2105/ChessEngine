#ifndef MAGICS_H
#define MAGICS_H

#include "core/utils/magics/Precomputed.h"

#include <stdint.h>

class Magics {
    private:
        static uint64_t rookAttacks[];
        static uint64_t bishopAttacks[];

        static uint64_t rookAttackPtrs[];
        static uint64_t bishopAttackPtrs[];

    public:
        static void initializeMagics();

        static inline uint64_t lookupRookAttacks(int32_t sq, uint64_t occupied) {
            uint64_t ptr = rookAttackPtrs[sq];
            occupied &= MagicNumbers::rookMasks[sq];
            int32_t shift = MagicNumbers::rookShifts[sq];
            uint64_t magic = MagicNumbers::rookMagics[sq];
            uint64_t index = (occupied * magic) >> shift;
            return rookAttacks[ptr + index];
        }

        static inline uint64_t lookupBishopAttacks(int32_t sq, uint64_t occupied) {
            uint64_t ptr = bishopAttackPtrs[sq];
            occupied &= MagicNumbers::bishopMasks[sq];
            int32_t shift = MagicNumbers::bishopShifts[sq];
            uint64_t magic = MagicNumbers::bishopMagics[sq];
            uint64_t index = (occupied * magic) >> shift;
            return bishopAttacks[ptr + index];
        }

};

#endif