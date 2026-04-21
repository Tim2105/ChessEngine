#include "tune/Simulation.h"

#include "tune/Definitions.h"

#include "core/chess/Referee.h"
#include "core/engine/search/PVSEngine.h"
#include "core/utils/Random.h"

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

std::pair<double, std::vector<Move>> sampleVariation(const std::vector<Variation>& variations, double temperature) {
    if(temperature <= 0.0)
        return {0.0, variations[0].moves};
    
    if(variations.size() == 1)
        return {0.0, variations[0].moves};

    double maxScore = variations[0].score;

    // Log-Sum-Exp-Trick für numerische Stabilität
    double sumExp = 0.0;
    std::vector<double> logProbs(variations.size());
    
    for(size_t i = 0; i < variations.size(); i++) {
        logProbs[i] = (variations[i].score - maxScore) / temperature;
        sumExp += std::exp(logProbs[i]);
    }

    std::uniform_real_distribution<double> dis(0.0, sumExp);
    std::mt19937& rng = Random::generator<1>();
    double rand = dis(rng);
    double cumulative = 0.0;
    
    for(size_t i = 0; i < variations.size(); i++) {
        cumulative += std::exp(logProbs[i]);
        if(rand <= cumulative || i == variations.size() - 1)
            return {logProbs[i], variations[i].moves};
    }

    return {logProbs.back(), variations.back().moves};
}

Result Simulation::simulateSingleGame(Board& board, Parameters whiteParameters, Parameters blackParameters) {
    if(Referee::isCheckmate(board))
        return board.getSideToMove() == WHITE ? Result{BLACK_WIN, {}, {}, {}} : Result{WHITE_WIN, {}, {}, {}};
    else if(Referee::isDraw(board))
        return Result{DRAW, {}, {}, {}};

    Result result;

    #ifdef USE_HCE
    std::vector<double> whiteNoise(currentParams.size(), 0.0);
    std::vector<double> blackNoise(currentParams.size(), 0.0);
    double whiteDecayFactor = 1.0;
    double blackDecayFactor = 1.0;

    if(addParameterNoise) {
        std::mt19937& rng = Random::generator<2>();
        std::normal_distribution<double> defaultDist(0.0, noiseStdDev);
        std::normal_distribution<double> linearDist(0.0, noiseLinearStdDev);

        for(size_t i = 0; i < currentParams.size(); i++) {
            if(!currentParams.isOptimizable(i))
                continue;

            whiteNoise[i] = defaultDist(rng) + linearDist(rng) * std::abs(currentParams[i]);
            blackNoise[i] = defaultDist(rng) + linearDist(rng) * std::abs(currentParams[i]);
        }

        // Berechne verrauschende Parameter
        for(size_t i = 0; i < currentParams.size(); i++) {
            if(!currentParams.isOptimizable(i))
                continue;

            whiteParameters[i] = (int16_t)std::round(currentParams[i] + whiteNoise[i] * whiteDecayFactor);
            blackParameters[i] = (int16_t)std::round(currentParams[i] + blackNoise[i] * blackDecayFactor);
        }
    }

    HandcraftedEvaluator neutralEvaluator(board, currentParams);
    #else
    NNUEEvaluator neutralEvaluator(board, currentParams);
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

                    whiteParameters[i] = (int16_t)std::round(currentParams[i] + whiteNoise[i] * whiteDecayFactor);
                }

                whiteDecayFactor *= noiseDecay;
            }
            #endif

            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            white.search(params);
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

            double logProb = 0.0;

            #ifdef USE_HCE
            std::vector<Move> sampledVar = white.getPrincipalVariation();
            #else
            std::vector<Move> sampledVar;
            std::tie(logProb, sampledVar) = sampleVariation(white.getVariations(), tau);
            tau *= temperatureDecay;
            #endif

            // Laufe die Variante durch und speichere die Bewertung des Blattknotens
            for(Move m : sampledVar)
                board.makeMove(m);

            double evaluation = neutralEvaluator.evaluate();
            if(board.getSideToMove() == BLACK)
                evaluation = -evaluation;
            result.leafEvaluations.push_back(evaluation);
            result.leafFENs.push_back(board.toFEN());
            result.logProbs.push_back(logProb);
            
            // Mache die Züge wieder rückgängig
            for(size_t i = 0; i < sampledVar.size(); i++)
                board.undoMove();

            Move chosenMove = sampledVar[0];
            board.makeMove(chosenMove);

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

                    blackParameters[i] = (int16_t)std::round(currentParams[i] + blackNoise[i] * blackDecayFactor);
                }

                blackDecayFactor *= noiseDecay;
            }
            #endif

            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            black.search(params);
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

            double logProb = 0.0;

            #ifdef USE_HCE
            std::vector<Move> sampledVar = black.getPrincipalVariation();
            #else
            std::vector<Move> sampledVar;
            std::tie(logProb, sampledVar) = sampleVariation(black.getVariations(), tau);
            tau *= temperatureDecay;
            #endif

            // Laufe die Variante durch und speichere die Bewertung des Blattknotens
            for(Move m : sampledVar)
                board.makeMove(m);

            double evaluation = neutralEvaluator.evaluate();
            if(board.getSideToMove() == BLACK)
                evaluation = -evaluation;
            result.leafEvaluations.push_back(evaluation);
            result.leafFENs.push_back(board.toFEN());
            result.logProbs.push_back(logProb);

            // Mache die Züge wieder rückgängig
            for(size_t i = 0; i < sampledVar.size(); i++)
                board.undoMove();

            Move chosenMove = sampledVar[0];
            board.makeMove(chosenMove);

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

            Result result = simulateSingleGame(board, currentParams, currentParams);
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

