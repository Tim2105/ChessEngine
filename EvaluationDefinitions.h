#ifndef EVALUATIONDEFINITIONS_H
#define EVALUATIONDEFINITIONS_H

#define MAX_DEPTH 32

#define MIN_SCORE -2147483647
#define MAX_SCORE 2147483647

#define MATE_SCORE 1000032

#include <stdint.h>
#include "Bitboard.h"

/**
 * @brief Hier werden Konstanten für die statische Evaluation definiert,
 * sodass diese leichter angepasst werden können.
 * 
 * Konstanten mit dem Präfix MG sind für das Midgame,
 * während Konstanten mit dem Präfix EG für das Endgame sind.
 */

extern int32_t PIECE_VALUE[];

extern Bitboard neighboringFiles[];

// Bonus für jedes Feld, dass eine Farbe angreift
#define MG_MOBILITY_VALUE 2
#define EG_MOBILITY_VALUE 1

// Bonus für jeden Bauern, der neben mindestens einem anderen Bauern steht
#define MG_PAWN_CONNECTED_BASE_VALUE 4
#define EG_PAWN_CONNECTED_BASE_VALUE 2
// Wird mit der Anzahl der fortgeschrittenen Felder multipliziert
#define MG_PAWN_CONNECTED_RANK_ADVANCED_MULTIPLIER 2
#define EG_PAWN_CONNECTED_RANK_ADVANCED_MULTIPLIER 5

// Bonus für jeden Bauern, der mindestens einen anderen Bauern deckt
#define MG_PAWN_CHAIN_VALUE 10
#define EG_PAWN_CHAIN_VALUE 10

// Bestrafung für zwei oder mehrere Bauern in einer Spalte (doppelte Bauern)
#define MG_PAWN_DOUBLED_VALUE -15
#define EG_PAWN_DOUBLED_VALUE -20

// Bestrafung für einen Bauern, der keine Nachbarn hat(keine Bauern in einer Nachbarspalte)
#define MG_PAWN_ISOLATED_BASE_VALUE -10
#define EG_PAWN_ISOLATED_BASE_VALUE -30
// Wird mit der Entfernung zu den äußeren Spalten multipliziert
#define MG_PAWN_ISOLATED_INNER_FILE_MULTIPLIER -3
#define EG_PAWN_ISOLATED_INNER_FILE_MULTIPLIER -2

// Bonus für jeden Freibauern(passed pawn)
#define MG_PAWN_PASSED_BASE_VALUE 20
#define EG_PAWN_PASSED_BASE_VALUE 30
// Wird mit der Anzahl der fortgeschrittenen Felder multipliziert
#define MG_PAWN_PASSED_RANK_ADVANCED_MULTIPLIER 15
#define EG_PAWN_PASSED_RANK_ADVANCED_MULTIPLIER 25

// Bestrafung für jedes Feld um den König herum, dass von einem Gegner angegriffen wird
#define MG_KING_SAFETY_VALUE -5
#define EG_KING_SAFETY_VALUE -8

/**
 * @brief Bauernschilder und -stürme werden nur für das Midgame bewertet.
 */

// Bonus für jeden Bauern, der den König schützt
#define MG_PAWN_SHIELD_VALUE 15

// Bonus für jeden Bauern, der den König angreift(oder einen Angriff droht)
#define MG_PAWN_STORM_BASE_VALUE 2
// Wird mit der Anzahl der fortgeschrittenen Felder multipliziert
#define MG_PAWN_STORM_DISTANCE_MULTIPLIER 2

/**
 * @brief Bewertung für die Distanz zwischen den Königen.
 * Wird addiert, wenn die eigene Farbe mehr Material besitzt und subtrahiert,
 * wenn die gegnerische Farbe mehr Material besitzt.
 */
#define EG_KING_DISTANCE_VALUE 20


/**
 * @brief Die PSQT-Tabellen aus der Sicht der weißen Figuren für das Midgame.
 */
extern int32_t MG_PSQT[][64];

#define MG_PSQT_MULTIPLIER 1

/**
 * @brief Die PSQT-Tabellen aus der Sicht der weißen Figuren für das Endgame.
 */
extern int32_t EG_PSQT[][64];

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

#define MIN_PHASE -0.25
#define MAX_PHASE 1.25

#endif