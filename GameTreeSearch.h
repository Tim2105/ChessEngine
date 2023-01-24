#ifndef GAMETREESEARCH_H
#define GAMETREESEARCH_H

#include "Board.h"
#include "Array.h"
#include "EvaluationDefinitions.h"
#include "BoardEvaluator.h"
#include <stdint.h>
#include <functional>
#include "HashTable.h"
#include "HeapHashTable.h"

#define PV_NODE 1
#define ALL_NODE 2
#define CUT_NODE 3
#define NULL_WINDOW_NODE 4
#define QUIESCENCE_NODE 8

#define IS_REGULAR_NODE(flags) (!((flags) & (NULL_WINDOW_NODE | QUIESCENCE_NODE)))
#define IS_NULL_WINDOW_NODE(flags) ((flags) & NULL_WINDOW_NODE)
#define IS_QUIESCENCE_NODE(flags) ((flags) & QUIESCENCE_NODE)

#define NUM_KILLER_MOVES 2

struct TranspositionTableEntry {
    int32_t score;
    uint8_t depth;
    uint8_t flags;
    Move hashMove;
};

/**
 * @brief Die Klasse implementiert die PV-Suche im Spielbaum.
 * https://www.chessprogramming.org/Principal_Variation_Search
 */
class GameTreeSearch {

    private:
        Board* board;
        BoardEvaluator evaluator;

        HeapHashTable<uint64_t, TranspositionTableEntry, 65536, 4> transpositionTable;

        Array<Move, MAX_DEPTH> pvTable[MAX_DEPTH];

        Move killerTable[MAX_DEPTH][NUM_KILLER_MOVES];

        uint16_t historyTable[15][120];

        uint8_t currentDepth = 0;

        /**
         * @brief Leert die Tabelle mit den Killerzügen.
         */
        void clearKillerTable();

        /**
         * @brief Leert die Tabelle für die History-Heuristik.
         */
        void clearHistoryTable();

        /**
         * @brief Die Methode initialisiert die PV-Suche im Spielbaum.
         * 
         * @param depth Die Suchtiefe.
         * @return int32_t Die Bewertung des Knotens.
         */
        int32_t pvSearchInit(uint8_t depth);

        /**
         * @brief Die Methode führt die PV-Suche im Spielbaum aus.
         * 
         * @param depth Die verbleibende Suchtiefe.
         * @param alpha Max-Score.
         * @param beta Min-Score.
         * @return int32_t Die Bewertung des Knotens.
         */
        int32_t pvSearch(uint8_t depth, int32_t alpha, int32_t beta);

        /**
         * @brief Die Methode führt die Nullfenster-Suche im Spielbaum aus.
         * 
         * @param depth Die verbleibende Suchtiefe.
         * @param beta Min-Score.
         * @return int32_t Die Bewertung des Knotens.
         */
        int32_t nwSearch(uint8_t depth, int32_t alpha, int32_t beta);

        /**
         * @brief Die Methode sortiert die Züge nach ihrer Bewertung.
         * 
         * @param moves Die Züge.
         * @param plyFromRoot Die Tiefe im Spielbaum(Relevant für Killer- und Historyheuristik).
         */
        void sortMoves(Array<Move, 256>& moves, int32_t plyFromRoot);

    public:
        /**
         * @brief Konstruktor
         * @param board Referenz auf das Brett
         */
        GameTreeSearch(Board& board);

        /**
         * @brief Destruktor
         */
        ~GameTreeSearch();

        /**
         * @brief Die Methode such mit einer PV-Suche nach der besten Zugfolge.
         * @param depth Die Suchtiefe.
         * @param pv Die Hauptvarianten.
         * @return Die Bewertung der besten Zugfolge.
         */
        int32_t search(uint8_t depth, std::vector<Move>& pv);
};

#endif