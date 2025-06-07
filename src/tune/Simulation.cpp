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

GameResult Simulation::simulateSingleGame(Board& board) {
    if(Referee::isCheckmate(board))
        return board.getSideToMove() == WHITE ? BLACK_WIN : WHITE_WIN;
    else if(Referee::isDraw(board))
        return DRAW;

    HCEParameters whiteParameters = whiteParams.value_or(HCEParameters());
    HCEParameters blackParameters = blackParams.value_or(HCEParameters());

    if(addParameterNoise) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<double> defaultDist(0.0, noiseStdDev);
        std::normal_distribution<double> linearDist(0.0, noiseLinearStdDev);

        for(size_t i = 0; i < whiteParameters.size(); i++) {
            if(!whiteParameters.isOptimizable(i))
                continue;

            whiteParameters[i] = std::round(whiteParameters[i] + defaultDist(gen) + linearDist(gen) * std::abs(whiteParameters[i]));
            blackParameters[i] = std::round(blackParameters[i] + defaultDist(gen) + linearDist(gen) * std::abs(blackParameters[i]));
        }
    }

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
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            white.search(params);
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

            Move bestMove = white.getBestMove();
            board.makeMove(bestMove);

            wtime -= std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            wtime = std::max(wtime, 0);
            wtime += increment;

            if(Referee::isCheckmate(board))
                return WHITE_WIN;
        } else {
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            black.search(params);
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

            Move bestMove = black.getBestMove();
            board.makeMove(bestMove);

            btime -= std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
            btime = std::max(btime, 0);
            btime += increment;

            if(Referee::isCheckmate(board))
                return BLACK_WIN;
        }

        if(Referee::isDraw(board) || board.getAge() >= DRAW_AFTER_N_MOVES)
            return DRAW;
    }
}

void Simulation::run() {
    std::cout << "Simulating " << startingPositions.size() << " games ("
              << timeControl / 1000.0 << "s + " << increment / 1000.0 << "s increment) "
              << "with " << numThreads << " threads..." << std::endl;

    std::cout << "Remaining games: " << startingPositions.size() << std::flush;

    std::mutex lock;
    std::atomic_int64_t currentIdx = startingPositions.size() - 1;
    uint64_t completedGames = 0;

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

            GameResult result = simulateSingleGame(board);
            results[index] = result;

            lock.lock();
            completedGames++;
            std::stringstream ss;
            ss << "\rRemaining games: " << std::left << std::setw(7) << startingPositions.size() - completedGames;
            std::cout << ss.str() << std::flush;
        }

        lock.unlock();
    };

    std::vector<std::thread> threads;
    for(size_t i = 0; i < numThreads; i++)
        threads.push_back(std::thread(threadFunction));

    for(size_t i = 0; i < numThreads; i++)
        threads[i].join();

    std::cout << "\rRemaining games: 0      " << std::endl;
}