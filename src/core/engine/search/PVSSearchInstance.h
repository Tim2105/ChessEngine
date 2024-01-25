#ifndef PVS_SEARCH_INSTANCE_H
#define PVS_SEARCH_INSTANCE_H

#include "core/chess/Board.h"

#include "core/engine/evaluation/StaticEvaluator.h"
#include "core/engine/evaluation/NNUEEvaluator.h"
#include "core/engine/search/SearchDefinitions.h"

#include "core/utils/tables/TranspositionTable.h"

#include <array>
#include <atomic>
#include <chrono>
#include <functional>
#include <cstdalign>

class PVSSearchInstance {
    private:
        struct MoveStackEntry {
            Array<MoveScorePair, 256> moveScorePairs;
            Move hashMove = Move::nullMove();
        };

        bool isMainThread = false;

        Board board;
        #if defined(USE_HCE)
            StaticEvaluator evaluator;
        #else
            NNUEEvaluator evaluator;
        #endif

        TranspositionTable& transpositionTable;

        alignas(64) Move killerMoves[MAX_PLY][2];
        alignas(64) int32_t historyTable[2][64][64];
        alignas(64) Array<Move, MAX_PLY> pvTable[MAX_PLY];

        std::atomic_bool& stopFlag;

        std::atomic<std::chrono::system_clock::time_point>& startTime;
        std::atomic<std::chrono::system_clock::time_point>& stopTime;

        std::atomic_uint64_t& nodesSearched;
        uint64_t locallySearchedNodes = 0;
        int16_t currentSearchDepth = 0;

        Array<MoveStackEntry, MAX_PLY> moveStack;

        const Array<Move, 256>& searchMoves;

        std::function<void()> checkupFunction;

        int16_t quiescence(int16_t ply, int16_t alpha, int16_t beta);

        int16_t determineExtension();
        int16_t determineReduction(int16_t moveCount, uint8_t nodeType);

        bool deactivateNullMove();

        void scoreMoves(const Array<Move, 256>& moves, uint16_t ply);
        void scoreMovesForQuiescence(const Array<Move, 256>& moves, uint16_t ply);

        MoveScorePair selectNextMove(uint16_t ply, bool useIID, int16_t depth);
        MoveScorePair selectNextMoveInQuiescence(uint16_t ply, int16_t minScore = MIN_SCORE + 1);

    public:
        PVSSearchInstance(Board& board, TranspositionTable& transpositionTable,
                          std::atomic_bool& stopFlag, std::atomic<std::chrono::system_clock::time_point>& startTime,
                          std::atomic<std::chrono::system_clock::time_point>& stopTime, std::atomic_uint64_t& nodesSearched,
                          const Array<Move, 256>& searchMoves, std::function<void()> checkupFunction) :
            board(board), evaluator(this->board), transpositionTable(transpositionTable), stopFlag(stopFlag), startTime(startTime), 
            stopTime(stopTime), nodesSearched(nodesSearched), moveStack(), searchMoves(searchMoves), checkupFunction(checkupFunction) {

            for(int16_t i = 0; i < MAX_PLY; i++) {
                killerMoves[i][0] = Move::nullMove();
                killerMoves[i][1] = Move::nullMove();
            }

            for(int16_t i = 0; i < 2; i++)
                for(int16_t j = 0; j < 64; j++)
                    for(int16_t k = 0; k < 64; k++)
                        historyTable[i][j][k] = 0;

            for(int16_t i = 0; i < MAX_PLY; i++)
                pvTable[i].clear();
        }

        int16_t pvs(int16_t depth, uint16_t ply, int16_t alpha, int16_t beta, bool allowNullMove, uint8_t nodeType);

        inline void setMainThread(bool isMainThread) {
            this->isMainThread = isMainThread;
        }

        inline void setCurrentSearchDepth(int16_t currentSearchDepth) {
            this->currentSearchDepth = currentSearchDepth;
        }

        inline Array<Move, MAX_PLY>& getPV() {
            return pvTable[0];
        }

    private:
        inline void addPVMove(uint16_t ply, Move move) {
            pvTable[ply].clear();
            pvTable[ply].push_back(move);

            if(ply < MAX_PLY - 1)
                pvTable[ply].push_back(pvTable[ply + 1]);
        }

        constexpr void clearPVTable(uint16_t ply) {
            pvTable[ply].clear();
        }

        constexpr void clearMoveStack(uint16_t ply) {
            moveStack[ply].moveScorePairs.clear();
            moveStack[ply].hashMove = Move::nullMove();
        }

        constexpr void addKillerMove(uint16_t ply, Move move) {
            if(move != killerMoves[ply][1]) {
                killerMoves[ply][0] = killerMoves[ply][1];
                killerMoves[ply][1] = move;
            }
        }

        constexpr bool isKillerMove(uint16_t ply, Move move) {
            return move == killerMoves[ply][0] || move == killerMoves[ply][1];
        }

        constexpr void incrementHistoryScore(Move move, int16_t depth) {
            historyTable[board.getSideToMove() / COLOR_MASK]
                        [move.getOrigin()]
                        [move.getDestination()] += (depth / ONE_PLY) * (depth / ONE_PLY);
        }

        inline void decrementHistoryScore(Move move, int16_t depth) {
            historyTable[board.getSideToMove() / COLOR_MASK]
                        [move.getOrigin()]
                        [move.getDestination()] -= depth / ONE_PLY;
        }

        inline int32_t getHistoryScore(Move move) {
            return historyTable[board.getSideToMove() / COLOR_MASK]
                               [move.getOrigin()]
                               [move.getDestination()];
        }

        inline int32_t getHistoryScore(Move move, int32_t side) {
            return historyTable[side / COLOR_MASK]
                               [move.getOrigin()]
                               [move.getDestination()];
        }

        static constexpr int16_t DELTA_MARGIN = 2000;

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