#include "tune/Simulation.h"

#include "core/chess/Referee.h"
#include "core/engine/search/PVSEngine.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>

Simulation::Simulation(std::vector<Board>& startingPositions, uint32_t timeControl, uint32_t increment) :
    Simulation(startingPositions, timeControl, increment, 0) {
    
    // Beanspruche alle Threads bis auf 2 oder 3/4 der Threads, je nachdem was größer ist
    unsigned int hwThreads = std::thread::hardware_concurrency();
    this->numThreads = std::max(hwThreads * 3 / 4, std::max(hwThreads - 2, 1u));
}

Simulation::Simulation(std::vector<Board>& startingPositions, uint32_t timeControl, uint32_t increment, size_t numThreads):
    startingPositions(startingPositions), timeControl(timeControl), increment(increment), numThreads(std::max(numThreads, (size_t)1)),
    addParameterNoise(false), noiseStdDev(0.0), noiseLinearStdDev(0.0) {

    results.resize(startingPositions.size());
}

thread_local std::mt19937 rng(std::random_device{}());

std::vector<Move> sampleMove(const std::vector<Variation>& variations, double temperature) {
    if(temperature <= 0.0)
        return variations[0].moves;
    
    if(variations.size() == 1)
        return variations[0].moves;

    // Log-Sum-Exp-Trick für numerische Stabilität
    double maxScore = variations[0].score;
    for(const Variation& var : variations)
        maxScore = std::max(maxScore, (double)var.score);

    double sumExp = 0.0;
    std::vector<double> logProbs(variations.size());
    
    for(size_t i = 0; i < variations.size(); i++) {
        logProbs[i] = (variations[i].score - maxScore) / temperature;
        sumExp += std::exp(logProbs[i]);
    }

    std::uniform_real_distribution<double> dis(0.0, sumExp);
    double rand = dis(rng);
    double cumulative = 0.0;
    
    for(size_t i = 0; i < variations.size(); i++) {
        cumulative += std::exp(logProbs[i]);
        if(rand <= cumulative || i == variations.size() - 1)
            return variations[i].moves;
    }

    return variations.back().moves;
}

Result Simulation::simulateSingleGame(Board& board) {
    if(Referee::isCheckmate(board))
        return board.getSideToMove() == WHITE ? Result{BLACK_WIN, {}, {}} : Result{WHITE_WIN, {}, {}};
    else if(Referee::isDraw(board))
        return Result{DRAW, {}, {}};

    Result result;

    #ifdef USE_HCE
    HCEParameters parameters = params.value_or(HCEParameters());
    HCEParameters whiteParameters = parameters;
    HCEParameters blackParameters = parameters;
    std::vector<double> whiteNoise(parameters.size(), 0.0);
    std::vector<double> blackNoise(parameters.size(), 0.0);
    double whiteDecayFactor = 1.0;
    double blackDecayFactor = 1.0;

    if(addParameterNoise) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<double> defaultDist(0.0, noiseStdDev);
        std::normal_distribution<double> linearDist(0.0, noiseLinearStdDev);

        for(size_t i = 0; i < parameters.size(); i++) {
            if(!parameters.isOptimizable(i))
                continue;

            whiteNoise[i] = defaultDist(gen) + linearDist(gen) * std::abs(parameters[i]);
            blackNoise[i] = defaultDist(gen) + linearDist(gen) * std::abs(parameters[i]);
        }

        // Berechne verrauschende Parameter
        for(size_t i = 0; i < parameters.size(); i++) {
            if(!parameters.isOptimizable(i))
                continue;

            whiteParameters[i] = (int16_t)std::round(parameters[i] + whiteNoise[i] * whiteDecayFactor);
            blackParameters[i] = (int16_t)std::round(parameters[i] + blackNoise[i] * blackDecayFactor);
        }
    }

    HandcraftedEvaluator neutralEvaluator(board, parameters);
    #else
    Parameters parameters = params.value_or(NNUE::DEFAULT_NETWORK);
    Parameters whiteParameters = parameters;
    Parameters blackParameters = parameters;

    NNUEEvaluator neutralEvaluator(board, parameters);

    double tau = temperature;
    #endif

    PVSEngine white(board, whiteParameters);
    white.setUCIOutput(false);
    PVSEngine black(board, blackParameters);
    black.setUCIOutput(false);

    int32_t wtime = timeControl;
    int32_t btime = timeControl;

    while(true) {
        UCI::SearchParams params = {
            .wtime = (uint32_t)wtime,
            .btime = (uint32_t)btime,
            .winc = increment,
            .binc = increment,
            .useWBTime = true
        };

        if(board.getSideToMove() == WHITE) {
            #ifdef USE_HCE
            if(addParameterNoise) {
                // Berechne verrauschende Parameter für Weiß
                for(size_t i = 0; i < whiteParameters.size(); i++) {
                    if(!whiteParameters.isOptimizable(i))
                        continue;

                    whiteParameters[i] = (int16_t)std::round(parameters[i] + whiteNoise[i] * whiteDecayFactor);
                }

                whiteDecayFactor *= noiseDecay;
            }
            #endif

            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            white.search(params);
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

            #ifdef USE_HCE
            const std::vector<Move>& bestVar = white.getPrincipalVariation();
            #else
            std::vector<Move> bestVar = sampleMove(white.getVariations(), tau);
            tau *= temperatureDecay;
            #endif

            // Laufe die Variante durch und speichere die Bewertung des Blattknotens
            for(Move m : bestVar)
                board.makeMove(m);

            double evaluation = neutralEvaluator.evaluate();
            if(board.getSideToMove() == BLACK)
                evaluation = -evaluation;
            result.leafEvaluations.push_back(evaluation);
            result.leafFENs.push_back(board.toFEN());
            
            // Mache die Züge wieder rückgängig
            for(size_t i = 0; i < bestVar.size(); i++)
                board.undoMove();

            Move bestMove = bestVar[0];
            board.makeMove(bestMove);

            wtime -= std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            wtime = std::max(wtime, 0);
            wtime += increment;

            if(Referee::isCheckmate(board)) {
                result.result = WHITE_WIN;
                return result;
            }
        } else {
            #ifdef USE_HCE
            if(addParameterNoise) {
                // Berechne verrauschende Parameter für Schwarz
                for(size_t i = 0; i < blackParameters.size(); i++) {
                    if(!blackParameters.isOptimizable(i))
                        continue;

                    blackParameters[i] = (int16_t)std::round(parameters[i] + blackNoise[i] * blackDecayFactor);
                }

                blackDecayFactor *= noiseDecay;
            }
            #endif

            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            black.search(params);
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

            #ifdef USE_HCE
            const std::vector<Move>& bestVar = black.getPrincipalVariation();
            #else
            std::vector<Move> bestVar = sampleMove(black.getVariations(), tau);
            tau *= temperatureDecay;
            #endif

            // Laufe die Variante durch und speichere die Bewertung des Blattknotens
            for(Move m : bestVar)
                board.makeMove(m);

            double evaluation = neutralEvaluator.evaluate();
            if(board.getSideToMove() == BLACK)
                evaluation = -evaluation;
            result.leafEvaluations.push_back(evaluation);
            result.leafFENs.push_back(board.toFEN());

            // Mache die Züge wieder rückgängig
            for(size_t i = 0; i < bestVar.size(); i++)
                board.undoMove();

            Move bestMove = bestVar[0];
            board.makeMove(bestMove);

            btime -= std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            btime = std::max(btime, 0);
            btime += increment;

            if(Referee::isCheckmate(board)) {
                result.result = BLACK_WIN;
                return result;
            }
        }

        if(Referee::isDraw(board) || board.getAge() >= DRAW_AFTER_N_MOVES) {
            result.result = DRAW;
            return result;
        }
    }
}

