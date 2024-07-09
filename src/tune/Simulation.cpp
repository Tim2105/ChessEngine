#if defined(USE_HCE) && defined(TUNE)

#include "tune/Simulation.h"

#include "core/chess/Referee.h"
#include "core/engine/search/PVSEngine.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

Simulation::Simulation(std::vector<Board>& startingPositions, uint32_t timeControl, uint32_t increment) :
    Simulation(startingPositions, timeControl, increment, 0) {
    
    // Beanspruche alle Threads bis auf 2 oder 3/4 der Threads, je nachdem was größer ist
    unsigned int hwThreads = std::thread::hardware_concurrency();
    this->numThreads = std::max(hwThreads * 3 / 4, std::max(hwThreads - 2, 1u));
}

Simulation::Simulation(std::vector<Board>& startingPositions, uint32_t timeControl, uint32_t increment, size_t numThreads):
    startingPositions(startingPositions), timeControl(timeControl), increment(increment), numThreads(std::max(numThreads, (size_t)1)) {

    results.resize(startingPositions.size());
}

GameResult Simulation::simulateSingleGame(Board& board) {
    if(Referee::isCheckmate(board))
        return board.getSideToMove() == WHITE ? BLACK_WIN : WHITE_WIN;
    else if(Referee::isDraw(board))
        return DRAW;

    HCEParameters whiteParameters = whiteParams.value_or(HCEParameters());
    HCEParameters blackParameters = blackParams.value_or(HCEParameters());

    PVSEngine white(board, whiteParameters);
    PVSEngine black(board, blackParameters);

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

            int32_t score = white.getBestMoveScore();
            if(score >= DECISIVE_SCORE)
                return WHITE_WIN;
            else if(score <= -DECISIVE_SCORE)
                return BLACK_WIN;
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

            int32_t score = black.getBestMoveScore();
            if(score >= DECISIVE_SCORE)
                return BLACK_WIN;
            else if(score <= -DECISIVE_SCORE)
                return WHITE_WIN;
        }

        if(Referee::isDraw(board))
            return DRAW;
    }
}

void Simulation::run() {
    std::cout << "Simulating " << startingPositions.size() << " games ("
              << timeControl / 1000.0 << "s + " << increment / 1000.0 << "s increment) "
              << "with " << numThreads << " threads..." << std::endl;

    std::cout << "Remaining games: " << startingPositions.size() << std::flush;

    std::mutex lock;
    int32_t currentIdx = startingPositions.size() - 1;

    auto threadFunction = [&]() {
        while(true) {
            lock.lock();
            if(currentIdx < 0) {
                lock.unlock();
                break;
            }

            int32_t index = currentIdx;
            currentIdx--;
            Board& board = startingPositions[index];
            lock.unlock();

            if(index % 10 == 9) {
                std::stringstream ss;
                ss << "\rRemaining games: " << std::left << std::setw(7) << index + 1;
                std::cout << ss.str() << std::flush;
            }

            GameResult result = simulateSingleGame(board);
            results[index] = result;
        }
    };

    std::vector<std::thread> threads;
    for(size_t i = 0; i < numThreads; i++)
        threads.push_back(std::thread(threadFunction));

    for(size_t i = 0; i < numThreads; i++)
        threads[i].join();

    std::cout << "\rRemaining games: 0      " << std::endl;
}

#endif