#include "core/chess/BoardDefinitions.h"
#include "core/utils/Bitboard.h"
#include "core/utils/magics/Magics.h"

std::ostream& operator<<(std::ostream& os, const Bitboard& bitboard) {
    for(int r = RANK_8; r >= RANK_1; r--) {
        for(int f = FILE_A; f <= FILE_H; f++) {
            if(bitboard.getBit(r * 8 + f))
                os << "1";
            else
                os << ".";
        }
        os << std::endl;
    }

    return os;
}

Bitboard diagonalAttackBitboard(int32_t sq, const Bitboard occupied) {
    return Magics::lookupBishopAttacks(sq, occupied);
}

Bitboard straightAttackBitboard(int32_t sq, const Bitboard occupied) {
    return Magics::lookupRookAttacks(sq, occupied);
}

Bitboard knightAttackBitboard(int32_t sq) {
    return knightAttacks[sq];
}

Bitboard pawnAttackBitboard(int32_t sq, int32_t side) {
    return pawnAttacks[side / COLOR_MASK][sq];
}

Bitboard kingAttackBitboard(int32_t sq) {
    return kingAttacks[sq];
}

Bitboard diagonalAttackUntilBlocked(int32_t sq, Bitboard targets, Bitboard occupied) {
    Bitboard attackBitboard;

    // Schließe die Targets aus dem Belegboard aus
    occupied &= ~targets;

    // Diagonale nach oben rechts
    Bitboard temp = Magics::lookupBishopAttacksTopRight(sq, targets);
    if((temp & targets) && !(temp & occupied))
        attackBitboard |= temp;

    // Diagonale nach unten links
    temp = Magics::lookupBishopAttacksBottomLeft(sq, targets);
    if((temp & targets) && !(temp & occupied))
        attackBitboard |= temp;

    // Diagonale nach unten rechts
    temp = Magics::lookupBishopAttacksBottomRight(sq, targets);
    if((temp & targets) && !(temp & occupied))
        attackBitboard |= temp;

    // Diagonale nach oben links
    temp = Magics::lookupBishopAttacksTopLeft(sq, targets);
    if((temp & targets) && !(temp & occupied))
        attackBitboard |= temp;

    return attackBitboard;
}

Bitboard horizontalAttackUntilBlocked(int32_t sq, Bitboard targets, Bitboard occupied) {
    Bitboard attackBitboard;

    // Schließe die Targets aus dem Belegboard aus
    occupied &= ~targets;

    // Vertikal nach oben
    Bitboard temp = Magics::lookupRookAttacksTop(sq, targets);
    if((temp & targets) && !(temp & occupied))
        attackBitboard |= temp;

    // Vertikal nach unten
    temp = Magics::lookupRookAttacksBottom(sq, targets);
    if((temp & targets) && !(temp & occupied))
        attackBitboard |= temp;

    // Horizontal nach rechts
    temp = Magics::lookupRookAttacksRight(sq, targets);
    if((temp & targets) && !(temp & occupied))
        attackBitboard |= temp;

    // Horizontal nach links
    temp = Magics::lookupRookAttacksLeft(sq, targets);
    if((temp & targets) && !(temp & occupied))
        attackBitboard |= temp;

    return attackBitboard;
}

