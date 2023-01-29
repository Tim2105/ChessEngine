#ifndef EVALUATIONDEFINITIONS_H
#define EVALUATIONDEFINITIONS_H

#define MAX_DEPTH 32

#define MIN_SCORE -2000000000
#define MAX_SCORE 2000000000

#define MATE_SCORE 1000000

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
#define MG_PAWN_CONNECTED_VALUE 10
#define EG_PAWN_CONNECTED_VALUE 8

// Bonus für jeden Bauern, der mindestens einen anderen Bauern deckt
extern Bitboard pawnChainMasks[][64];
#define MG_PAWN_CHAIN_VALUE 25
#define EG_PAWN_CHAIN_VALUE 30

// Bestrafung für zwei oder mehrere Bauern in einer Spalte (doppelte Bauern)
extern Bitboard doubledPawnMasks[][64];
#define MG_PAWN_DOUBLED_VALUE -25
#define EG_PAWN_DOUBLED_VALUE -35

// Bestrafung für einen Bauern, der keine Nachbarn hat(keine Bauern in einer Nachbarspalte)
#define MG_PAWN_ISOLATED_VALUE -20
#define EG_PAWN_ISOLATED_VALUE -40

// Bonus für jeden Freibauern(passed pawn)
extern Bitboard sentryMasks[][64];
#define MG_PAWN_PASSED_BASE_VALUE 20
#define EG_PAWN_PASSED_BASE_VALUE 30
// Wird mit der Anzahl der fortgeschrittenen Felder multipliziert
#define MG_PAWN_PASSED_RANK_ADVANCED_MULTIPLIER 15
#define EG_PAWN_PASSED_RANK_ADVANCED_MULTIPLIER 25

// Bestrafung für jedes Feld um den König herum, dass von einem Gegner angegriffen wird
#define MG_KING_SAFETY_VALUE -20
#define EG_KING_SAFETY_VALUE -24

/**
 * @brief Bauernschilder und -stürme werden nur für das Midgame bewertet.
 */

// Bonus für jeden Bauern, der den König schützt.
#define MG_PAWN_SHIELD_VALUE 15

// Bonus für jeden Bauern, der den König angreift(oder einen Angriff droht)
#define MG_PAWN_STORM_BASE_VALUE 10
// Wird mit der Anzahl der fortgeschrittenen Felder multipliziert
#define MG_PAWN_STORM_DISTANCE_MULTIPLIER 10

// Bonus für jedes Feld in der Mitte, das von einem eigenen Bauern besetzt ist
#define MG_CENTER_CONTROL_VALUE 5

/**
 * @brief Bewertung für die Distanz zwischen den Königen.
 * Wird addiert, wenn die eigene Farbe mehr Material(wie in EG_WINNING_MATERIAL_ADVANTAGE angegeben) besitzt und subtrahiert,
 * wenn die gegnerische Farbe mehr Material besitzt.
 */
#define EG_KING_DISTANCE_VALUE 20
#define EG_WINNING_MATERIAL_ADVANTAGE 200

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
#define PROMOTION_QUEEN_SCORE 800
#define PROMOTION_ROOK_SCORE 400
#define PROMOTION_BISHOP_SCORE 200
#define PROMOTION_KNIGHT_SCORE 200

#endif