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

#define MATE_SCORE 21000

#define MVVLVA 0
#define SEE 1

#define QUIESCENCE_SCORE_CUTOFF (MIN_SCORE)

#define ASP_WINDOW 25
#define ASP_STEP_FACTOR 4
#define ASP_MAX_DEPTH 2

#define ONE_PLY 6
#define ONE_SIXTH_PLY 1
#define ONE_THIRD_PLY 2
#define ONE_HALF_PLY 3
#define TWO_THIRDS_PLY 4
#define FIVE_SIXTHS_PLY 5

#define MAX_REDUCTION (4 * ONE_PLY)
#define DEFAULT_REDUCTION (FIVE_SIXTHS_PLY)

struct TranspositionTableEntry {
    int8_t depth;
    int16_t score;
    uint8_t type;
    Move hashMove;
};

struct MoveScorePair {
    Move move;
    int32_t score;
};

template<>
struct std::greater<MoveScorePair> {
    bool operator()(const MoveScorePair& lhs, const MoveScorePair& rhs) {
        return lhs.score > rhs.score;
    }
};

template<>
struct std::less<MoveScorePair> {
    bool operator()(const MoveScorePair& lhs, const MoveScorePair& rhs) {
        return lhs.score < rhs.score;
    }
};

class SearchTree {

    private:
        HeapHashTable<uint64_t, TranspositionTableEntry, 131072, 4> transpositionTable;

        int8_t currentMaxDepth;
        uint32_t nodesSearched;

        Board* board;
        BoardEvaluator evaluator;

        std::atomic_bool searching;

        Array<Move, 32> pvTable[32];
        Move killerMoves[32][2];
        int32_t relativeHistory[2][64][64];

        std::vector<Move> principalVariation;

        void clearRelativeHistory();

        void clearPvTable();

        void searchTimer(uint32_t searchTime);

        void sortMoves(Array<Move, 256>& moves, int16_t ply, int32_t moveEvalFunc);

        void sortAndCutMoves(Array<Move, 256>& moves, int32_t minScore, int32_t moveEvalFunc);

        void sortAndCutMoves(Array<Move, 256>& moves, int16_t ply, int32_t minScore, int32_t moveEvalFunc);

        int16_t rootSearch(int8_t depth, int16_t expectedScore);

        int16_t pvSearchRoot(int8_t depth, int16_t alpha, int16_t beta);

        int16_t pvSearch(int8_t depth, int16_t ply, int16_t alpha, int16_t beta);

        int16_t nwSearch(int8_t depth, int16_t ply, int16_t alpha, int16_t beta);

        int16_t quiescence(int16_t alpha, int16_t beta, int32_t captureSquare);

        int8_t determineExtension(int8_t depth, Move& m, int32_t moveCount, bool isCheckEvasion = false);

        int8_t determineReduction(int8_t depth, Move& m, int32_t moveCount, bool isCheckEvasion = false);

    public:
        SearchTree(Board& b);

        ~SearchTree() = default;

        int16_t search(uint32_t searchTime);

        constexpr int8_t getLastSearchDepth() { return currentMaxDepth; }

        constexpr uint32_t getNodesSearched() { return nodesSearched; }

        inline std::vector<Move> getPrincipalVariation() { return principalVariation; }
};

#endif