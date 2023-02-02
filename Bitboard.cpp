#include "Bitboard.h"
#include "BoardDefinitions.h"

Bitboard pawnAttacks[2][64] = {
    // White
    {
        0x200,0x500,0xA00,0x1400,0x2800,0x5000,0xA000,0x4000,
        0x20000,0x50000,0xA0000,0x140000,0x280000,0x500000,0xA00000,0x400000,
        0x2000000,0x5000000,0xA000000,0x14000000,0x28000000,0x50000000,0xA0000000,0x40000000,
        0x200000000,0x500000000,0xA00000000,0x1400000000,0x2800000000,0x5000000000,0xA000000000,0x4000000000,
        0x20000000000,0x50000000000,0xA0000000000,0x140000000000,0x280000000000,0x500000000000,0xA00000000000,0x400000000000,
        0x2000000000000,0x5000000000000,0xA000000000000,0x14000000000000,0x28000000000000,0x50000000000000,0xA0000000000000,0x40000000000000,
        0x200000000000000,0x500000000000000,0xA00000000000000,0x1400000000000000,0x2800000000000000,0x5000000000000000,0xA000000000000000,0x4000000000000000,
        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
    },
    // Black
    {
        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
        0x2,0x5,0xA,0x14,0x28,0x50,0xA0,0x40,
        0x200,0x500,0xA00,0x1400,0x2800,0x5000,0xA000,0x4000,
        0x20000,0x50000,0xA0000,0x140000,0x280000,0x500000,0xA00000,0x400000,
        0x2000000,0x5000000,0xA000000,0x14000000,0x28000000,0x50000000,0xA0000000,0x40000000,
        0x200000000,0x500000000,0xA00000000,0x1400000000,0x2800000000,0x5000000000,0xA000000000,0x4000000000,
        0x20000000000,0x50000000000,0xA0000000000,0x140000000000,0x280000000000,0x500000000000,0xA00000000000,0x400000000000,
        0x2000000000000,0x5000000000000,0xA000000000000,0x14000000000000,0x28000000000000,0x50000000000000,0xA0000000000000,0x40000000000000
    }
};

Bitboard knightAttacks[64] = {
    0x20400,0x50800,0xA1100,0x142200,0x284400,0x508800,0xA01000,0x402000,
    0x2040004,0x5080008,0xA110011,0x14220022,0x28440044,0x50880088,0xA0100010,0x40200020,
    0x204000402,0x508000805,0xA1100110A,0x1422002214,0x2844004428,0x5088008850,0xA0100010A0,0x4020002040,
    0x20400040200,0x50800080500,0xA1100110A00,0x142200221400,0x284400442800,0x508800885000,0xA0100010A000,0x402000204000,
    0x2040004020000,0x5080008050000,0xA1100110A0000,0x14220022140000,0x28440044280000,0x50880088500000,0xA0100010A00000,0x40200020400000,
    0x204000402000000,0x508000805000000,0xA1100110A000000,0x1422002214000000,0x2844004428000000,0x5088008850000000,0xA0100010A0000000,0x4020002040000000,
    0x400040200000000,0x800080500000000,0x1100110A00000000,0x2200221400000000,0x4400442800000000,0x8800885000000000,0x100010A000000000,0x2000204000000000,
    0x4020000000000,0x8050000000000,0x110A0000000000,0x22140000000000,0x44280000000000,0x88500000000000,0x10A00000000000,0x20400000000000
};

Bitboard kingAttacks[64] = {
    0x302,0x705,0xE0A,0x1C14,0x3828,0x7050,0xE0A0,0xC040,
    0x30203,0x70507,0xE0A0E,0x1C141C,0x382838,0x705070,0xE0A0E0,0xC040C0,
    0x3020300,0x7050700,0xE0A0E00,0x1C141C00,0x38283800,0x70507000,0xE0A0E000,0xC040C000,
    0x302030000,0x705070000,0xE0A0E0000,0x1C141C0000,0x3828380000,0x7050700000,0xE0A0E00000,0xC040C00000,
    0x30203000000,0x70507000000,0xE0A0E000000,0x1C141C000000,0x382838000000,0x705070000000,0xE0A0E0000000,0xC040C0000000,
    0x3020300000000,0x7050700000000,0xE0A0E00000000,0x1C141C00000000,0x38283800000000,0x70507000000000,0xE0A0E000000000,0xC040C000000000,
    0x302030000000000,0x705070000000000,0xE0A0E0000000000,0x1C141C0000000000,0x3828380000000000,0x7050700000000000,0xE0A0E00000000000,0xC040C00000000000,
    0x203000000000000,0x507000000000000,0xA0E000000000000,0x141C000000000000,0x2838000000000000,0x5070000000000000,0xA0E0000000000000,0x40C0000000000000
};

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
    return knightAttacks[sq];
}

Bitboard pawnAttackBitboard(int32_t sq, int32_t side) {
    return pawnAttacks[side / COLOR_MASK][sq];
}

Bitboard kingAttackBitboard(int32_t sq) {
    return kingAttacks[sq];
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