#include "Bitboard.h"
#include "BoardDefinitions.h"

Bitboard::Bitboard() {
    bitboard = 0ULL;
}

Bitboard::Bitboard(uint64_t bitboard) {
    this->bitboard = bitboard;
}

Bitboard::Bitboard(const Bitboard& bitboard) {
    this->bitboard = bitboard.bitboard;
}

std::ostream& operator<<(std::ostream& os, const Bitboard& bitboard) {
    for(int r = RANK_8; r >= RANK_1; r--) {
        for(int f = FILE_A; f <= FILE_H; f++) {
            if(bitboard.getBit(r * 8 + f))
                os << "1";
            else
                os << "0";
        }
        os << std::endl;
    }

    return os;
}

uint64_t Bitboard::getBitboard() const {
    return bitboard;
}

void Bitboard::setBitboard(uint64_t bitboard) {
    this->bitboard = bitboard;
}

Bitboard::operator bool() const {
    return bitboard != 0ULL;
}

Bitboard::operator uint64_t() const {
    return bitboard;
}

Bitboard diagonalAttackBitboard(int32_t sq, const Bitboard occupied) {
    Bitboard attackBitboard;

    // Diagonale nach oben rechts
    for(int32_t i = sq + 9; i < 64 && i % 8 != 0; i += 9) {
        attackBitboard.setBit(i);

        if(occupied.getBit(i))
            break;
    }

    // Diagonale nach unten links
    for(int32_t i = sq - 9; i >= 0 && i % 8 != 7; i -= 9) {
        attackBitboard.setBit(i);

        if(occupied.getBit(i))
            break;
    }

    // Diagonale nach unten rechts
    for(int32_t i = sq + 7; i < 64 && i % 8 != 7; i += 7) {
        attackBitboard.setBit(i);

        if(occupied.getBit(i))
            break;
    }

    // Diagonale nach oben links
    for(int32_t i = sq - 7; i >= 0 && i % 8 != 0; i -= 7) {
        attackBitboard.setBit(i);

        if(occupied.getBit(i))
            break;
    }

    return attackBitboard;
}

Bitboard straightAttackBitboard(int32_t sq, const Bitboard occupied) {
    Bitboard attackBitboard;

    // Vertikal nach oben
    for(int32_t i = sq + 8; i < 64; i += 8) {
        attackBitboard.setBit(i);

        if(occupied.getBit(i))
            break;
    }

    // Vertikal nach unten
    for(int32_t i = sq - 8; i >= 0; i -= 8) {
        attackBitboard.setBit(i);

        if(occupied.getBit(i))
            break;
    }

    // Horizontal nach rechts
    for(int32_t i = sq + 1; i < 64 && i % 8 != 0; i += 1) {
        attackBitboard.setBit(i);

        if(occupied.getBit(i))
            break;
    }

    // Horizontal nach links
    for(int32_t i = sq - 1; i >= 0 && i % 8 != 7; i -= 1) {
        attackBitboard.setBit(i);

        if(occupied.getBit(i))
            break;
    }

    return attackBitboard;
}

Bitboard knightAttackBitboard(int32_t sq) {
    int32_t knightMoves[8] = { -17, -15, -10, -6, 6, 10, 15, 17 };
    Bitboard attackBitboard;

    for(int32_t i = 0; i < 8; i++) {
        int32_t target = sq + knightMoves[i];

        if(target >= 0 && target < 64 && abs((target % 8) - (sq % 8)) <= 2)
            attackBitboard.setBit(target);
    }
    return attackBitboard;
}

Bitboard pawnAttackBitboard(int32_t sq, int32_t side) {
    int32_t sideAsSign = side == WHITE ? 1 : -1;

    int32_t pawnMoves[2] = { 7, 9 };
    Bitboard attackBitboard;

    for(int32_t i = 0; i < 2; i++) {
        int32_t target = sq + pawnMoves[i] * sideAsSign;

        if(target >= 0 && target < 64 && abs((target % 8) - (sq % 8)) == 1)
            attackBitboard.setBit(target);
    }
    return attackBitboard;
}

Bitboard kingAttackBitboard(int32_t sq) {
    int32_t kingMoves[8] = { -9, -8, -7, -1, 1, 7, 8, 9 };
    Bitboard attackBitboard;

    for(int32_t i = 0; i < 8; i++) {
        int32_t target = sq + kingMoves[i];

        if(target >= 0 && target < 64 && abs((target % 8) - (sq % 8)) <= 1)
            attackBitboard.setBit(target);
    }
    return attackBitboard;
}

Bitboard diagonalAttackUntilBlocked(int32_t sq, const Bitboard targets, const Bitboard occupied) {
    Bitboard attackBitboard;

    // Diagonale nach oben rechts
    Bitboard temp;
    for(int32_t i = sq + 9; i < 64 && i % 8 != 0; i += 9) {
        temp.setBit(i);

        if(targets.getBit(i)) {
            attackBitboard |= temp;
            break;
        }

        if(occupied.getBit(i))
            break;
    }

    // Diagonale nach unten links
    temp = Bitboard();
    for(int32_t i = sq - 9; i >= 0 && i % 8 != 7; i -= 9) {
        temp.setBit(i);

        if(targets.getBit(i)) {
            attackBitboard |= temp;
            break;
        }

        if(occupied.getBit(i))
            break;
    }

    // Diagonale nach unten rechts
    temp = Bitboard();
    for(int32_t i = sq + 7; i < 64 && i % 8 != 7; i += 7) {
        temp.setBit(i);

        if(targets.getBit(i)) {
            attackBitboard |= temp;
            break;
        }

        if(occupied.getBit(i))
            break;
    }

    // Diagonale nach oben links
    temp = Bitboard();
    for(int32_t i = sq - 7; i >= 0 && i % 8 != 0; i -= 7) {
        temp.setBit(i);

        if(targets.getBit(i)) {
            attackBitboard |= temp;
            break;
        }

        if(occupied.getBit(i))
            break;
    }

    return attackBitboard;
}

Bitboard straightAttackUntilBlocked(int32_t sq, const Bitboard targets, const Bitboard occupied) {
    Bitboard attackBitboard;

    // Vertikal nach oben
    Bitboard temp;
    for(int32_t i = sq + 8; i < 64; i += 8) {
        temp.setBit(i);

        if(targets.getBit(i)) {
            attackBitboard |= temp;
            break;
        }

        if(occupied.getBit(i))
            break;
    }

    // Vertikal nach unten
    temp = Bitboard();
    for(int32_t i = sq - 8; i >= 0; i -= 8) {
        temp.setBit(i);

        if(targets.getBit(i)) {
            attackBitboard |= temp;
            break;
        }

        if(occupied.getBit(i))
            break;
    }

    // Horizontal nach rechts
    temp = Bitboard();
    for(int32_t i = sq + 1; i < 64 && i % 8 != 0; i += 1) {
        temp.setBit(i);

        if(targets.getBit(i)) {
            attackBitboard |= temp;
            break;
        }

        if(occupied.getBit(i))
            break;
    }

    // Horizontal nach links
    temp = Bitboard();
    for(int32_t i = sq - 1; i >= 0 && i % 8 != 7; i -= 1) {
        temp.setBit(i);

        if(targets.getBit(i)) {
            attackBitboard |= temp;
            break;
        }

        if(occupied.getBit(i))
            break;
    }

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

int32_t getStraightPinnedToSquare(int32_t sq, Bitboard ownPieces,
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