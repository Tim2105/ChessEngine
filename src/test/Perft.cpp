#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

#include "test/Perft.h"

#include "core/utils/Array.h"
#include "core/utils/MoveNotations.h"

#include "uci/Options.h"

void perftImpl(Board& board, int depth, uint64_t& count) {
    if(depth <= 1) {
        count += board.generateLegalMoves().size();
        return;
    }
    
    Array<Move, 256> moves;
    board.generateLegalMoves(moves);
    for(Move m : moves) {
        board.makeMove(m);
        perftImpl(board, depth - 1, count);
        board.undoMove();
    }
}

uint64_t perft(Board& board, int depth) {
    uint64_t count = 0;
    perftImpl(board, depth, count);
    return count;
}

void printPerftResults(Board& board, int depth) {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    std::cout << "Perft results for "
        << board.toFEN() << " at depth " << depth << "\n";

    if(depth <= 1) {
        Array<Move, 256> moves = board.generateLegalMoves();
        for(Move m : moves)
            std::cout << std::setw(5) << m.toString() << ": 1" << "\n";

        std::cout << "Total: " << moves.size() << "\n";

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms\n" << std::endl;
        return;
    }

    uint64_t accumulatedNodes = 0;

    Array<Move, 256> moves;
    board.generateLegalMoves(moves);

    #if defined(DISABLE_THREADS)
        for(Move m : moves) {
            board.makeMove(m);
            uint64_t nodes = perft(board, depth - 1);
            board.undoMove();

            accumulatedNodes += nodes;
            std::cout << std::setw(5) << m.toString() << ": " << nodes << std::endl;
        }
    #else

        size_t numThreads = UCI::options["Threads"].getValue<size_t>();

        std::vector<std::thread> threads;
        std::vector<Board> boards;
        std::vector<uint64_t> nodes;

        for(size_t i = 0; i < numThreads; i++) {
            boards.push_back(board);
            nodes.push_back(0);
        }

        std::mutex coutMutex, moveArrayMutex;

        for(size_t i = 0; i < numThreads; i++) {
            threads.push_back(std::thread([&](size_t threadIndex) {
                moveArrayMutex.lock();
                while(moves.size() > 0) {
                    Move m = moves.pop_back();
                    moveArrayMutex.unlock();

                    boards[threadIndex].makeMove(m);
                    uint64_t count = perft(boards[threadIndex], depth - 1);
                    boards[threadIndex].undoMove();

                    nodes[threadIndex] += count;

                    coutMutex.lock();
                    std::cout << std::setw(5) << m.toString() << ": " << count << std::endl;
                    coutMutex.unlock();

                    moveArrayMutex.lock();
                }

                moveArrayMutex.unlock();
            }, i));
        }

        for(size_t i = 0; i < threads.size(); i++) {
            threads[i].join();
            accumulatedNodes += nodes[i];
        }

    #endif

    std::cout << "\n";
    std::cout << "Total: " << accumulatedNodes << "\n";
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << " Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << "\n";
    std::cout << "  N/s: " << accumulatedNodes / std::max((uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count(),
                            (uint64_t)1) * 1000 << std::endl;
}