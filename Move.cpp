#include "Move.h"
#include <string>

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