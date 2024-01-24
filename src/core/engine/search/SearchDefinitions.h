#ifndef SEARCH_DEFINITIONS_H
#define SEARCH_DEFINITIONS_H

#include "core/chess/Move.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdint.h>

struct MoveScorePair {
    Move move;
    int16_t score;

    bool operator>(const MoveScorePair& rhs) const {
        return score > rhs.score;
    }

    bool operator<(const MoveScorePair& rhs) const {
        return score < rhs.score;
    }
};

static constexpr int16_t ONE_PLY = 6;
static constexpr int16_t ONE_SIXTH_PLY = 1;
static constexpr int16_t ONE_THIRD_PLY = 2;
static constexpr int16_t ONE_HALF_PLY = 3;
static constexpr int16_t TWO_THIRDS_PLY = 4;
static constexpr int16_t FIVE_SIXTHS_PLY = 5;

static constexpr int16_t MAX_PLY = 128;
static constexpr int32_t NODES_PER_CHECKUP = 1024;

static constexpr int16_t MATE_SCORE = 21000;
static constexpr bool isMateScore(int16_t score) {
    return std::abs(score) >= MATE_SCORE - MAX_PLY - 1;
}

static constexpr int32_t isMateIn(int16_t score) {
    return (MATE_SCORE - std::abs(score) + 1) / 2;
}

static constexpr int16_t DRAW_SCORE = 0;
static constexpr int16_t MIN_SCORE = -30000;
static constexpr int16_t MAX_SCORE = 30000;
static constexpr int16_t NEUTRAL_SCORE = 0;

static constexpr uint8_t PV_NODE = 0;
static constexpr uint8_t CUT_NODE = 1;
static constexpr uint8_t ALL_NODE = 2;

static constexpr int16_t HASH_MOVE_SCORE = MAX_SCORE;
static constexpr int16_t KILLER_MOVE_SCORE = 80;

static constexpr int16_t NULL_MOVE_THREAT_MARGIN = 200;

static constexpr int16_t calculateNullMoveReduction(int16_t depth) {
    if(depth <= 8 * ONE_PLY)
        return 3 * ONE_PLY;
    else
        return 4 * ONE_PLY;
}

#endif