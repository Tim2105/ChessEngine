#include "Move.h"
#include <string>

Move::Move() {
    
}

Move::Move(int32_t origin, int32_t destination, int32_t flags) {
    move = (origin << 11) | (destination << 4) | flags;
}

Move::~Move() {
    
}

std::ostream& operator<<(std::ostream &os, Move &m) {
    os << m.toString();

    return os;
}

std::string Move::toString() {
    std::string origin = "";
    std::string destination = "";
    std::string flags = "";

    origin += 'A' + SQ2F(getOrigin());
    origin += std::to_string(SQ2R(getOrigin()) + 1);

    destination += 'A' + SQ2F(getDestination());
    destination += std::to_string(SQ2R(getDestination()) + 1);

    if(isDoublePawn()) {
        flags += " DOUBLE_PAWN";
    }
    if(isCastle()) {
        if(isKingsideCastle()) {
            flags += " KINGSIDE_CASTLE";
        }
        if(isQueensideCastle()) {
            flags += " QUEENSIDE_CASTLE";
        }
    } 
    if(isCapture()) {
        flags += " CAPTURE";
    }
    if(isEnPassant()) {
        flags += " EN_PASSANT";
    }
    if(isPromotion()) {
        flags += " PROMOTION";
        if(isPromotionKnight()) {
            flags += " KNIGHT";
        }
        if(isPromotionBishop()) {
            flags += " BISHOP";
        }
        if(isPromotionRook()) {
            flags += " ROOK";
        }
        if(isPromotionQueen()) {
            flags += " QUEEN";
        }
    }

    return origin + "->" + destination + flags;
}

int32_t Move::getOrigin() {
    return (move >> 11) & 0x7F;
}

int32_t Move::getDestination() {
    return (move >> 4) & 0x7F;
}

bool Move::isQuiet() {
    return (move & 0xF) == MOVE_QUIET;
}

bool Move::isDoublePawn() {
    return (move & 0xF) == MOVE_DOUBLE_PAWN;
}

bool Move::isCastle() {
    return (move & 0xF) == MOVE_KINGSIDE_CASTLE || (move & 0xF) == MOVE_QUEENSIDE_CASTLE;
}

bool Move::isKingsideCastle() {
    return (move & 0xF) == MOVE_KINGSIDE_CASTLE;
}

bool Move::isQueensideCastle() {
    return (move & 0xF) == MOVE_QUEENSIDE_CASTLE;
}

bool Move::isCapture() {
    return (move & MOVE_CAPTURE);
}

bool Move::isEnPassant() {
    return (move & 0xF) == MOVE_EN_PASSANT;
}

bool Move::isPromotion() {
    return (move & MOVE_PROMOTION);
}

bool Move::isPromotionKnight() {
    return (move & 0b1011) == MOVE_PROMOTION_KNIGHT;
}

bool Move::isPromotionBishop() {
    return (move & 0b1011) == MOVE_PROMOTION_BISHOP;
}

bool Move::isPromotionRook() {
    return (move & 0b1011) == MOVE_PROMOTION_ROOK;
}

bool Move::isPromotionQueen() {
    return (move & 0b1011) == MOVE_PROMOTION_QUEEN;
}

bool Move::operator==(const Move& other) const {
    return move == other.move;
}
