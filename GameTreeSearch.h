#ifndef GAMETREESEARCH_H
#define GAMETREESEARCH_H

#include "Board.h"
#include "Array.h"
#include "EvaluationDefinitions.h"
#include "BoardEvaluator.h"
#include <stdint.h>

/**
 * @brief Die Klasse implementiert die PV-Suche im Spielbaum.
 * https://www.chessprogramming.org/Principal_Variation_Search
 */
class GameTreeSearch {

    private:
        Board* board;
        BoardEvaluator evaluator;

        /**
         * @brief Die Methode führt die PV-Suche im Spielbaum aus.
         * 
         * @param depth Die verbleibende Suchtiefe.
         * @param alpha Max-Score.
         * @param beta Min-Score.
         * @param pv Die bisherige Hauptvariante.
         * @return int32_t Die Bewertung des Knotens.
         */
        int32_t pvSearch(int depth, int32_t alpha, int32_t beta, Array<Move, 32>& pv);

        /**
         * @brief Die Methode führt die Nullfenster-Suche im Spielbaum aus.
         * 
         * @param depth Die verbleibende Suchtiefe.
         * @param alpha Max-Score.
         * @param beta Min-Score.
         * @param pv Die bisherige Hauptvariante.
         * @return int32_t Die Bewertung des Knotens.
         */
        int32_t nwSearch(int depth, int32_t alpha, int32_t beta);

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
        Array<Move, MAX_DEPTH> search(int32_t depth);
};

#endif