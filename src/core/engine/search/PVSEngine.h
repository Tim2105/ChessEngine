#ifndef PVS_ENGINE_H
#define PVS_ENGINE_H

#include <atomic>

#include "core/engine/search/InterruptedEngine.h"
#include "core/engine/search/SearchDefinitions.h"
#include "core/engine/evaluation/UpdatedEvaluator.h"

#include "core/utils/tables/TranspositionTable.h"

class PVSEngine: public InterruptedEngine {
    public:
        struct MoveScorePair {
            Move move;
            int16_t score;
        };

    private:
        /**
         * @brief Das Ersetzungsprädikat für die Transpositionstabelle.
         * Ein Eintrag wird ersetzt, wenn die Funktion true zurückgibt.
         * 
         * @param lhs Der neue Eintrag.
         * @param rhs Der alte Eintrag.
         */
        static constexpr bool ttReplacementPredicate(const TranspositionTableEntry& lhs,
                                                     const TranspositionTableEntry& rhs) {
            
            return lhs.depth * 11 / 10 + lhs.age * ONE_PLY > rhs.depth * 11 / 10 + rhs.age * ONE_PLY;
        };

        TranspositionTable<ttReplacementPredicate> transpositionTable;
        Move killerMoves[MAX_PLY][2];
        int32_t historyTable[2][64][64];
        Array<Move, MAX_PLY> pvTable[MAX_PLY];

        UpdatedEvaluator& evaluator;
        Board boardCopy;

        std::atomic_bool stopFlag = true;
        std::atomic_bool isTimeControlled = false;
        std::chrono::system_clock::time_point startTime;
        std::chrono::milliseconds timeMin;
        std::chrono::milliseconds timeMax;

        uint64_t nodesSearched = 0;
        int16_t maxDepthReached = 0;

        struct MoveStackEntry {
            Array<MoveScorePair, 256> moveScorePairs;
            Move hashMove = Move::nullMove();
        };

        Array<MoveStackEntry, MAX_PLY> moveStack;
        Array<MoveScorePair, 5> pvHistory;

        int16_t pvs(int16_t depth, uint16_t ply, int16_t alpha, int16_t beta, bool allowNullMove, bool isPVNode);
        int16_t quiescence(int16_t ply, int16_t alpha, int16_t beta);

        void collectPVLine(int16_t score);
        int16_t determineExtension(bool isCheckEvasion);
        int16_t determineReduction(int16_t moveCount, int16_t moveScore);

        bool deactivateNullMove();

        void scoreMoves(const Array<Move, 256>& moves, uint16_t ply);
        void scoreMovesForQuiescence(const Array<Move, 256>& moves, uint16_t ply);
        MoveScorePair selectNextMove(uint16_t ply);
        MoveScorePair selectNextMoveInQuiescence(uint16_t ply, int16_t minScore = MIN_SCORE + 1);

        void calculateTimeLimits(uint32_t time, bool treatAsTimeControl);
        bool extendSearch(bool isTimeControlled);

        inline void updateStopFlag() {
            if(std::chrono::system_clock::now() >= startTime + timeMax &&
               maxDepthReached > 0)
                stopFlag = true;
        }

    public:
        PVSEngine(UpdatedEvaluator& evaluator, uint32_t numVariations = 1,
                  uint32_t checkupInterval = 2, std::function<void()> checkupCallback = nullptr)
                : InterruptedEngine(evaluator, numVariations, checkupInterval, [this, checkupCallback]{
                    updateStopFlag();

                    if(checkupCallback)
                        checkupCallback();
                }), evaluator(evaluator) {}

        PVSEngine(const PVSEngine& other) = delete;
        PVSEngine& operator=(const PVSEngine& other) = delete;

        void search(uint32_t time, bool treatAsTimeControl = false) override;

        inline void stop() override {
            stopFlag = true;
        }

        inline void setTime(uint32_t time, bool treatAsTimeControl = false) override {
            isTimeControlled = treatAsTimeControl;
            calculateTimeLimits(time, treatAsTimeControl);
        }

        inline void setHashTableSize(uint32_t size) override {
            transpositionTable.resize(size / 2);
        }

