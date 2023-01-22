#ifndef GAMETREESEARCH_H
#define GAMETREESEARCH_H

#include "Board.h"
#include "Array.h"
#include "EvaluationDefinitions.h"
#include "BoardEvaluator.h"
#include <stdint.h>
#include <functional>
#include "HashTable.h"

#define PV_NODE 1
#define ALL_NODE 2
#define CUT_NODE 3

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

        HashTable<uint64_t, TranspositionTableEntry, 2048, 8> transpositionTable;

        /**
         * @brief Die Methode führt die PV-Suche im Spielbaum aus.
         * 
         * @param depth Die verbleibende Suchtiefe.
         * @param alpha Max-Score.
         * @param beta Min-Score.
         * @param pv Die bisherige Hauptvariante.
         * @return int32_t Die Bewertung des Knotens.
         */
        int32_t pvSearch(uint8_t depth, int32_t alpha, int32_t beta, Array<Move, MAX_DEPTH>& pv);

        /**
         * @brief Die Methode führt die Nullfenster-Suche im Spielbaum aus.
         * 
         * @param depth Die verbleibende Suchtiefe.
         * @param alpha Max-Score.
         * @param beta Min-Score.
         * @param pv Die bisherige Hauptvariante.
         * @return int32_t Die Bewertung des Knotens.
         */
        int32_t nwSearch(uint8_t depth, int32_t alpha, int32_t beta);

        /**
         * @brief Die Methode sortiert die Züge nach ihrer Bewertung.
         * 
         * @param moves Die Züge.
         */
        void sortMoves(Array<Move, 256>& moves);

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
         * @return Die beste Zugfolge.
         */
        int32_t search(uint8_t depth, Array<Move, MAX_DEPTH>& pv);
};

#endif