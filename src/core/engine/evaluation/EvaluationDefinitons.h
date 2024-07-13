#ifndef EVALUATION_DEFINITIONS_H
#define EVALUATION_DEFINITIONS_H

#include "core/utils/Bitboard.h"

#include <functional>
#include <stdint.h>

struct Score {
    int mg;
    int eg;

    constexpr Score operator+(const Score& other) const {
        return Score{mg + other.mg, eg + other.eg};
    }

    constexpr Score operator-(const Score& other) const {
        return Score{mg - other.mg, eg - other.eg};
    }

    constexpr Score& operator+=(const Score& other) {
        mg += other.mg;
        eg += other.eg;
        return *this;
    }

    constexpr Score& operator-=(const Score& other) {
        mg -= other.mg;
        eg -= other.eg;
        return *this;
    }

    constexpr Score operator*(int scalar) const {
        return Score{mg * scalar, eg * scalar};
    }

    constexpr Score& operator*=(int scalar) {
        mg *= scalar;
        eg *= scalar;
        return *this;
    }

    constexpr Score operator/(int scalar) const {
        return Score{mg / scalar, eg / scalar};
    }

    constexpr Score& operator/=(int scalar) {
        mg /= scalar;
        eg /= scalar;
        return *this;
    }

    constexpr bool operator==(const Score& other) const {
        return mg == other.mg && eg == other.eg;
    }

    constexpr bool operator!=(const Score& other) const {
        return !(*this == other);
    }

    constexpr Score operator-() const {
        return Score{-mg, -eg};
    }
};

constexpr Score operator*(int scalar, const Score& score) {
    return score * scalar;
}

inline std::ostream& operator<<(std::ostream& os, const Score& score) {
    os << "{" << score.mg << ", " << score.eg << "}";
    return os;
}

#endif // EVALUATION_DEFINITIONS_H