#ifndef SEARCH_TREE_H
#define SEARCH_TREE_H

#include <stdint.h>
#include "core/chess/Move.h"
#include "core/utils/tables/TranspositionTable.h"
#include "core/utils/tables/HashTable.h"
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include "core/chess/Board.h"
#include <vector>
#include "core/engine/BoardEvaluator.h"

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
#define IS_MATE_SCORE(x) ((x) > MATE_SCORE - 1000)

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

struct Variation {
    std::vector<Move> moves;
    int32_t score;
};

template<>
struct std::greater<Variation> {
    bool operator()(const Variation& lhs, const Variation& rhs) {
        return lhs.score > rhs.score;
    }
};

template<>
struct std::less<Variation> {
    bool operator()(const Variation& lhs, const Variation& rhs) {
        return lhs.score < rhs.score;
    }
};

class SearchTree {
    private:
        TranspositionTable<524288, 4> transpositionTable;

        uint32_t numVariations;

        int16_t currentMaxDepth;
        uint16_t currentAge;
        uint32_t nodesSearched;

        Board* board;
        BoardEvaluator evaluator;

        std::atomic_bool searching;

        Array<Move, 64> pvTable[64];
        Move killerMoves[256][2];
        int32_t relativeHistory[2][64][64];

        HashTable<Move, int32_t, 128, 4> seeCache;
        HashTable<Move, int32_t, 128, 4> moveScoreCache;

        std::vector<Variation> variations;
        std::mutex variationMutex;

        void clearRelativeHistory();

        void clearPvTable();

        void clearKillerMoves();

        void shiftKillerMoves();

        void clearVariations();

        void searchTimer(uint32_t searchTime);

        void sortMovesAtRoot(Array<Move, 256>& moves, int32_t moveEvalFunc);

        void sortMoves(Array<Move, 256>& moves, int16_t ply, int32_t moveEvalFunc);

        void sortAndCutMoves(Array<Move, 256>& moves, int32_t minScore, int32_t moveEvalFunc);

        void sortAndCutMoves(Array<Move, 256>& moves, int16_t ply, int32_t minScore, int32_t moveEvalFunc);

        int16_t rootSearch(int16_t depth, int16_t expectedScore);

        int16_t pvSearchRoot(int16_t depth, int16_t alpha, int16_t beta);

        int16_t pvSearch(int16_t depth, int16_t ply, int16_t alpha, int16_t beta);

        int16_t nwSearch(int16_t depth, int16_t ply, int16_t alpha, int16_t beta);

        int16_t quiescence(int16_t alpha, int16_t beta, int32_t captureSquare);

        int16_t determineExtension(Move& m, bool isCheckEvasion = false);

        int16_t determineReduction(int16_t depth, int32_t moveCount, bool isCheckEvasion = false);

    public:
        SearchTree(Board& b);

        ~SearchTree() = default;

        int16_t search(uint32_t searchTime);

        constexpr int16_t getLastSearchDepth() { return currentMaxDepth; }

        constexpr uint32_t getNodesSearched() { return nodesSearched; }

        inline std::vector<Move> getPrincipalVariation() {
            variationMutex.lock();

            if(variations.size() > 0) {
                return variations[0].moves;
            } else {
                return std::vector<Move>();
            }

            variationMutex.unlock();
        }

        inline std::vector<Variation> getVariations() {
            variationMutex.lock();

            std::vector<Variation> variationsCopy = variations;

            variationMutex.unlock();

            return variationsCopy;
        }

        void setBoard(Board& b);
};

#endif