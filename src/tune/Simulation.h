#ifndef DISABLE_THREADS

#ifndef SIMULATION_H
#define SIMULATION_H

#include "core/chess/Board.h"
#include "core/utils/hce/HCEParameters.h"
#include "core/utils/nnue/NNUEInstance.h"

#include <optional>
#include <stdint.h>
#include <vector>

enum GameResult {
    WHITE_WIN,
    BLACK_WIN,
    DRAW
};

class Result {
    public:
        GameResult result;
        std::vector<int> evaluations;
};

class Simulation {
    #ifdef USE_HCE
    using Parameters = HCEParameters;
    #else
    using Parameters = std::reference_wrapper<const NNUE::Network>;
    #endif

    private:
        std::vector<Board>& startingPositions;
        std::vector<Result> results;
        uint32_t timeControl;
        uint32_t increment;
        size_t numThreads;

        std::optional<Parameters> whiteParams;
        std::optional<Parameters> blackParams;

        bool addParameterNoise;
        double noiseStdDev;
        double noiseLinearStdDev;

        Result simulateSingleGame(Board& board);

        static constexpr unsigned int DRAW_AFTER_N_MOVES = 400;

    public:
        Simulation(std::vector<Board>& startingPositions, uint32_t timeControl, uint32_t increment);
        Simulation(std::vector<Board>& startingPositions, uint32_t timeControl, uint32_t increment, size_t numThreads);

        void run();

        inline std::vector<Result>& getResults() {
            return results;
        }

        inline void setWhiteParams(const Parameters& params) {
            whiteParams.emplace(params);
        }

        inline void setBlackParams(const Parameters& params) {
            blackParams.emplace(params);
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