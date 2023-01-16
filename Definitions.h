#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define DEBUG

#ifndef DEBUG
#define ASSERT(x)
#else
#define ASSERT(x) \
if(!(x)) { \
printf("ASSERTION FAILED: In %s on line %d\n", \
        __FILE__, __LINE__); \
}
#endif

#define FR2SQ(f,r) (( 21 + (f) ) + ( (r) * 10 ))
#define SQ2F(sq) (( (sq) % 10 ) - 1 )
#define SQ2R(sq) (( (sq) / 10 ) - 2 )

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

#include <stdint.h>

extern int32_t KNIGHT_ATTACKS[];

extern int32_t DIAGONAL_ATTACKS[];

extern int32_t STRAIGHT_ATTACKS[];

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
    A1 = 21, B1, C1, D1, E1, F1, G1, H1,
    A2 = 31, B2, C2, D2, E2, F2, G2, H2,
    A3 = 41, B3, C3, D3, E3, F3, G3, H3,
    A4 = 51, B4, C4, D4, E4, F4, G4, H4,
    A5 = 61, B5, C5, D5, E5, F5, G5, H5,
    A6 = 71, B6, C6, D6, E6, F6, G6, H6,
    A7 = 81, B7, C7, D7, E7, F7, G7, H7,
    A8 = 91, B8, C8, D8, E8, F8, G8, H8, NO_SQ
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

#define NORTH 10
#define SOUTH -10
#define EAST 1
#define WEST -1
#define NORTH_EAST 11
#define NORTH_WEST 9
#define SOUTH_EAST -9
#define SOUTH_WEST -11

#endif