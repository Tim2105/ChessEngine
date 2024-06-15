#include "core/utils/Bitboard.h"

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

Bitboard diagonalAttackUntilBlocked(int32_t sq, Bitboard targets, Bitboard occupied) {
    Bitboard attackBitboard;

    // Schließe die Targets aus dem Belegboard aus
    occupied &= ~targets;

    Bitboard allBishopAttacks = Magics::lookupBishopAttacks(sq, targets);

    // Diagonale nach oben rechts
    Bitboard temp = allBishopAttacks & Bitboard(Magics::bishopAttackTopRightMask(sq));
    if((temp & targets) && !(temp & occupied))
        attackBitboard |= temp;

    // Diagonale nach unten links
    temp = allBishopAttacks & Bitboard(Magics::bishopAttackBottomLeftMask(sq));
    if((temp & targets) && !(temp & occupied))
        attackBitboard |= temp;

    // Diagonale nach unten rechts
    temp = allBishopAttacks & Bitboard(Magics::bishopAttackBottomRightMask(sq));
    if((temp & targets) && !(temp & occupied))
        attackBitboard |= temp;

    // Diagonale nach oben links
    temp = allBishopAttacks & Bitboard(Magics::bishopAttackTopLeftMask(sq));
    if((temp & targets) && !(temp & occupied))
        attackBitboard |= temp;

    return attackBitboard;
}

Bitboard horizontalAttackUntilBlocked(int32_t sq, Bitboard targets, Bitboard occupied) {
    Bitboard attackBitboard;

    // Schließe die Targets aus dem Belegboard aus
    occupied &= ~targets;

    Bitboard allRookAttacks = Magics::lookupRookAttacks(sq, targets);

    // Vertikal nach oben
    Bitboard temp = allRookAttacks & Bitboard(Magics::rookAttackTopMask(sq));
    if((temp & targets) && !(temp & occupied))
        attackBitboard |= temp;

    // Vertikal nach unten
    temp = allRookAttacks & Bitboard(Magics::rookAttackBottomMask(sq));
    if((temp & targets) && !(temp & occupied))
        attackBitboard |= temp;

    // Horizontal nach rechts
    temp = allRookAttacks & Bitboard(Magics::rookAttackRightMask(sq));
    if((temp & targets) && !(temp & occupied))
        attackBitboard |= temp;

    // Horizontal nach links
    temp = allRookAttacks & Bitboard(Magics::rookAttackLeftMask(sq));
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

    if(potentialPinners && potentialPinnedPieces.popcount() == 1) {
        pinnedSquares[numPins] = potentialPinnedPieces.getFSB();
        pinnedDirections[numPins] = NORTH_EAST;
        numPins++;
    }

    potentialAttacks = bishopAttacks & Bitboard(Magics::bishopAttackBottomLeftMask(sq));
    potentialPinnedPieces = potentialAttacks & ownPieces;
    potentialPinners = potentialAttacks & enemyPieces;

    if(potentialPinners && potentialPinnedPieces.popcount() == 1) {
        pinnedSquares[numPins] = potentialPinnedPieces.getFSB();
        pinnedDirections[numPins] = SOUTH_WEST;
        numPins++;
    }

    potentialAttacks = bishopAttacks & Bitboard(Magics::bishopAttackBottomRightMask(sq));
    potentialPinnedPieces = potentialAttacks & ownPieces;
    potentialPinners = potentialAttacks & enemyPieces;

    if(potentialPinners && potentialPinnedPieces.popcount() == 1) {
        pinnedSquares[numPins] = potentialPinnedPieces.getFSB();
        pinnedDirections[numPins] = SOUTH_EAST;
        numPins++;
    }

    potentialAttacks = bishopAttacks & Bitboard(Magics::bishopAttackTopLeftMask(sq));
    potentialPinnedPieces = potentialAttacks & ownPieces;
    potentialPinners = potentialAttacks & enemyPieces;

    if(potentialPinners && potentialPinnedPieces.popcount() == 1) {
        pinnedSquares[numPins] = potentialPinnedPieces.getFSB();
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

    if(potentialPinners && potentialPinnedPieces.popcount() == 1) {
        pinnedSquares[numPins] = potentialPinnedPieces.getFSB();
        pinnedDirections[numPins] = NORTH;
        numPins++;
    }

    potentialAttacks = rookAttacks & Bitboard(Magics::rookAttackBottomMask(sq));
    potentialPinnedPieces = potentialAttacks & ownPieces;
    potentialPinners = potentialAttacks & enemyPieces;

    if(potentialPinners && potentialPinnedPieces.popcount() == 1) {
        pinnedSquares[numPins] = potentialPinnedPieces.getFSB();
        pinnedDirections[numPins] = SOUTH;
        numPins++;
    }

    potentialAttacks = rookAttacks & Bitboard(Magics::rookAttackRightMask(sq));
    potentialPinnedPieces = potentialAttacks & ownPieces;
    potentialPinners = potentialAttacks & enemyPieces;

    if(potentialPinners && potentialPinnedPieces.popcount() == 1) {
        pinnedSquares[numPins] = potentialPinnedPieces.getFSB();
        pinnedDirections[numPins] = EAST;
        numPins++;
    }

    potentialAttacks = rookAttacks & Bitboard(Magics::rookAttackLeftMask(sq));
    potentialPinnedPieces = potentialAttacks & ownPieces;
    potentialPinners = potentialAttacks & enemyPieces;

    if(potentialPinners && potentialPinnedPieces.popcount() == 1) {
        pinnedSquares[numPins] = potentialPinnedPieces.getFSB();
        pinnedDirections[numPins] = WEST;
        numPins++;
    }

    return numPins;
}