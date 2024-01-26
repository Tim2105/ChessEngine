#ifndef PVS_ENGINE_H
#define PVS_ENGINE_H

#include <array>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

#include "core/engine/search/PVSSearchInstance.h"
#include "core/engine/search/SearchDefinitions.h"
#include "core/engine/search/Variation.h"
#include "core/engine/evaluation/Evaluator.h"

#include "core/utils/tables/TranspositionTable.h"

#include "uci/UCI.h"

#define UNUSED(x) (void)(x)

class PVSEngine {
    private:
        Board& board;

        std::vector<Variation> variations;
        uint32_t numVariations = 0;

        TranspositionTable transpositionTable;

        std::chrono::system_clock::time_point lastCheckupTime;
        std::chrono::milliseconds checkupInterval;

        std::function<void()> checkupCallback;

        std::atomic_bool stopFlag = true;
        bool isTimeControlled = false;
        std::atomic_bool isPondering = false;
        std::atomic<std::chrono::system_clock::time_point> startTime;
        std::atomic<std::chrono::system_clock::time_point> stopTime;
        std::chrono::milliseconds timeMin;
        std::chrono::milliseconds timeMax;

        std::atomic_uint64_t nodesSearched = 0;
        int16_t maxDepthReached = 0;

        struct MoveStackEntry {
            Array<MoveScorePair, 256> moveScorePairs;
            Move hashMove = Move::nullMove();
        };

        Array<MoveStackEntry, MAX_PLY> moveStack;
        Array<MoveScorePair, 5> pvHistory;

        std::vector<std::thread> threads;
        std::vector<PVSSearchInstance*> instances;

        void createHelperThreads(size_t numThreads, const UCI::SearchParams& params);
        void destroyHelperThreads();

        void startHelperThreads(int16_t depth, int16_t alpha, int16_t beta);
        void stopHelperThreads();

        void calculateTimeLimits(const UCI::SearchParams& params);
        bool extendSearch(bool isTimeControlled);

        inline bool isCheckupTime() {
            return std::chrono::system_clock::now() >= lastCheckupTime + checkupInterval;
        }

        inline void updateStopFlag() {
            if(std::chrono::system_clock::now() >= startTime.load() + timeMax &&
               maxDepthReached > 0)
                stopFlag.store(true);
        }

        inline void checkup() {
            lastCheckupTime = std::chrono::system_clock::now();

            updateStopFlag();

            if(checkupCallback)
                checkupCallback();
        }

    public:
        PVSEngine(Board& board, uint32_t numVariations = 1,
                  uint32_t checkupInterval = 2, std::function<void()> checkupCallback = nullptr)
                : board(board), numVariations(numVariations),
                  checkupInterval(checkupInterval), checkupCallback(checkupCallback) {}

        PVSEngine(const PVSEngine& other) = delete;
        PVSEngine& operator=(const PVSEngine& other) = delete;

        void search(const UCI::SearchParams& params);

        inline void stop() {
            stopFlag.store(true);
        }

        inline void setHashTableCapacity(size_t capacity) {
            transpositionTable.resize(capacity);
        }

        inline size_t getHashTableCapacity() {
            return transpositionTable.getCapacity();
        }

        inline size_t getHashTableSize() {
            return transpositionTable.getEntriesWritten();
        }

        inline void clearHashTable() {
            transpositionTable.clear();
        }

        inline void setBoard(Board& board) {
            this->board = board;
        }

        inline void setNumVariations(uint32_t numVariations) {
            this->numVariations = numVariations;
        }

        inline void setCheckupCallback(std::function<void()> checkupCallback) {
            this->checkupCallback = checkupCallback;
        }

        inline void setPondering(bool isPondering) {
            this->isPondering.store(isPondering);
        }

        inline Board& getBoard() {
            return board;
        }

        inline Move getBestMove() {
            return variations.empty() ? Move() : variations[0].moves[0];
        }

        inline int16_t getBestMoveScore() {
            return variations.empty() ? 0 : variations[0].score;
        }

        inline std::vector<Variation> getVariations() {
            return variations;
        }

        inline std::vector<Move> getPrincipalVariation() {
            return variations.empty() ? std::vector<Move>() : variations[0].moves;
        }

        inline uint64_t getNodesSearched() const {
            return nodesSearched.load();
        }

        constexpr int16_t getMaxDepthReached() const {
            return maxDepthReached;
        }

        inline int64_t getElapsedTime() const {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - startTime.load()).count();
        }

        inline bool isSearching() const {
            return !stopFlag.load();
        }

    private:
        constexpr void clearPVHistory() {
            pvHistory.clear();
        }
};

#endif