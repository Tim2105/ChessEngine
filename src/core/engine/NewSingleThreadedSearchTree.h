#ifndef NEW_SINGLE_THREADED_SEARCH_TREE_H
#define NEW_SINGLE_THREADED_SEARCH_TREE_H

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

#define ASP_WINDOW 25
#define ASP_STEP_FACTOR 4
#define ASP_MAX_DEPTH 2

#define ONE_PLY 6
#define ONE_SIXTH_PLY 1
#define ONE_THIRD_PLY 2
#define ONE_HALF_PLY 3
#define TWO_THIRDS_PLY 4
#define FIVE_SIXTHS_PLY 5

#define MAX_PLY 256

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

class NewSingleThreadedSearchTree : public SearchTree {
    private:
        TranspositionTable<2097152, 4> transpositionTable;
        Board searchBoard;

        int16_t currentMaxDepth;
        uint16_t currentAge;
        uint32_t nodesSearched;

        std::thread timerThread;
        std::thread searchThread;
        std::atomic_bool searching;

        Array<Move, 64> pvTable[64];
        Move killerMoves[MAX_PLY][2];
        int32_t relativeHistory[2][64][64];

        HashTable<Move, int32_t, 128, 4> seeCache;
        HashTable<Move, int32_t, 128, 4> moveScoreCache;

        int16_t mateDistance;

        void clearRelativeHistory();

        void clearPvTable();

        void clearKillerMoves();

        void shiftKillerMoves();

        void searchTimer(uint32_t searchTime);

        void runSearch();

        inline int32_t scoreMove(Move& move, int16_t ply);

        void sortMovesAtRoot(Array<Move, 256>& moves);

        void sortMoves(Array<Move, 256>& moves, int16_t ply);

        void sortAndCutMovesForQuiescence(Array<Move, 256>& moves, int32_t minScore, int32_t moveEvalFunc);

        void sortAndCutMoves(Array<Move, 256>& moves, int16_t ply, int32_t minScore);

        int16_t rootSearch(int16_t depth, int16_t expectedScore);

        int16_t pvSearchRoot(int16_t depth, int16_t alpha, int16_t beta);

        int16_t pvSearch(int16_t depth, int16_t ply, int16_t alpha, int16_t beta, bool nullMoveAllowed = true);

        int16_t nwSearch(int16_t depth, int16_t ply, int16_t alpha, int16_t beta, bool nullMoveAllowed = true);

        int16_t quiescence(int16_t alpha, int16_t beta);

        inline int16_t determineExtension(bool isThreat, bool isMateThreat, bool isCheckEvasion = false);

        inline int16_t determineReduction(int16_t depth, int32_t moveCount, bool isCheckEvasion = false);

        constexpr bool isMateLine() { return mateDistance != MAX_PLY; }

    public:
        NewSingleThreadedSearchTree() = delete;

        NewSingleThreadedSearchTree(Evaluator& e, uint32_t numVariations = 1) : SearchTree(e, numVariations) {
            currentMaxDepth = 0;
            currentAge = board->getPly();
            nodesSearched = 0;
            searching = false;
        }

        ~NewSingleThreadedSearchTree() = default;

        virtual void search(uint32_t searchTime, bool dontBlock = false) override;

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
        static constexpr int32_t DEFAULT_CAPTURE_MOVE_SCORE = 100;

        static constexpr int32_t NEUTRAL_SEE_SCORE = 0;

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
                          0,  0,   0,   0,   0,   0,   0,   0,
                        -35, -1, -20, -23, -15,  24,  38, -22,
                        -26, -4,  -4, -10,   3,   3,  33, -12,
                        -27, -2,  -5,  12,  17,   6,  10, -25,
                        -14, 13,   6,   5,   6,  12,  17, -23,
                        -6,   7,  13,  -4,  -8,  27,  12, -10,
                        49,  67,  30,  47,  34,  63,  17,  -6,
                         0,   0,   0,   0,   0,   0,   0,   0,
                },
                // Knight
                {
                        -105, -21, -58, -33, -17, -28, -19,  -23,
                         -29, -53, -12,  -3,  -1,  18, -14,  -19,
                          11,  -9,  12,  10,  19,  17,  25,  -16,
                          -8,  21,  19,  28,  13,  16,   4,  -13,
                          22,  18,  69,  37,  53,  19,  17,  -9,
                          44,  73, 129,  84,  65,  37,  60,  -47,
                         -17,   7,  62,  23,  36,  72, -41,  -73,
                        -107, -15, -97,  61, -49, -34, -89, -167,
                },
                // Bishop
                {
                        -33,  -3, -14, -21, -13, -12, -39, -21,
                          4,  15,  16,   0,   7,  21,  33,   1,
                          0,  15,  15,  15,  14,  27,  18,  10,
                         -6,  13,  13,  26,  34,  12,  10,   4,
                         -4,   5,  19,  50,  37,  37,   7,  -2,
                        -16,  37,  43,  40,  35,  50,  37,  -2,
                        -26,  16, -18, -13,  30,  59,  18, -47,
                        -29,   4, -82, -37, -25, -42,   7,  -8,
                },
                // Rook
                {
                        -19, -13,   1,  17, 16,  7, -37, -26,
                        -44, -16, -20,  -9, -1, 11,  -6, -71,
                        -45, -25, -16, -17,  3,  0,  -5, -33,
                        -36, -26, -12,  -1,  9, -7,   6, -23,
                        -24, -11,   7,  26, 24, 35,  -8, -20,
                         -5,  19,  26,  36, 17, 45,  61,  16,
                         27,  32,  58,  62, 80, 67,  26,  44,
                         32,  42,  32,  51, 63,  9,  31,  43,
                },
                // Queen
                {
                        -10, -28,  -9,  10, -15, -25, -41, -60,
                        -45, -18,  11,   2,   8,  15, -13,  -9,
                        -24,  -8, -11,  -2,  -5,   2,   4,  -5,
                        -19, -36,  -9, -10,  -2,  -4,  -7, -13,
                        -37, -37, -16, -16,  -1,  17, -12, -91,
                        -23, -27,   7,   8,  29,  56,  37,  47,
                        -34, -49,  -5,   1, -16,  57,  18,  44,
                        -38, -10,  29,  12,  59,  44,  33,  35,
                },
                // King
                {
                        -15,  36,  12, -54,   8, -28,  24,  14,
                          1,   7,  -8, -64, -43, -16,   9,   8,
                        -14, -14, -22, -46, -44, -30, -15, -27,
                        -49,  -1, -27, -39, -46, -44, -33, -51,
                        -17, -20, -12, -27, -30, -25, -14, -36,
                         -9,  24,   2, -16, -20,   6,  22, -22,
                         29,  -1, -20,  -7,  -8,  -4, -38, -29,
                        -65,  23,  16, -15, -56, -34,   2,  13,
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