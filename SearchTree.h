#ifndef SEARCH_TREE_H
#define SEARCH_TREE_H

#include <stdint.h>
#include "Move.h"
#include "HeapHashTable.h"
#include <thread>
#include <atomic>
#include "Board.h"
#include <vector>
#include "BoardEvaluator.h"

#define EXACT_NODE 1
#define CUT_NODE 2

#define PV_NODE 0
#define NW_NODE 4

#define IS_REGULAR_NODE(x) (!((x) & 4))
#define IS_NW_NODE(x) ((x) & 4)
#define NODE_TYPE(x) ((x) & 3)

#define MIN_SCORE -32000
#define MAX_SCORE 32000

#define MATE_SCORE 20000

#define MVVLVA 0
#define SEE 1

#define SEE_SCORE_CUTOFF -10

#define ASP_WINDOW 25
#define ASP_STEP_FACTOR 4
#define ASP_MAX_DEPTH 2

#define ONE_PLY 4
#define ONE_FOURTH_PLY 1
#define ONE_HALF_PLY 2
#define THREE_FOURTH_PLY 3

#define FULL_MOVE_DEPTH 4

struct TranspositionTableEntry {
    int8_t depth;
    int16_t score;
    uint8_t type;
    Move hashMove;
};

class SearchTree {

    private:
        HeapHashTable<uint64_t, TranspositionTableEntry, 131072, 4> transpositionTable;

        int8_t currentMaxDepth;
        uint32_t nodesSearched;

        Board* board;
        BoardEvaluator evaluator;

        std::atomic_bool searching;

        Move killerMoves[64][2];

        std::vector<Move> principalVariation;

        void searchTimer(uint32_t searchTime);

        void sortMoves(Array<Move, 256>& moves, int8_t depth, int32_t moveEvalFunc);

        void sortAndCutMoves(Array<Move, 256>& moves, int32_t minScore, int32_t moveEvalFunc);

        void sortAndCutMoves(Array<Move, 256>& moves, int8_t depth, int32_t minScore, int32_t moveEvalFunc);

        int16_t rootSearch(int8_t depth, int16_t expectedScore);

        int16_t pvSearch(int8_t depth, int16_t alpha, int16_t beta);

        int16_t nwSearch(int8_t depth, int16_t alpha, int16_t beta);

        int16_t quiescence(int16_t alpha, int16_t beta);

        int8_t determineExtension(int8_t depth, Move& m);

        int8_t determineReduction(int8_t depth, Move& m, int32_t moveNumber);

        std::vector<Move> findPrincipalVariation();

    public:
        SearchTree(Board& b);

        int16_t search(uint32_t searchTime);

        inline int8_t getLastSearchDepth() { return currentMaxDepth; }

        inline uint32_t getNodesSearched() { return nodesSearched; }

        inline std::vector<Move> getPrincipalVariation() { return principalVariation; }
};

#endif