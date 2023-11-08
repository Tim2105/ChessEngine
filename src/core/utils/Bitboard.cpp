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
    int32_t pinnedSquare = -1;

    Bitboard temp;

    // Diagonale nach oben rechts 
    for(int32_t i = sq + 9; i < 64 && i % 8 != 0; i += 9) {
        temp.setBit(i);

        if(ownPieces.getBit(i)) {
            if(pinnedSquare != -1)
                break;

            pinnedSquare = i;
        } else if(occupied.getBit(i)) {
            if(enemyPieces.getBit(i))
                if(pinnedSquare != -1) {
                    pinnedSquares[numPins] = pinnedSquare;
                    pinnedDirections[numPins] = NORTH_EAST;
                    numPins++;
                    break;
                }
                else
                    break;
            else
                break;
        }
    }

    temp = 0ULL;

    // Diagonale nach unten links
    pinnedSquare = -1;
    for(int32_t i = sq - 9; i >= 0 && i % 8 != 7; i -= 9) {
        if(ownPieces.getBit(i)) {
            if(pinnedSquare != -1)
                break;

            pinnedSquare = i;
        } else if(occupied.getBit(i)) {
            if(enemyPieces.getBit(i))
                if(pinnedSquare != -1) {
                    pinnedSquares[numPins] = pinnedSquare;
                    pinnedDirections[numPins] = SOUTH_WEST;
                    numPins++;
                    break;
                }
                else
                    break;
            else
                break;
        }
    }

    temp = 0ULL;

    // Diagonale nach unten rechts
    pinnedSquare = -1;
    for(int32_t i = sq + 7; i < 64 && i % 8 != 7; i += 7) {
        if(ownPieces.getBit(i)) {
            if(pinnedSquare != -1)
                break;

            pinnedSquare = i;
        } else if(occupied.getBit(i)) {
            if(enemyPieces.getBit(i))
                if(pinnedSquare != -1) {
                    pinnedSquares[numPins] = pinnedSquare;
                    pinnedDirections[numPins] = SOUTH_EAST;
                    numPins++;
                    break;
                }
                else
                    break;
            else
                break;
        }
    }

    temp = 0ULL;

    // Diagonale nach oben links
    pinnedSquare = -1;
    for(int32_t i = sq - 7; i >= 0 && i % 8 != 0; i -= 7) {
        if(ownPieces.getBit(i)) {
            if(pinnedSquare != -1)
                break;

            pinnedSquare = i;
        } else if(occupied.getBit(i)) {
            if(enemyPieces.getBit(i))
                if(pinnedSquare != -1) {
                    pinnedSquares[numPins] = pinnedSquare;
                    pinnedDirections[numPins] = NORTH_WEST;
                    numPins++;
                    break;
                }
                else
                    break;
            else
                break;
        }
    }

    return numPins;
}

int32_t getHorizontallyPinnedToSquare(int32_t sq, Bitboard ownPieces,
                                    Bitboard enemyPieces, Bitboard occupied,
                                    int32_t* pinnedSquares, int32_t* pinnedDirections) {
    
    int32_t numPins = 0;
    int32_t pinnedSquare = -1;

    // Vertikal nach oben
    for(int32_t i = sq + 8; i < 64; i += 8) {
        if(ownPieces.getBit(i)) {
            if(pinnedSquare != -1)
                break;

            pinnedSquare = i;
        } else if(occupied.getBit(i)) {
            if(enemyPieces.getBit(i))
                if(pinnedSquare != -1) {
                    pinnedSquares[numPins] = pinnedSquare;
                    pinnedDirections[numPins] = NORTH;
                    numPins++;
                    break;
                }
                else
                    break;
            else
                break;
        }
    }

    // Vertikal nach unten
    pinnedSquare = -1;
    for(int32_t i = sq - 8; i >= 0; i -= 8) {
        if(ownPieces.getBit(i)) {
            if(pinnedSquare != -1)
                break;

            pinnedSquare = i;
        } else if(occupied.getBit(i)) {
            if(enemyPieces.getBit(i))
                if(pinnedSquare != -1) {
                    pinnedSquares[numPins] = pinnedSquare;
                    pinnedDirections[numPins] = SOUTH;
                    numPins++;
                    break;
                }
                else
                    break;
            else
                break;
        }
    }

    // Horizontal nach rechts
    pinnedSquare = -1;
    for(int32_t i = sq + 1; i < 64 && i % 8 != 0; i += 1) {
        if(ownPieces.getBit(i)) {
            if(pinnedSquare != -1)
                break;

            pinnedSquare = i;
        } else if(occupied.getBit(i)) {
            if(enemyPieces.getBit(i))
                if(pinnedSquare != -1) {
                    pinnedSquares[numPins] = pinnedSquare;
                    pinnedDirections[numPins] = EAST;
                    numPins++;
                    break;
                }
                else
                    break;
            else
                break;
        }
    }

    // Horizontal nach links
    pinnedSquare = -1;
    for(int32_t i = sq - 1; i >= 0 && i % 8 != 7; i -= 1) {
        if(ownPieces.getBit(i)) {
            if(pinnedSquare != -1)
                break;

            pinnedSquare = i;
        } else if(occupied.getBit(i)) {
            if(enemyPieces.getBit(i))
                if(pinnedSquare != -1) {
                    pinnedSquares[numPins] = pinnedSquare;
                    pinnedDirections[numPins] = WEST;
                    numPins++;
                    break;
                }
                else
                    break;
            else
                break;
        }
    }

    return numPins;
}