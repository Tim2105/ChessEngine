#ifndef SEARCH_DETAILS_H
#define SEARCH_DETAILS_H

#include "core/engine/search/Variation.h"

#include <chrono>
#include <stdint.h>
#include <vector>

struct SearchDetails {
    std::vector<Variation> variations;
    uint32_t nodesSearched;
    std::chrono::milliseconds timeTaken;
    int16_t depth;

    SearchDetails() : nodesSearched(0), timeTaken(0), depth(0) {}

    inline int32_t getNumVariations() const {
        return variations.size();
    }

    inline Variation getPrincipalVariation() const {
        return variations.empty() ? Variation() : variations[0];
    }

    inline Move getBestMove() const {
        return variations.empty() ? Move() : variations[0].moves.empty() ? Move() : variations[0].moves[0];
    }

    inline int16_t getBestMoveScore() const {
        return variations.empty() ? 0 : variations[0].score;
    }

    constexpr double nodesPerSecond() const {
        return nodesSearched / (timeTaken.count() / 1000.0);
    }

    constexpr double kiloNodesPerSecond() const {
        return nodesPerSecond() / 1000.0;
    }

    constexpr double megaNodesPerSecond() const {
        return kiloNodesPerSecond() / 1000.0;
    }
};

#endif