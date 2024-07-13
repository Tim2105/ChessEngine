#ifndef VARIATION_H
#define VARIATION_H

#include "core/chess/Move.h"

#include <stdint.h>
#include <vector>

struct Variation {
    std::vector<Move> moves;
    int score;
    int depth;
    int selectiveDepth;
};

template<>
struct std::greater<Variation> {
    bool operator()(const Variation& lhs, const Variation& rhs) {
        return lhs.score > rhs.score;
    }
};

template<>
struct std::less<Variation> {
    bool operator()(const Variation& lhs, const Variation& rhs) {
        return lhs.score < rhs.score;
    }
};

#endif