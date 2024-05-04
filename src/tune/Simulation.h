#ifndef DISABLE_THREADS

#ifndef SIMULATION_H
#define SIMULATION_H

#include "core/chess/Board.h"

#include <stdint.h>
#include <vector>

enum GameResult {
    WHITE_WIN,
    BLACK_WIN,
    DRAW
};

class Simulation {

    private:
        std::vector<Board>& startingPositions;
        std::vector<GameResult> results;
        uint32_t timeControl;
        uint32_t increment;
        size_t numThreads;

        GameResult simulateSingleGame(Board& board);

        static constexpr int32_t DECISIVE_SCORE = 1000; // 10 cp

    public:
        Simulation(std::vector<Board>& startingPositions, uint32_t timeControl, uint32_t increment);
        Simulation(std::vector<Board>& startingPositions, uint32_t timeControl, uint32_t increment, size_t numThreads);

        void run();

        inline std::vector<GameResult>& getResults() {
            return results;
        }
};

#endif

#endif