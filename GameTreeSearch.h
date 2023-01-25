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
#include <cmath>

#define PV_NODE 1
#define ALL_NODE 2
#define CUT_NODE 3
#define NULL_WINDOW_NODE 4

#define TYPEOF_NODE(flags) ((flags) & 3)

#define IS_REGULAR_NODE(flags) (!((flags) & NULL_WINDOW_NODE))
#define IS_NULL_WINDOW_NODE(flags) ((flags) & NULL_WINDOW_NODE)

#define IS_PV_NODE(flags) (((flags) & 3) & PV_NODE)
#define IS_ALL_NODE(flags) (((flags) & 3) & ALL_NODE)
#define IS_CUT_NODE(flags) (((flags) & 3) & CUT_NODE)

#define FULL_MOVE_SEARCH_DEPTH_FUNCTION(depth) ((int32_t)floor(log2((float)((depth) + 1))))

#define LMR_MOVE_COUNT_FUNCTION(depth, moveCount) ((int32_t)round(log((float)((depth) + 1)) * log((float)(moveCount))))

#define NUM_KILLER_MOVES 2

#define ASPIRATION_WINDOW_SIZE 50
#define ASPIRATION_WINDOW_STEP_BASE 2
#define MAX_ASPIRATION_WINDOW_STEPS 2

struct TranspositionTableEntry {
    int32_t score;
    uint8_t depth;
    uint8_t flags;
    Move hashMove;
};

struct QuiescenceTranspositionTableEntry {
    int32_t score;
    uint8_t flags;
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
         * @brief Leert die Hauptvariantentabelle.
         */
        void clearPVTable();

        /**
         * @brief Die Methode initialisiert die PV-Suche im Spielbaum.
         * 
         * @param depth Die Suchtiefe.
         * @return int32_t Die Bewertung des Knotens.
         */
        int32_t pvSearchInit(uint8_t depth, bool enableAspirationWindow = false, int32_t expectedScore = 0);

        /**
         * @brief Die Methode startet die PV-Suche an der Wurzel des Spielbaums.
         * 
         * @param depth Die Suchtiefe.
         * @return int32_t Die Bewertung des Knotens.
         */
        int32_t pvSearchRoot(uint8_t depth, int32_t alpha, int32_t beta);

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
         * @brief Die Methode führt eine Quieszenzsuche im Spielbaum durch.
         * Eine Quieszenzsuche ist eine Suche, die nur Züge bewertet,
         * die eine Figur schlagen oder Schach entkommen.
         */
        int32_t quiescence(int32_t alpha, int32_t beta);

        /**
         * @brief Die Methode sortiert die Züge nach ihrer Bewertung.
         * 
         * @param moves Die Züge.
         * @param plyFromRoot Die Tiefe im Spielbaum(Relevant für Killer- und Historyheuristik).
         */
        void sortMoves(Array<Move, 256>& moves, int32_t plyFromRoot);

        /**
         * @brief Die Methode sortiert die Züge der Quieszenzsuche nach ihrer Bewertung.
         * Züge mit einer SEE-Bewertung < 0 werden verworfen.
         * 
         * @param moves Die Züge.
         */
        void sortMovesQuiescence(Array<Move, 256>& moves);

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