#ifndef EVALUATIONDEFINITIONS_H
#define EVALUATIONDEFINITIONS_H

#define MAX_DEPTH 32

#define MIN_SCORE -2147483647
#define MAX_SCORE 2147483647

#define MATE_SCORE 1000032

#include <stdint.h>
#include "Bitboard.h"

extern int32_t PIECE_VALUE[];

extern Bitboard neighboringFiles[];

// Bonus für jedes Feld, dass eine Farbe angreift
#define MOBILITY_VALUE 2

// Bonus für jeden Bauern, der neben mindestens einem anderen Bauern steht
#define PAWN_CONNECTED_BASE_VALUE 4
// Wird mit der Anzahl der fortgeschrittenen Felder multipliziert
#define PAWN_CONNECTED_RANK_ADVANCED_MULTIPLIER 2

// Bonus für jeden Bauern, der mindestens einen anderen Bauern deckt
#define PAWN_CHAIN_VALUE 10

// Bestrafung für zwei oder mehrere Bauern in einer Spalte (doppelte Bauern)
#define PAWN_DOUBLED_VALUE -15

// Bestrafung für einen Bauern, der keine Nachbarn hat(keine Bauern in einer Nachbarspalte)
#define PAWN_ISOLATED_BASE_VALUE -10
// Wird mit der Entfernung zu den äußeren Spalten multipliziert
#define PAWN_ISOLATED_INNER_FILE_MULTIPLIER -3

// Bonus für jeden Freibauern(passed pawn)
#define PAWN_PASSED_BASE_VALUE 20
// Wird mit der Anzahl der fortgeschrittenen Felder multipliziert
#define PAWN_PASSED_RANK_ADVANCED_MULTIPLIER 15

// Bonus für jeden Bauern, der den König schützt
#define PAWN_SHIELD_VALUE 15

// Bonus für jeden Bauern, der den König angreift(oder einen Angriff droht)
#define PAWN_STORM_BASE_VALUE 2
// Wird mit der Anzahl der fortgeschrittenen Felder multipliziert
#define PAWN_STORM_DISTANCE_MULTIPLIER 2

// Bestrafung für jedes Feld um den König herum, dass von einem Gegner angegriffen wird
#define KING_SAFETY_VALUE -5

#endif