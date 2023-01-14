#include "Move.h"

Move::Move() {
    
}

Move::Move(int32_t origin, int32_t destination, int32_t flags) {
    move = (origin << 12) | (destination << 6) | flags;
}

int32_t Move::getOrigin() {
    return (move >> 12) & 0x3F;
}

int32_t Move::getDestination() {
    return (move >> 6) & 0x3F;
}

bool Move::isQuiet() {
    return (move & 0x3F) == MOVE_QUIET;
}

bool Move::isDoublePawn() {
    return (move & 0x3F) == MOVE_DOUBLE_PAWN;
}

bool Move::isCastle() {
    return (move & 0x3F) == MOVE_KINGSIDE_CASTLE || (move & 0x3F) == MOVE_QUEENSIDE_CASTLE;
}

bool Move::isKingsideCastle() {
    return (move & 0x3F) == MOVE_KINGSIDE_CASTLE;
}

bool Move::isQueensideCastle() {
    return (move & 0x3F) == MOVE_QUEENSIDE_CASTLE;
}

bool Move::isCapture() {
    return (move & MOVE_CAPTURE);
}

bool Move::isEnPassant() {
    return (move & 0x3F) == MOVE_EN_PASSANT;
}

bool Move::isPromotion() {
    return (move & MOVE_PROMOTION);
}

bool Move::isPromotionKnight() {
    return (move & 0x3F) == MOVE_PROMOTION_KNIGHT;
}

bool Move::isPromotionBishop() {
    return (move & 0x3F) == MOVE_PROMOTION_BISHOP;
}

bool Move::isPromotionRook() {
    return (move & 0x3F) == MOVE_PROMOTION_ROOK;
}

bool Move::isPromotionQueen() {
    return (move & 0x3F) == MOVE_PROMOTION_QUEEN;
}

