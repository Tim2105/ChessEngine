#include "TestMagics.h"

#include "core/utils/Bitboard.h"

#include "core/utils/magics/Magics.h"
#include "core/utils/magics/MagicsFinder.h"

#include <iostream>

bool testRookMagic(int32_t sq) {
    uint64_t occupied = MagicNumbers::rookMasks[sq];
    int32_t numOccupancies = 1 << __builtin_popcountll(occupied);
    uint64_t occupancies[numOccupancies];
    MagicsFinder::generateAllOccupancyCombinations(occupied, occupancies);

    for(int32_t i = 0; i < numOccupancies; i++) {
        Bitboard expected = MagicsFinder::rookAttackMask(sq, occupancies[i]);
        Bitboard actual = Magics::lookupRookAttacks(sq, occupancies[i]);

        if(expected != actual) {
            std::cout << "Rook magic test failed for square " << sq << std::endl;
            std::cout << "Occupancy: " << std::endl;
            std::cout << Bitboard(occupancies[i]) << std::endl;
            std::cout << "Expected: " << std::endl;
            std::cout << expected << std::endl;
            std::cout << "Actual: " << std::endl;
            std::cout << actual << std::endl;
            return false;
        }
    }

    return true;
}

bool testBishopMagic(int32_t sq) {
    uint64_t occupied = MagicNumbers::bishopMasks[sq];
    int32_t numOccupancies = 1 << __builtin_popcountll(occupied);
    uint64_t occupancies[numOccupancies];
    MagicsFinder::generateAllOccupancyCombinations(occupied, occupancies);

    for(int32_t i = 0; i < numOccupancies; i++) {
        Bitboard expected = MagicsFinder::bishopAttackMask(sq, occupancies[i]);
        Bitboard actual = Magics::lookupBishopAttacks(sq, occupancies[i]);

        if(expected != actual) {
            std::cout << "Bishop magic test failed for square " << sq << std::endl;
            std::cout << "Occupancy: " << std::endl;
            std::cout << Bitboard(occupancies[i]) << std::endl;
            std::cout << "Expected: " << std::endl;
            std::cout << expected << std::endl;
            std::cout << "Actual: " << std::endl;
            std::cout << actual << std::endl;
            return false;
        }
    }

    return true;
}

void testAllRookMagics() {
    std::cout << "Testing rook magics..." << std::endl;

    for(int32_t sq = 0; sq < 64; sq++) {
        if(!testRookMagic(sq)) {
            std::cout << "Rook magic test failed for square " << sq << std::endl;
            return;
        }
    }

    std::cout << "Rook magic test passed!" << std::endl;
}

void testAllBishopMagics() {
    std::cout << "Testing bishop magics..." << std::endl;

    for(int32_t sq = 0; sq < 64; sq++) {
        if(!testBishopMagic(sq)) {
            std::cout << "Bishop magic test failed for square " << sq << std::endl;
            return;
        }
    }

    std::cout << "Bishop magic test passed!" << std::endl;
}