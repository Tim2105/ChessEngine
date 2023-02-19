#ifndef SINGLE_THREADED_SEARCH_TREE_H
#define SINGLE_THREADED_SEARCH_TREE_H

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
#include "core/engine/Evaluator.h"
#include "core/engine/SearchTree.h"

#define EXACT_NODE 1
#define CUT_NODE 2

#define PV_NODE 0
#define NW_NODE 4

#define IS_REGULAR_NODE(x) (!((x) & 4))
#define IS_NW_NODE(x) ((x) & 4)
#define NODE_TYPE(x) ((x) & 3)

#define MVVLVA 0
#define SEE 1

#define QUIESCENCE_SCORE_CUTOFF (-10)

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

class SingleThreadedSearchTree : public SearchTree {
    private:
        TranspositionTable<2097152, 4> transpositionTable;

        int16_t currentMaxDepth;
        uint16_t currentAge;
        uint32_t nodesSearched;

        std::thread timerThread;
        std::atomic_bool searching;

        Array<Move, 64> pvTable[64];
        Move killerMoves[256][2];
        int32_t relativeHistory[2][64][64];

        HashTable<Move, int32_t, 128, 4> seeCache;
        HashTable<Move, int32_t, 128, 4> moveScoreCache;

        void clearRelativeHistory();

        void clearPvTable();

        void clearKillerMoves();

        void shiftKillerMoves();

        void searchTimer(uint32_t searchTime);

        void sortMovesAtRoot(Array<Move, 256>& moves, int32_t moveEvalFunc);

        void sortMoves(Array<Move, 256>& moves, int16_t ply, int32_t moveEvalFunc);

        void sortAndCutMoves(Array<Move, 256>& moves, int32_t minScore, int32_t moveEvalFunc);

        void sortAndCutMoves(Array<Move, 256>& moves, int16_t ply, int32_t minScore, int32_t moveEvalFunc);

        int16_t rootSearch(int16_t depth, int16_t expectedScore);

        int16_t pvSearchRoot(int16_t depth, int16_t alpha, int16_t beta);

        int16_t pvSearch(int16_t depth, int16_t ply, int16_t alpha, int16_t beta);

        int16_t nwSearch(int16_t depth, int16_t ply, int16_t alpha, int16_t beta, bool nullMoveAllowed = true);

        int16_t quiescence(int16_t alpha, int16_t beta, int32_t captureSquare);

        int16_t determineExtension(Move& m, bool isThreat, bool isCheckEvasion = false);

        int16_t determineReduction(int16_t depth, int32_t moveCount, bool isCheckEvasion = false);

    public:
        SingleThreadedSearchTree() = delete;

        SingleThreadedSearchTree(Evaluator& e, uint32_t numVariations = 1) : SearchTree(e, numVariations) {
            currentMaxDepth = 0;
            currentAge = board->getPly();
            nodesSearched = 0;
            searching = false;
        }

        ~SingleThreadedSearchTree() = default;

        virtual void search(uint32_t searchTime) override;

        virtual void stop() override;

        constexpr int16_t getLastSearchDepth() { return currentMaxDepth / ONE_PLY - 1; }

        constexpr uint32_t getNodesSearched() { return nodesSearched; }

        inline void setBoard(Board& b) {
            board = &b;
            evaluator.setBoard(b);

            transpositionTable.clear();
            clearRelativeHistory();
            clearPvTable();
            clearKillerMoves();
            clearVariations();
        }

    private:

        // Für die Gefahrenerkennung in der Nullzugusche
        static constexpr int16_t THREAT_MARGIN = 200;

