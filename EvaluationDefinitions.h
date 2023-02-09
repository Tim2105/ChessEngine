#ifndef EVALUATIONDEFINITIONS_H
#define EVALUATIONDEFINITIONS_H

#define MAX_DEPTH 32

#include <stdint.h>
#include "Bitboard.h"

/**
 * @brief Hier werden Konstanten für die statische Evaluation definiert,
 * sodass diese leichter angepasst werden können.
 * 
 * Konstanten mit dem Präfix MG sind für das Midgame,
 * während Konstanten mit dem Präfix EG für das Endgame sind.
 */

extern int16_t PIECE_VALUE[];

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

// Bestrafung für jedes Feld um den König herum, dass von einem Gegner angegriffen wird.
// Felder auf denen eigene Figuren stehen werden ausgenommen.
#define MG_KING_SAFETY_VALUE -58
#define EG_KING_SAFETY_VALUE -24

// Bonus für jedes Feld, dass von einer Figur angegriffen wird.
// Felder auf denen eigene Figuren stehen werden ausgenommen.
#define MG_PAWN_MOBILITY_VALUE 0
#define EG_PAWN_MOBILITY_VALUE 0
#define MG_KNIGHT_MOBILITY_VALUE 0
#define EG_KNIGHT_MOBILITY_VALUE 0
#define MG_BISHOP_MOBILITY_VALUE 5
#define EG_BISHOP_MOBILITY_VALUE 3
#define MG_ROOK_MOBILITY_VALUE 4
#define EG_ROOK_MOBILITY_VALUE 2
#define MG_QUEEN_MOBILITY_VALUE 1
#define EG_QUEEN_MOBILITY_VALUE 1

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

// Bonus für jedes Feld in der Mitte, das von einem eigenen Bauern besetzt ist
#define MG_CENTER_CONTROL_VALUE 12

/**
 * @brief Bewertung für die Distanz zwischen den Königen.
 * Wird addiert, wenn die eigene Farbe mehr Material(wie in EG_WINNING_MATERIAL_ADVANTAGE angegeben) besitzt und subtrahiert,
 * wenn die gegnerische Farbe mehr Material besitzt.
 */
#define EG_KING_DISTANCE_VALUE 20
#define EG_WINNING_MATERIAL_ADVANTAGE 500

/**
 * @brief Die PSQT-Tabellen aus der Sicht der weißen Figuren für das Midgame.
 */
extern int16_t MG_PSQT[][64];

#define MG_PSQT_MULTIPLIER 1

/**
 * @brief Die PSQT-Tabellen aus der Sicht der weißen Figuren für das Endgame.
 */
extern int16_t EG_PSQT[][64];

#define EG_PSQT_MULTIPLIER 1

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