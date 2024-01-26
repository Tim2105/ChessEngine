#ifndef MAGICS_H
#define MAGICS_H

#include "core/utils/magics/Precomputed.h"

#include <stdint.h>
#include <stdexcept>

class Magics {
    private:
        static uint64_t rookAttacks[];
        static uint64_t bishopAttacks[];

        static uint64_t rookAttackPtrs[];
        static uint64_t bishopAttackPtrs[];

        static uint64_t rookAttacksTopMask[];
        static uint64_t rookAttacksRightMask[];
        static uint64_t rookAttacksBottomMask[];
        static uint64_t rookAttacksLeftMask[];

        static uint64_t bishopAttacksTopLeftMask[];
        static uint64_t bishopAttacksTopRightMask[];
        static uint64_t bishopAttacksBottomLeftMask[];
        static uint64_t bishopAttacksBottomRightMask[];

    public:
        static void initializeMagics();

        static inline uint64_t lookupRookAttacks(int32_t sq, uint64_t occupied) noexcept {
            uint64_t ptr = rookAttackPtrs[sq];
            occupied &= MagicNumbers::rookMasks[sq];
            int32_t shift = MagicNumbers::rookShifts[sq];
            uint64_t magic = MagicNumbers::rookMagics[sq];
            uint64_t index = (occupied * magic) >> shift;
            return rookAttacks[ptr + index];
        }

        static inline uint64_t lookupBishopAttacks(int32_t sq, uint64_t occupied) noexcept {
            uint64_t ptr = bishopAttackPtrs[sq];
            occupied &= MagicNumbers::bishopMasks[sq];
            int32_t shift = MagicNumbers::bishopShifts[sq];
            uint64_t magic = MagicNumbers::bishopMagics[sq];
            uint64_t index = (occupied * magic) >> shift;
            return bishopAttacks[ptr + index];
        }

        static inline uint64_t lookupRookAttacksTop(int32_t sq, uint64_t occupied) {
            return lookupRookAttacks(sq, occupied) & rookAttacksTopMask[sq];
        }

        static inline uint64_t lookupRookAttacksRight(int32_t sq, uint64_t occupied) {
            return lookupRookAttacks(sq, occupied) & rookAttacksRightMask[sq];
        }

        static inline uint64_t lookupRookAttacksBottom(int32_t sq, uint64_t occupied) {
            return lookupRookAttacks(sq, occupied) & rookAttacksBottomMask[sq];
        }

        static inline uint64_t lookupRookAttacksLeft(int32_t sq, uint64_t occupied) {
            return lookupRookAttacks(sq, occupied) & rookAttacksLeftMask[sq];
        }

        static inline uint64_t lookupBishopAttacksTopLeft(int32_t sq, uint64_t occupied) {
            return lookupBishopAttacks(sq, occupied) & bishopAttacksTopLeftMask[sq];
        }

        static inline uint64_t lookupBishopAttacksTopRight(int32_t sq, uint64_t occupied) {
            return lookupBishopAttacks(sq, occupied) & bishopAttacksTopRightMask[sq];
        }

        static inline uint64_t lookupBishopAttacksBottomLeft(int32_t sq, uint64_t occupied) {
            return lookupBishopAttacks(sq, occupied) & bishopAttacksBottomLeftMask[sq];
        }

        static inline uint64_t lookupBishopAttacksBottomRight(int32_t sq, uint64_t occupied) {
            return lookupBishopAttacks(sq, occupied) & bishopAttacksBottomRightMask[sq];
        }

        static inline uint64_t rookAttackTopMask(int32_t sq) {
            return rookAttacksTopMask[sq];
        }

        static inline uint64_t rookAttackRightMask(int32_t sq) {
            return rookAttacksRightMask[sq];
        }

        static inline uint64_t rookAttackBottomMask(int32_t sq) {
            return rookAttacksBottomMask[sq];
        }

        static inline uint64_t rookAttackLeftMask(int32_t sq) {
            return rookAttacksLeftMask[sq];
        }

        static inline uint64_t bishopAttackTopLeftMask(int32_t sq) {
            return bishopAttacksTopLeftMask[sq];
        }

        static inline uint64_t bishopAttackTopRightMask(int32_t sq) {
            return bishopAttacksTopRightMask[sq];
        }

        static inline uint64_t bishopAttackBottomLeftMask(int32_t sq) {
            return bishopAttacksBottomLeftMask[sq];
        }

        static inline uint64_t bishopAttackBottomRightMask(int32_t sq) {
            return bishopAttacksBottomRightMask[sq];
        }
};

#endif