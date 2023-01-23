#include "Move.h"
#include <string>

Move::Move() {
    move = 0;
}

Move::Move(int32_t origin, int32_t destination, int32_t flags) {
    move = (origin << 11) | (destination << 4) | flags;
}

Move::~Move() {
    
}

bool Move::exists() const {
    return move != 0;
}

std::ostream& operator<<(std::ostream &os, const Move &m) {
    os << m.toString();

    return os;
}

std::string Move::toString() const {
    std::string origin = "";
    std::string destination = "";
    std::string promotionType = "";

    origin += 'a' + SQ2F(getOrigin());
    origin += std::to_string(SQ2R(getOrigin()) + 1);

    destination += 'a' + SQ2F(getDestination());
    destination += std::to_string(SQ2R(getDestination()) + 1);

    if (isPromotion()) {
        if (isPromotionKnight()) {
            promotionType = "n";
        } else if (isPromotionBishop()) {
            promotionType = "b";
        } else if (isPromotionRook()) {
            promotionType = "r";
        } else {
            promotionType = "q";
        }
    }

    return origin + destination + promotionType;
}

int32_t Move::getOrigin()  const {
    return (move >> 11) & 0x7F;
}

int32_t Move::getDestination() const {
    return (move >> 4) & 0x7F;
}

bool Move::isQuiet() const {
    return (move & 0xF) == MOVE_QUIET;
}

bool Move::isDoublePawn() const {
    return (move & 0xF) == MOVE_DOUBLE_PAWN;
}

bool Move::isCastle() const {
    return (move & 0xF) == MOVE_KINGSIDE_CASTLE || (move & 0xF) == MOVE_QUEENSIDE_CASTLE;
}

bool Move::isKingsideCastle() const {
    return (move & 0xF) == MOVE_KINGSIDE_CASTLE;
}

bool Move::isQueensideCastle() const {
    return (move & 0xF) == MOVE_QUEENSIDE_CASTLE;
}

bool Move::isCapture() const {
    return (move & MOVE_CAPTURE);
}

bool Move::isEnPassant() const {
    return (move & 0xF) == MOVE_EN_PASSANT;
}

bool Move::isPromotion() const {
    return (move & MOVE_PROMOTION);
}

bool Move::isPromotionKnight() const {
    return (move & 0b1011) == MOVE_PROMOTION_KNIGHT;
}

bool Move::isPromotionBishop() const {
    return (move & 0b1011) == MOVE_PROMOTION_BISHOP;
}

bool Move::isPromotionRook() const {
    return (move & 0b1011) == MOVE_PROMOTION_ROOK;
}

bool Move::isPromotionQueen() const {
    return (move & 0b1011) == MOVE_PROMOTION_QUEEN;
}

bool Move::operator==(const Move& other) const {
    return move == other.move;
}

bool Move::operator!=(const Move& other) const {
    return move != other.move;
}

bool Move::operator<(const Move& other) const {
    return move < other.move;
}

bool Move::operator>(const Move& other) const {
    return move > other.move;
}
