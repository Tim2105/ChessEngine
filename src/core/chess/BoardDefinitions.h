#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <stdint.h>

/**
 * @brief Definitionen für das Spielfeld und die Zuggeneration
 */

#define FR2SQ(f,r) ((r) * 8 + f)
#define SQ2F(sq) ((sq) % 8)
#define SQ2R(sq) ((sq) / 8)

#define TYPEOF(p) ((p) & 7)

enum {
    PAWN = 1,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

enum {
    EMPTY,
    WHITE_PAWN,
    WHITE_KNIGHT,
    WHITE_BISHOP,
    WHITE_ROOK,
    WHITE_QUEEN,
    WHITE_KING,
    BLACK_PAWN = 9,
    BLACK_KNIGHT,
    BLACK_BISHOP,
    BLACK_ROOK,
    BLACK_QUEEN,
    BLACK_KING
};

constexpr int32_t KNIGHT_ATTACKS[8] = {
    -17, -15, -10, -6, 6, 10, 15, 17
};

constexpr int32_t DIAGONAL_ATTACKS[4] = {
    -9, -7, 7, 9
};

constexpr int32_t STRAIGHT_ATTACKS[4] = {
    -8, -1, 1, 8
};

enum {
    FILE_A,
    FILE_B,
    FILE_C,
    FILE_D,
    FILE_E,
    FILE_F,
    FILE_G,
    FILE_H,
    FILE_NONE
};

enum {
    RANK_1,
    RANK_2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8,
    RANK_NONE
};

#define WHITE 0
#define BLACK 8
#define COLOR_MASK 8

enum {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8, NO_SQ
};

enum {
    WHITE_KINGSIDE_CASTLE = 1,
    WHITE_QUEENSIDE_CASTLE = 2,
    BLACK_KINGSIDE_CASTLE = 4,
    BLACK_QUEENSIDE_CASTLE = 8
};

#define MOVE_QUIET 0
#define MOVE_DOUBLE_PAWN 1
#define MOVE_KINGSIDE_CASTLE 2
#define MOVE_QUEENSIDE_CASTLE 3
#define MOVE_CAPTURE 4
#define MOVE_EN_PASSANT 5
#define MOVE_PROMOTION 8
#define MOVE_PROMOTION_KNIGHT 8
#define MOVE_PROMOTION_BISHOP 9
#define MOVE_PROMOTION_ROOK 10
#define MOVE_PROMOTION_QUEEN 11

#define QUIET_MOVES MOVE_QUIET, MOVE_DOUBLE_PAWN

#define CAPTURE_MOVES MOVE_CAPTURE, MOVE_EN_PASSANT

#define PROMOTION_MOVES MOVE_PROMOTION_KNIGHT, MOVE_PROMOTION_BISHOP, \
                        MOVE_PROMOTION_ROOK, MOVE_PROMOTION_QUEEN

#define CASTLING_MOVES MOVE_KINGSIDE_CASTLE, MOVE_QUEENSIDE_CASTLE

#define ALL_MOVES MOVE_QUIET, MOVE_DOUBLE_PAWN, \
                  MOVE_KINGSIDE_CASTLE, MOVE_QUEENSIDE_CASTLE, \
                  MOVE_CAPTURE, MOVE_EN_PASSANT, \
                  MOVE_PROMOTION_KNIGHT, MOVE_PROMOTION_BISHOP, \
                  MOVE_PROMOTION_ROOK, MOVE_PROMOTION_QUEEN

#define ALL_PIECES PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING

#define NULL_MOVE 0

#define NORTH 8
#define SOUTH -8
#define EAST 1
#define WEST -1
#define NORTH_EAST 9
#define NORTH_WEST 7
#define SOUTH_EAST -7
#define SOUTH_WEST -9

#endif