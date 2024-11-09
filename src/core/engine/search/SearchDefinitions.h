#ifndef SEARCH_DEFINITIONS_H
#define SEARCH_DEFINITIONS_H

#include "core/chess/Move.h"

#include <algorithm>
#include <assert.h>
#include <cmath>
#include <cstddef>
#include <stdint.h>

/**
 * @brief Speichert einen Zug und die dazugehörige Bewertung.
 */
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

/**
 * ply => Abstand zur Wurzel des Suchbaums
 */

static constexpr int MAX_PLY = 128;

/**
 * @brief Die Anzahl der Knoten, nach denen die Checkup-Funktion aufgerufen werden soll.
 */
static constexpr size_t NODES_PER_CHECKUP = 1024;

/**
 * Definitionen für Bewertungen
 */

static constexpr int MATE_SCORE = 21000;

/**
 * @brief Überprüft, ob ein Wert eine Mattbewertung ist.
 */
static constexpr bool isMateScore(int score) {
    return std::abs(score) >= MATE_SCORE - MAX_PLY - 1;
}

/**
 * @brief Berechnet bei einer Mattbewertung die verbleibenden Züge.
 */
static constexpr int isMateIn(int score) {
    return (MATE_SCORE - std::abs(score) + 1) / 2;
}

static constexpr int DRAW_SCORE = 0;
static constexpr int MIN_SCORE = -30000;
static constexpr int MAX_SCORE = 30000;
static constexpr int NEUTRAL_SCORE = 0;

/**
 * Bewertungen für die Zugvorsortierung
 */

static constexpr int HASH_MOVE_SCORE = MAX_SCORE - 1;
static constexpr int GOOD_CAPTURE_MOVES_MAX = HASH_MOVE_SCORE - 1;
static constexpr int GOOD_CAPTURE_MOVES_NEUTRAL = GOOD_CAPTURE_MOVES_MAX - 900;
static constexpr int GOOD_CAPTURE_MOVES_MIN = GOOD_CAPTURE_MOVES_NEUTRAL;
static constexpr int KILLER_MOVE_SCORE = GOOD_CAPTURE_MOVES_MIN - 1;
static constexpr int QUIET_MOVES_MIN = MIN_SCORE + 1;
static constexpr int QUIET_MOVES_NEUTRAL = NEUTRAL_SCORE;
static constexpr int QUIET_MOVES_MAX = KILLER_MOVE_SCORE - 1;

static_assert(MIN_SCORE < QUIET_MOVES_MIN);
static_assert(QUIET_MOVES_MIN < QUIET_MOVES_NEUTRAL && QUIET_MOVES_NEUTRAL < QUIET_MOVES_MAX);
static_assert(KILLER_MOVE_SCORE < GOOD_CAPTURE_MOVES_MIN);
static_assert(GOOD_CAPTURE_MOVES_MIN <= GOOD_CAPTURE_MOVES_NEUTRAL && GOOD_CAPTURE_MOVES_NEUTRAL < GOOD_CAPTURE_MOVES_MAX);
static_assert(GOOD_CAPTURE_MOVES_MAX < HASH_MOVE_SCORE && HASH_MOVE_SCORE < MAX_SCORE);
static_assert(QUIET_MOVES_MIN < NEUTRAL_SCORE && NEUTRAL_SCORE < QUIET_MOVES_MAX);
static_assert(QUIET_MOVES_MAX < KILLER_MOVE_SCORE);
static_assert(KILLER_MOVE_SCORE < GOOD_CAPTURE_MOVES_MIN);
static_assert(GOOD_CAPTURE_MOVES_MAX < HASH_MOVE_SCORE);

/**
 * Knotentypen:
 * 
 * PV_NODE: PV-Knoten sind Knoten, dessen Bewertung innerhalb des Intervalls ]alpha, beta[ liegt.
 * CUT_NODE: CUT-Knoten sind Knoten, dessen Bewertung größer oder gleich beta ist.
 * ALL_NODE: ALL-Knoten sind Knoten, dessen Bewertung kleiner oder gleich alpha ist.
 */

static constexpr uint8_t PV_NODE = 0;
static constexpr uint8_t CUT_NODE = 1;
static constexpr uint8_t ALL_NODE = 2;

static constexpr int NULL_MOVE_COOLDOWN = 1;
static constexpr int SINGULAR_EXT_COOLDOWN = 3;

static constexpr int IMPROVING_THRESHOLD = 0;

/**
 * @brief Berechnet, wie stark eine Nullzugsuche reduziert werden soll.
 * 
 * @param depth Die aktuelle Suchtiefe.
 * @param staticEval Die statische Bewertung der Position.
 * @param beta Unser aktueller Beta-Wert.
 * @param isImproving Gibt an, ob die Position sich verbessert hat.
 * @return Die Reduktion in Plies.
 */
static constexpr int calculateNullMoveReduction(int depth, int staticEval, int beta, bool isImproving) {
    return 1 + (std::min((staticEval - beta) / 256, 2) + depth / 6) + !isImproving;
}

/**
 * @brief Berechnet, wie weit eine statische Bewertung unter alpha ligen muss,
 * um Futility-Pruning anzuwenden.
 * 
 * @param depth Die aktuelle Suchtiefe.
 * @param isImproving Gibt an, ob die Position sich verbessert hat.
 * 
 * @return Die Margin für Futility-Pruning.
 */
static constexpr int calculateFutilityMargin(int depth, bool isImproving) {
    return 200 + 200 * (depth - 1) + 200 * !isImproving;
}

/**
 * Konstanten für das Delta-Pruning
 */

static constexpr int DELTA_MARGIN = 1000;

#endif