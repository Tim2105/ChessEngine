#include "EvaluationDefinitions.h"

int32_t PIECE_VALUE[7] = {
        0, // Empty
        100, // Pawn
        320, // Knight
        330, // Bishop
        500, // Rook
        900, // Queen
        0 // King
};

Bitboard neighboringFiles[8] = {
        0x101010101010101, // A
        0x505050505050505, // B
        0xA0A0A0A0A0A0A0A, // C
        0x1414141414141414, // D
        0x2828282828282828, // E
        0x5050505050505050, // F
        0xA0A0A0A0A0A0A0A0, // G
        0x4040404040404040 // H
};