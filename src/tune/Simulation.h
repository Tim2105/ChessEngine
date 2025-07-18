#ifndef DISABLE_THREADS

#ifndef SIMULATION_H
#define SIMULATION_H

#include "core/chess/Board.h"
#include "core/utils/hce/HCEParameters.h"

#include <optional>
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

        std::optional<HCEParameters> whiteParams;
        std::optional<HCEParameters> blackParams;

        bool addParameterNoise;
        double noiseStdDev;
        double noiseLinearStdDev;

        GameResult simulateSingleGame(Board& board);

        static constexpr unsigned int DRAW_AFTER_N_MOVES = 400;

    public:
        Simulation(std::vector<Board>& startingPositions, uint32_t timeControl, uint32_t increment);
        Simulation(std::vector<Board>& startingPositions, uint32_t timeControl, uint32_t increment, size_t numThreads);

        void run();

        inline std::vector<GameResult>& getResults() {
            return results;
        }

        inline void setWhiteParams(const HCEParameters& params) {
            whiteParams = params;
        }

        inline void setBlackParams(const HCEParameters& params) {
            blackParams = params;
        }

        inline void setParameterNoise(double stdDev) {
            addParameterNoise = true;
            noiseStdDev = stdDev;
        }

        inline void setLinearParameterNoise(double stdDev) {
            addParameterNoise = true;
            noiseLinearStdDev = stdDev;
        }

        inline void setNoParameterNoise() {
            addParameterNoise = false;
        }
};

#endif

#endif