void Simulation::run() {
    std::cout << "Simulating " << startingPositions.size() << " games ("
              << timeControl / 1000.0 << "s + " << increment / 1000.0 << "s increment) "
              << "with " << numThreads << " threads..." << std::endl;

    std::cout << "Remaining games: " << std::left << std::setw(7) << startingPositions.size();
    std::cout << std::right << " | White wins: " << std::setw(7) << 0;
    std::cout << " | Black wins: " << std::setw(7) << 0;
    std::cout << " | Draws: " << std::setw(7) << 0 << std::flush;

    std::mutex lock;
    std::atomic_int64_t currentIdx = startingPositions.size() - 1;
    std::atomic_uint64_t whiteWins = 0, blackWins = 0, draws = 0;
    std::atomic_uint64_t completedGames = 0;

    auto threadFunction = [&]() {
        lock.lock();
        while(true) {
            if(currentIdx.load() < 0) {
                lock.unlock();
                break;
            }

            size_t index = currentIdx.fetch_sub(1);
            Board& board = startingPositions[index];
            lock.unlock();

            Result result = simulateSingleGame(board);
            results[index] = result;
            completedGames.fetch_add(1);

            switch(result.result) {
                case WHITE_WIN:
                    whiteWins.fetch_add(1);
                    break;
                case BLACK_WIN:
                    blackWins.fetch_add(1);
                    break;
                case DRAW:
                    draws.fetch_add(1);
                    break;
            }

            lock.lock();
            std::stringstream ss;
            ss << "\rRemaining games: " << std::left << std::setw(7) << startingPositions.size() - completedGames;
            ss << std::right << " | White wins: " << std::setw(7) << whiteWins.load();
            ss << " | Black wins: " << std::setw(7) << blackWins.load();
            ss << " | Draws: " << std::setw(7) << draws.load();
            std::cout << ss.str() << std::flush;
        }

        lock.unlock();
    };

    std::vector<std::thread> threads;
    for(size_t i = 0; i < numThreads; i++)
        threads.push_back(std::thread(threadFunction));

    for(size_t i = 0; i < numThreads; i++)
        threads[i].join();

    std::cout << "\rRemaining games: " << std::left << std::setw(7) << 0;
    std::cout << std::right;
    std::cout << " | White wins: " << std::setw(7) << whiteWins.load();
    std::cout << " | Black wins: " << std::setw(7) << blackWins.load();
    std::cout << " | Draws: " << std::setw(7) << draws.load() << std::endl;
}