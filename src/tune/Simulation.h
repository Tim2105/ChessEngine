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
        std::vector<int> leafEvaluations;
        std::vector<std::string> leafFENs;
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

        std::optional<Parameters> params;

        bool addParameterNoise;
        double noiseStdDev;
        double noiseLinearStdDev;
        double noiseDecay;
        double temperature;
        double temperatureDecay;

        Result simulateSingleGame(Board& board);

        static constexpr unsigned int DRAW_AFTER_N_MOVES = 400;

    public:
        Simulation(std::vector<Board>& startingPositions, uint32_t timeControl, uint32_t increment);
        Simulation(std::vector<Board>& startingPositions, uint32_t timeControl, uint32_t increment, size_t numThreads);

        void run();

        inline std::vector<Result>& getResults() {
            return results;
        }

        inline void setParams(const Parameters& newParams) {
            params.emplace(newParams);
        }

        inline void setParameterNoise(double stdDev) {
            addParameterNoise = true;
            noiseStdDev = stdDev;
        }

        inline void setLinearParameterNoise(double stdDev) {
            addParameterNoise = true;
            noiseLinearStdDev = stdDev;
        }

        inline void setNoiseDecay(double decay) {
            noiseDecay = decay;
        }

        inline void setNoParameterNoise() {
            addParameterNoise = false;
        }

        inline void setTemperature(double t) {
            temperature = t;
        }

        inline void setTemperatureDecay(double decay) {
            temperatureDecay = decay;
        }
};

#endif

#endif