        inline size_t getHashTableSize() override {
            return transpositionTable.getBucketCount() * 2;
        }

        inline void clearHashTable() override {
            transpositionTable.clear();
        }

        inline void setBoard(Board& board) override {
            Engine::setBoard(board);
        }

        inline void setCheckupCallback(std::function<void()> checkupCallback) override {
            InterruptedEngine::setCheckupCallback([this, checkupCallback]{
                updateStopFlag();

                if(checkupCallback)
                    checkupCallback();
            });
        }

        constexpr uint64_t getNodesSearched() const {
            return nodesSearched;
        }

        constexpr int16_t getMaxDepthReached() const {
            return maxDepthReached;
        }

        inline int64_t getElapsedTime() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime).count();
        }

        inline bool isSearching() const {
            return !stopFlag;
        }

    private:
        constexpr void clearMoveStack(uint16_t ply) {
            moveStack[ply].moveScorePairs.clear();
            moveStack[ply].hashMove = Move::nullMove();
        }

        constexpr void clearKillerMoves() {
            for(size_t i = 0; i < MAX_PLY; i++) {
                killerMoves[i][0] = Move::nullMove();
                killerMoves[i][1] = Move::nullMove();
            }
        }

        constexpr void addKillerMove(uint16_t ply, Move move) {
            if(move != killerMoves[ply][0]) {
                killerMoves[ply][1] = killerMoves[ply][0];
                killerMoves[ply][0] = move;
            }
        }

        constexpr bool isKillerMove(uint16_t ply, Move move) {
            return move == killerMoves[ply][0] || move == killerMoves[ply][1];
        }

        constexpr void clearHistoryTable() {
            for(size_t i = 0; i < 64; i++)
                for(size_t j = 0; j < 64; j++) {
                    historyTable[0][i][j] = 0;
                    historyTable[1][i][j] = 0;
                }
        }

        constexpr void incrementHistoryScore(Move move, int16_t depth) {
            historyTable[boardCopy.getSideToMove() / COLOR_MASK]
                        [move.getOrigin()]
                        [move.getDestination()] += (depth / ONE_PLY) * (depth / ONE_PLY);
        }

        constexpr void decrementHistoryScore(Move move, int16_t depth) {
            historyTable[boardCopy.getSideToMove() / COLOR_MASK]
                        [move.getOrigin()]
                        [move.getDestination()] -= (depth / ONE_PLY);
        }

        constexpr int32_t getHistoryScore(Move move) {
            return historyTable[boardCopy.getSideToMove() / COLOR_MASK]
                               [move.getOrigin()]
                               [move.getDestination()];
        }

        constexpr int32_t getHistoryScore(Move move, int32_t side) {
            return historyTable[side / COLOR_MASK]
                               [move.getOrigin()]
                               [move.getDestination()];
        }

        constexpr void clearPVTable() {
            for(size_t i = 0; i < MAX_PLY; i++)
                pvTable[i].clear();
        }

        constexpr void clearPVTable(uint16_t ply) {
            pvTable[ply].clear();
        }

        inline void addPVMove(uint16_t ply, Move move) {
            pvTable[ply].clear();
            pvTable[ply].push_back(move);

            if(ply < MAX_PLY - 1)
                pvTable[ply].push_back(pvTable[ply + 1]);
        }

        constexpr Array<Move, MAX_PLY>& getPVLine() {
            return pvTable[0];
        }

        constexpr void clearPVHistory() {
            pvHistory.clear();
        }

        static constexpr int16_t DELTA_MARGIN = 1000;

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

template<>
struct std::greater<PVSEngine::MoveScorePair> {
    bool operator()(const PVSEngine::MoveScorePair& lhs, const PVSEngine::MoveScorePair& rhs) const {
        return lhs.score > rhs.score;
    }
};

template<>
struct std::less<PVSEngine::MoveScorePair> {
    bool operator()(const PVSEngine::MoveScorePair& lhs, const PVSEngine::MoveScorePair& rhs) const {
        return lhs.score < rhs.score;
    }
};

#endif