void Simulation::run(EloTableType& eloTable, double playerChoiceTemperature) {
    std::cout << "Refining Elo ratings with " << startingPositions.size() << " games ("
              << timeControl / 1000.0 << "s + " << increment / 1000.0 << "s increment) "
              << "with " << numThreads << " threads..." << std::endl;

    // Bestimme die Matchups
    std::vector<std::pair<std::string, std::string>> matchups(startingPositions.size());
    for(size_t i = 0; i < startingPositions.size(); i++) {
        std::string player1 = eloTable.getRandomPlayerName(playerChoiceTemperature);
        std::string player2 = eloTable.getRandomPlayerNameExcluding(player1, playerChoiceTemperature);
        matchups[i] = {player1, player2};
    }

    results.resize(startingPositions.size() * 2); // Platz für Hin- und Rückspiel

    std::cout << "Remaining games: " << std::left << std::setw(7) << startingPositions.size();
    std::cout << std::right << " | Last Result: " << std::setw(40) << "N/A" << std::flush;

    std::mutex lock;
    std::atomic_int64_t currentIdx = startingPositions.size() - 1;
    std::atomic_uint64_t completedGames = 0;

    auto threadFunction = [&]() {
        lock.lock();
        while(true) {
            if(currentIdx.load() < 0) {
                lock.unlock();
                break;
            }

            size_t index = currentIdx.fetch_sub(1);
            Board board1 = startingPositions[index];
            Board board2 = startingPositions[index];
            std::string player1 = matchups[index].first;
            std::string player2 = matchups[index].second;
            lock.unlock();

            Parameters params1 = eloTable.getData(player1);
            Parameters params2 = eloTable.getData(player2);

            // Hinspiel
            Result result1 = simulateSingleGame(board1, params1, params2);
            results[index * 2] = result1;

            // Rückspiel
            Result result2 = simulateSingleGame(board2, params2, params1);
            results[index * 2 + 1] = result2;

            // Sieger bestimmen
            float score1 = 0.0f, score2 = 0.0f;
            switch(result1.result) {
                case WHITE_WIN:
                    score1 += 1.0f;
                    break;
                case BLACK_WIN:
                    score2 += 1.0f;
                    break;
                case DRAW:
                    score1 += 0.5f;
                    score2 += 0.5f;
                    break;
            }

            switch(result2.result) {
                case WHITE_WIN:
                    score2 += 1.0f;
                    break;
                case BLACK_WIN:
                    score1 += 1.0f;
                    break;
                case DRAW:
                    score1 += 0.5f;
                    score2 += 0.5f;
                    break;
            }

            GameResult finalResult;
            if(score1 > score2)
                finalResult = WHITE_WIN;
            else if(score2 > score1)
                finalResult = BLACK_WIN;
            else
                finalResult = DRAW;

            completedGames.fetch_add(1);
            lock.lock();
            eloTable.addGameResult(player1, player2, finalResult);

            std::stringstream resultSS;
            resultSS << player1 << "(" << score1 << ") vs " << player2 << "(" << score2 << ")";

            std::stringstream ss;
            ss << "\rRemaining games: " << std::left << std::setw(7) << startingPositions.size() - completedGames;
            ss << std::right << " | Last Result: " << std::setw(40) << resultSS.str();
            std::cout << ss.str() << std::flush;
        }

        lock.unlock();
    };

    std::vector<std::thread> threads;
    for(size_t i = 0; i < numThreads; i++)
        threads.push_back(std::thread(threadFunction));

    for(size_t i = 0; i < numThreads; i++)
        threads[i].join();

    std::cout << std::endl;
}