        /**
         * @brief Die PSQT für die Zugvorsortierung.
         */
        static constexpr int16_t MOVE_ORDERING_PSQT[7][64] = {
                // Empty
                {},
                // Pawn
                {
                        0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, -10, -10, 0, 0, 0,
                        0, 0, 0, 5, 5, 0, 0, 0,
                        5, 5, 10, 20, 20, 10, 5, 5,
                        10, 10, 10, 20, 20, 10, 10, 10,
                        20, 20, 20, 30, 30, 30, 20, 20,
                        30, 30, 30, 40, 40, 30, 30, 30,
                        90, 90, 90, 90, 90, 90, 90, 90
                },
                // Knight
                {
                        -5, -10, 0, 0, 0, 0, -10, -5,
                        -5, 0, 0, 0, 0, 0, 0, -5,
                        -5, 5, 20, 10, 10, 20, 5, -5,
                        -5, 10, 20, 30, 30, 20, 10, -5,
                        -5, 10, 20, 30, 30, 20, 10, -5,
                        -5, 5, 20, 20, 20, 20, 5, -5,
                        -5, 0, 0, 10, 10, 0, 0, -5,
                        -5, 0, 0, 0, 0, 0, 0, -5
                },
                // Bishop
                {
                        0, 0, -10, 0, 0, -10, 0, 0,
                        0, 30, 0, 0, 0, 0, 30, 0,
                        0, 10, 0, 0, 0, 0, 10, 0,
                        0, 0, 10, 20, 20, 10, 0, 0,
                        0, 0, 10, 20, 20, 10, 0, 0,
                        0, 0, 0, 10, 10, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0
                },
                // Rook
                {
                        0, 0, 0, 20, 20, 0, 0, 0,
                        0, 0, 10, 20, 20, 10, 0, 0,
                        0, 0, 10, 20, 20, 10, 0, 0,
                        0, 0, 10, 20, 20, 10, 0, 0,
                        0, 0, 10, 20, 20, 10, 0, 0,
                        0, 0, 10, 20, 20, 10, 0, 0,
                        50, 50, 50, 50, 50, 50, 50, 50,
                        50, 50, 50, 50, 50, 50, 50, 50
                },
                // Queen
                {
                        -20, -10, -5, 0, 0, -5, -10, -20,
                        -10, 0, 5, 10, 10, 5, 0, -10,
                        -5, 5, 10, 15, 15, 10, 5, -5,
                        0, 10, 15, 20, 20, 15, 10, 0,
                        0, 10, 15, 20, 20, 15, 10, 0,
                        -5, 5, 10, 15, 15, 10, 5, -5,
                        -10, 0, 5, 10, 10, 5, 0, -10,
                        -20, -10, -5, 0, 0, -5, -10, -20
                },
                // King
                {
                        0, 0, 5, 0, -15, 0, 10, 0,
                        0, 5, 5, -5, -5, 0, 5, 0,
                        0, 0, 5, 10, 10, 5, 0, 0,
                        0, 5, 10, 20, 20, 10, 5, 0,
                        0, 5, 10, 20, 20, 10, 5, 0,
                        0, 5, 5, 10, 10, 5, 5, 0,
                        0, 0, 5, 5, 5, 5, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0
                }
        };

        /**
         * @brief Masken unm Sentry-Bauern zu erkennen.
         * Sentry-Bauern sind Bauern, die gegnerische Bauern auf dem Weg zur Aufwertung blockieren oder schlagen können.
         */
        static constexpr Bitboard sentryMasks[2][64] = {
                // White
                {
                        0x303030303030300,0x707070707070700,0xE0E0E0E0E0E0E00,0x1C1C1C1C1C1C1C00,0x3838383838383800,0x7070707070707000,0xE0E0E0E0E0E0E000,0xC0C0C0C0C0C0C000,
                        0x303030303030000,0x707070707070000,0xE0E0E0E0E0E0000,0x1C1C1C1C1C1C0000,0x3838383838380000,0x7070707070700000,0xE0E0E0E0E0E00000,0xC0C0C0C0C0C00000,
                        0x303030303000000,0x707070707000000,0xE0E0E0E0E000000,0x1C1C1C1C1C000000,0x3838383838000000,0x7070707070000000,0xE0E0E0E0E0000000,0xC0C0C0C0C0000000,
                        0x303030300000000,0x707070700000000,0xE0E0E0E00000000,0x1C1C1C1C00000000,0x3838383800000000,0x7070707000000000,0xE0E0E0E000000000,0xC0C0C0C000000000,
                        0x303030000000000,0x707070000000000,0xE0E0E0000000000,0x1C1C1C0000000000,0x3838380000000000,0x7070700000000000,0xE0E0E00000000000,0xC0C0C00000000000,
                        0x303000000000000,0x707000000000000,0xE0E000000000000,0x1C1C000000000000,0x3838000000000000,0x7070000000000000,0xE0E0000000000000,0xC0C0000000000000,
                        0x300000000000000,0x700000000000000,0xE00000000000000,0x1C00000000000000,0x3800000000000000,0x7000000000000000,0xE000000000000000,0xC000000000000000,
                        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                },
                // Black
                {
                        0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                        0x3,0x7,0xE,0x1C,0x38,0x70,0xE0,0xC0,
                        0x303,0x707,0xE0E,0x1C1C,0x3838,0x7070,0xE0E0,0xC0C0,
                        0x30303,0x70707,0xE0E0E,0x1C1C1C,0x383838,0x707070,0xE0E0E0,0xC0C0C0,
                        0x3030303,0x7070707,0xE0E0E0E,0x1C1C1C1C,0x38383838,0x70707070,0xE0E0E0E0,0xC0C0C0C0,
                        0x303030303,0x707070707,0xE0E0E0E0E,0x1C1C1C1C1C,0x3838383838,0x7070707070,0xE0E0E0E0E0,0xC0C0C0C0C0,
                        0x30303030303,0x70707070707,0xE0E0E0E0E0E,0x1C1C1C1C1C1C,0x383838383838,0x707070707070,0xE0E0E0E0E0E0,0xC0C0C0C0C0C0,
                        0x3030303030303,0x7070707070707,0xE0E0E0E0E0E0E,0x1C1C1C1C1C1C1C,0x38383838383838,0x70707070707070,0xE0E0E0E0E0E0E0,0xC0C0C0C0C0C0C0,
                }
        };
};

#endif