int32_t getDiagonallyPinnedToSquare(int32_t sq, Bitboard ownPieces,
                                    Bitboard enemyPieces, Bitboard occupied,
                                    int32_t* pinnedSquares, int32_t* pinnedDirections) {
    int32_t numPins = 0;

    Bitboard bishopAttacks = Magics::lookupBishopAttacks(sq, occupied);

    Bitboard potentialAttacks = bishopAttacks & Bitboard(Magics::bishopAttackTopRightMask(sq));
    Bitboard potentialPinnedPieces = potentialAttacks & ownPieces;
    Bitboard potentialPinners = potentialAttacks & enemyPieces;

    if(potentialPinners && potentialPinnedPieces.getNumberOfSetBits() == 1) {
        pinnedSquares[numPins] = potentialPinnedPieces.getFirstSetBit();
        pinnedDirections[numPins] = NORTH_EAST;
        numPins++;
    }

    potentialAttacks = bishopAttacks & Bitboard(Magics::bishopAttackBottomLeftMask(sq));
    potentialPinnedPieces = potentialAttacks & ownPieces;
    potentialPinners = potentialAttacks & enemyPieces;

    if(potentialPinners && potentialPinnedPieces.getNumberOfSetBits() == 1) {
        pinnedSquares[numPins] = potentialPinnedPieces.getFirstSetBit();
        pinnedDirections[numPins] = SOUTH_WEST;
        numPins++;
    }

    potentialAttacks = bishopAttacks & Bitboard(Magics::bishopAttackBottomRightMask(sq));
    potentialPinnedPieces = potentialAttacks & ownPieces;
    potentialPinners = potentialAttacks & enemyPieces;

    if(potentialPinners && potentialPinnedPieces.getNumberOfSetBits() == 1) {
        pinnedSquares[numPins] = potentialPinnedPieces.getFirstSetBit();
        pinnedDirections[numPins] = SOUTH_EAST;
        numPins++;
    }

    potentialAttacks = bishopAttacks & Bitboard(Magics::bishopAttackTopLeftMask(sq));
    potentialPinnedPieces = potentialAttacks & ownPieces;
    potentialPinners = potentialAttacks & enemyPieces;

    if(potentialPinners && potentialPinnedPieces.getNumberOfSetBits() == 1) {
        pinnedSquares[numPins] = potentialPinnedPieces.getFirstSetBit();
        pinnedDirections[numPins] = NORTH_WEST;
        numPins++;
    }

    return numPins;
}

int32_t getHorizontallyPinnedToSquare(int32_t sq, Bitboard ownPieces,
                                    Bitboard enemyPieces, Bitboard occupied,
                                    int32_t* pinnedSquares, int32_t* pinnedDirections) {
    
    int32_t numPins = 0;

    Bitboard rookAttacks = Magics::lookupRookAttacks(sq, occupied);

    Bitboard potentialAttacks = rookAttacks & Bitboard(Magics::rookAttackTopMask(sq));
    Bitboard potentialPinnedPieces = potentialAttacks & ownPieces;
    Bitboard potentialPinners = potentialAttacks & enemyPieces;

    if(potentialPinners && potentialPinnedPieces.getNumberOfSetBits() == 1) {
        pinnedSquares[numPins] = potentialPinnedPieces.getFirstSetBit();
        pinnedDirections[numPins] = NORTH;
        numPins++;
    }

    potentialAttacks = rookAttacks & Bitboard(Magics::rookAttackBottomMask(sq));
    potentialPinnedPieces = potentialAttacks & ownPieces;
    potentialPinners = potentialAttacks & enemyPieces;

    if(potentialPinners && potentialPinnedPieces.getNumberOfSetBits() == 1) {
        pinnedSquares[numPins] = potentialPinnedPieces.getFirstSetBit();
        pinnedDirections[numPins] = SOUTH;
        numPins++;
    }

    potentialAttacks = rookAttacks & Bitboard(Magics::rookAttackRightMask(sq));
    potentialPinnedPieces = potentialAttacks & ownPieces;
    potentialPinners = potentialAttacks & enemyPieces;

    if(potentialPinners && potentialPinnedPieces.getNumberOfSetBits() == 1) {
        pinnedSquares[numPins] = potentialPinnedPieces.getFirstSetBit();
        pinnedDirections[numPins] = EAST;
        numPins++;
    }

    potentialAttacks = rookAttacks & Bitboard(Magics::rookAttackLeftMask(sq));
    potentialPinnedPieces = potentialAttacks & ownPieces;
    potentialPinners = potentialAttacks & enemyPieces;

    if(potentialPinners && potentialPinnedPieces.getNumberOfSetBits() == 1) {
        pinnedSquares[numPins] = potentialPinnedPieces.getFirstSetBit();
        pinnedDirections[numPins] = WEST;
        numPins++;
    }

    return numPins;
}