#ifndef VARIATION_H
#define VARIATION_H

#include <vector>
#include "core/chess/Move.h"
#include <stdint.h>
#include <functional>

struct Variation {
    std::vector<Move> moves;
    int32_t score;
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

struct MoveScorePair {
    Move move;
    int32_t score;
};

template<>
struct std::greater<MoveScorePair> {
    bool operator()(const MoveScorePair& lhs, const MoveScorePair& rhs) {
        return lhs.score > rhs.score;
    }
};

template<>
struct std::less<MoveScorePair> {
    bool operator()(const MoveScorePair& lhs, const MoveScorePair& rhs) {
        return lhs.score < rhs.score;
    }
};

#endif