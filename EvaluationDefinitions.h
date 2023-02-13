#ifndef EVALUATIONDEFINITIONS_H
#define EVALUATIONDEFINITIONS_H

#include <stdint.h>
#include "Bitboard.h"

/**
 * @brief Hier werden Konstanten für die statische Evaluation definiert,
 * sodass diese leichter angepasst werden können.
 * 
 * Konstanten mit dem Präfix MG sind für das Midgame,
 * während Konstanten mit dem Präfix EG für das Endgame sind.
 */

extern int16_t MG_PIECE_VALUE[];
extern int16_t EG_PIECE_VALUE[];

// Springer sind weniger Wert, wenn weniger Bauern auf dem Feld sind
#define KNIGHT_CAPTURED_PAWN_VALUE -3

// Bonus für das Läuferpaar
#define BISHOP_PAIR_VALUE 50

extern Bitboard neighboringFiles[];

extern Bitboard centerSquares;

// Bonus für jeden Bauern, der neben mindestens einem anderen Bauern steht
extern Bitboard connectedPawnMasks[];
#define MG_PAWN_CONNECTED_VALUE 3
#define EG_PAWN_CONNECTED_VALUE 1

// Bonus für jeden Bauern, der mindestens einen anderen Bauern deckt
extern Bitboard pawnChainMasks[][64];
#define MG_PAWN_CHAIN_VALUE 16
#define EG_PAWN_CHAIN_VALUE 12

// Bestrafung für zwei oder mehrere Bauern in einer Spalte (doppelte Bauern)
extern Bitboard doubledPawnMasks[][64];
#define MG_PAWN_DOUBLED_VALUE -13
#define EG_PAWN_DOUBLED_VALUE -20

// Bestrafung für einen Bauern, der keine Nachbarn hat(keine Bauern in einer Nachbarspalte)
#define MG_PAWN_ISOLATED_VALUE -14
#define EG_PAWN_ISOLATED_VALUE -8

// Bonus für jeden Freibauern(passed pawn)
extern Bitboard sentryMasks[][64];
#define MG_PAWN_PASSED_BASE_VALUE 10
#define EG_PAWN_PASSED_BASE_VALUE 30
// Wird mit der Anzahl der fortgeschrittenen Felder multipliziert
#define MG_PAWN_PASSED_RANK_ADVANCED_MULTIPLIER 20
#define EG_PAWN_PASSED_RANK_ADVANCED_MULTIPLIER 45

// Bonus für jedes Feld, dass von einer Figur angegriffen wird.
// Felder auf denen eigene Figuren stehen werden ausgenommen.
#define MG_PAWN_MOBILITY_VALUE 0
#define EG_PAWN_MOBILITY_VALUE 0
#define MG_KNIGHT_MOBILITY_VALUE 2
#define EG_KNIGHT_MOBILITY_VALUE 1
#define MG_BISHOP_MOBILITY_VALUE 6
#define EG_BISHOP_MOBILITY_VALUE 4
#define MG_ROOK_MOBILITY_VALUE 5
#define EG_ROOK_MOBILITY_VALUE 3
#define MG_QUEEN_MOBILITY_VALUE 2
#define EG_QUEEN_MOBILITY_VALUE 1

// Bestrafung für Figuren, die En Prise(ungeschützt) sind
#define MG_PIECE_EN_PRISE_VALUE -6
#define EG_PIECE_EN_PRISE_VALUE -3

// Angriffszone des Königs
extern Bitboard kingAttackZoneMask[][64];

#define KING_SAFETY_TABLE_SIZE 100
extern int32_t kingSafetyTable[];

#define MG_KING_SAFETY_KNIGHT_THREAT_VALUE 2
#define MG_KING_SAFETY_BISHOP_THREAT_VALUE 2
#define MG_KING_SAFETY_ROOK_THREAT_VALUE 3
#define MG_KING_SAFETY_QUEEN_THREAT_VALUE 5

/**
 * @brief Bauernschilder und -stürme werden nur für das Midgame bewertet.
 */

// Bonus für jeden Bauern, der den König schützt.
extern Bitboard pawnShieldMask[][64];
#define MG_PAWN_SHIELD_VALUE 21

// Bonus für jeden Bauern, der den König angreift(oder einen Angriff droht)
extern Bitboard pawnStormMask[][64];
#define MG_PAWN_STORM_BASE_VALUE 5
// Wird mit der Anzahl der fortgeschrittenen Felder multipliziert
#define MG_PAWN_STORM_DISTANCE_MULTIPLIER 20

/**
 * @brief Bewertung für die Distanz zwischen den Königen.
 * Wird addiert, wenn die eigene Farbe mehr Material(wie in EG_WINNING_MATERIAL_ADVANTAGE angegeben) besitzt und subtrahiert,
 * wenn die gegnerische Farbe mehr Material besitzt.
 */
#define EG_KING_DISTANCE_VALUE 20
#define EG_WINNING_MATERIAL_ADVANTAGE 500

/**
 * @brief Die PSQT aus der Sicht der weißen Figuren für das Midgame.
 */
extern int16_t MG_PSQT[][64];

#define MG_PSQT_MULTIPLIER 1

/**
 * @brief Die PSQT aus der Sicht der weißen Figuren für das Endgame.
 */
extern int16_t EG_PSQT[][64];

#define EG_PSQT_MULTIPLIER 1

/**
 * @brief Die PSQT für die Zugvorsortierung.
 */
extern int16_t MOVE_ORDERING_PSQT[][64];

/**
 * @brief Konstanten zur Berechnung der Spielphase.
 * Eine Phase von 0 bedeutet, Midgame und eine Phase von 1 bedeutet Endgame.
 * Diese Phase wird benutzt um zwischen Midgame- und Endgameevaluation zu interpolieren.
 */

// Figurengewichte

#define PAWN_WEIGHT 0
#define KNIGHT_WEIGHT 1
#define BISHOP_WEIGHT 1
#define ROOK_WEIGHT 2
#define QUEEN_WEIGHT 4

// Phasengrenzen, können unter 0 oder über 1 sein,
// die berechnete Phase wird aber zwischen 0 und 1 eingeschränkt

#define MIN_PHASE -0.5
#define MAX_PHASE 1.25

/**
 * @brief Konstanten für die Zugvorsortierung
 */

#define HASH_MOVE_SCORE 2000
#define PROMOTION_QUEEN_SCORE 1100
#define PROMOTION_ROOK_SCORE 500
#define PROMOTION_BISHOP_SCORE 300
#define PROMOTION_KNIGHT_SCORE 300

#endif