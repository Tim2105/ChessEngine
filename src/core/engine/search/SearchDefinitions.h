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
 * 
 * Definiere ein Ply als 6, das Rechnen mit anteiligen Plies zu ermöglichen.
 */

static constexpr int16_t ONE_PLY = 6;
static constexpr int16_t ONE_SIXTH_PLY = 1;
static constexpr int16_t ONE_THIRD_PLY = 2;
static constexpr int16_t ONE_HALF_PLY = 3;
static constexpr int16_t TWO_THIRDS_PLY = 4;
static constexpr int16_t FIVE_SIXTHS_PLY = 5;

static constexpr int16_t MAX_PLY = 128;

/**
 * @brief Die Anzahl der Knoten, nach denen die Checkup-Funktion aufgerufen werden soll.
 */
static constexpr int32_t NODES_PER_CHECKUP = 1024;

/**
 * Definitionen für Bewertungen
 */

static constexpr int16_t MATE_SCORE = 21000;

/**
 * @brief Überprüft, ob ein Wert eine Mattbewertung ist.
 */
static constexpr bool isMateScore(int16_t score) {
    return std::abs(score) >= MATE_SCORE - MAX_PLY - 1;
}

/**
 * @brief Berechnet bei einer Mattbewertung die verbleibenden Züge.
 */
static constexpr int32_t isMateIn(int16_t score) {
    return (MATE_SCORE - std::abs(score) + 1) / 2;
}

static constexpr int16_t DRAW_SCORE = 0;
static constexpr int16_t MIN_SCORE = -30000;
static constexpr int16_t MAX_SCORE = 30000;
static constexpr int16_t NEUTRAL_SCORE = 0;

/**
 * Bewertungen für die Zugvorsortierung
 */

static constexpr int32_t HASH_MOVE_SCORE = MAX_SCORE - 1;
static constexpr int32_t GOOD_CAPTURE_MOVES_MAX = HASH_MOVE_SCORE - 1;
static constexpr int32_t GOOD_CAPTURE_MOVES_NEUTRAL = GOOD_CAPTURE_MOVES_MAX - 900;
static constexpr int32_t GOOD_CAPTURE_MOVES_MIN = GOOD_CAPTURE_MOVES_NEUTRAL;
static constexpr int32_t KILLER_MOVE_SCORE = GOOD_CAPTURE_MOVES_MIN - 1;
static constexpr int32_t QUIET_MOVES_MIN = MIN_SCORE + 1;
static constexpr int32_t QUIET_MOVES_NEUTRAL = NEUTRAL_SCORE;
static constexpr int32_t QUIET_MOVES_MAX = KILLER_MOVE_SCORE - 1;

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

static constexpr int8_t NULL_MOVE_COOLDOWN = 2;

/**
 * @brief Berechnet, wie stark eine Nullzugsuche reduziert werden soll.
 * 
 * @param depth Die aktuelle Suchtiefe.
 * @param staticEval Die statische Bewertung der Position.
 * @param beta Unser aktueller Beta-Wert.
 * @return Die Reduktion in Plies.
 */
static constexpr int16_t calculateNullMoveReduction(int16_t depth, int16_t staticEval, int16_t beta) {
    return ONE_PLY + (std::min((staticEval - beta) / 256, 2) + depth / (4 * ONE_PLY)) * ONE_PLY;
}

/**
 * @brief Berechnet, wie weit eine statische Bewertung unter alpha ligen muss,
 * um Futility-Pruning anzuwenden.
 * 
 * @param depth Die aktuelle Suchtiefe.
 * 
 * @return Die Margin für Futility-Pruning.
 */
static constexpr int16_t calculateFutilityMargin(int16_t depth) {
    return 200 + 150 * (depth / ONE_PLY - 1);
}

/**
 * Konstanten für das Delta-Pruning
 */

static constexpr int16_t DELTA_MARGIN = 1000;

#endif