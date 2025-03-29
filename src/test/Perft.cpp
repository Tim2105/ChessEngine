#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

#include "test/Perft.h"

#include <atomic>
#include <cmath>
#include <mutex>
#include <tuple>
#include <vector>

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

        if(numThreads == 1) {
            // sequenzielle Ausführung
            for(Move m : moves) {
                board.makeMove(m);
                uint64_t nodes = perft(board, depth - 1);
                board.undoMove();

                accumulatedNodes += nodes;
                std::cout << std::setw(5) << m.toString() << ": " << nodes << std::endl;
            }
        } else {
            std::vector<std::thread> threads;
            std::vector<std::tuple<Move, std::vector<Board>>> work(moves.size());
            std::vector<std::atomic_size_t> workProgress(moves.size());
            std::vector<std::atomic_uint64_t> nodes(moves.size());

            size_t moveIndex = 0;
            size_t boardIndex = 0;
            std::mutex workMutex, coutMutex;

            // Annahme: Verzweigungsfaktor = moves.size()
            // Erhöhe die Entpacktiefe solange bis die Anzahl der Bretter
            // mindestens doppelt so groß wie die Anzahl der Threads ist
            size_t unpackingDepth = 1;
            size_t numBoardsPerMove = std::max(moves.size(), size_t(10)); // Mindestens 10
            if(board.isCheck()) // Im Schach ist der Verzweigungsfaktor wahrscheinlich höher
                numBoardsPerMove = 8 + board.getPieceBitboard().popcount() * 3;
            
            while(numBoardsPerMove < numThreads * 2) {
                unpackingDepth++;
                numBoardsPerMove *= numBoardsPerMove;
            }

            // Die Entpacktiefe darf nicht größer als die Suchtiefe sein
            unpackingDepth = std::min(unpackingDepth, (size_t)depth - 1);

            const std::function<void(Board&, size_t, std::vector<Board>&)> unpackBoards =
                [&unpackBoards](Board& root, size_t depth, std::vector<Board>& boards) {
                if(depth == 0) {
                    boards.push_back(root);
                    return;
                }

                Array<Move, 256> moves;
                root.generateLegalMoves(moves);

                for(Move m : moves) {
                    root.makeMove(m);
                    unpackBoards(root, depth - 1, boards);
                    root.undoMove();
                }
            };

            for(Move m : moves) {
                board.makeMove(m);

                std::get<0>(work[moveIndex]) = m;
                std::get<1>(work[moveIndex]).reserve(numBoardsPerMove + numBoardsPerMove / 2);
                unpackBoards(board, unpackingDepth, std::get<1>(work[moveIndex]));

                board.undoMove();
                moveIndex++;
            }

            moveIndex = 0;

            for(size_t i = 0; i < numThreads; i++) {
                threads.push_back(std::thread([&]() {
                    do {
                        workMutex.lock();
                        size_t localMoveIndex = moveIndex;
                        if(localMoveIndex >= work.size()) {
                            workMutex.unlock();
                            return;
                        }

                        size_t localBoardIndex = boardIndex++;
                        if(boardIndex >= std::get<1>(work[localMoveIndex]).size()) {
                            moveIndex += 1;
                            boardIndex = 0;
                        }
                        workMutex.unlock();

                        Board& board = std::get<1>(work[localMoveIndex])[localBoardIndex];

                        uint64_t count = perft(board, depth - unpackingDepth - 1);
                        nodes[localMoveIndex].fetch_add(count);
                        size_t progress = workProgress[localMoveIndex].fetch_add(1);

                        // Live output of finished nodes at 1 ply
                        if(progress + 1 >= std::get<1>(work[localMoveIndex]).size()) {
                            coutMutex.lock();
                            std::cout << std::setw(5) << std::get<0>(work[localMoveIndex]).toString() << ": " << nodes[localMoveIndex].load() << std::endl;
                            coutMutex.unlock();
                        }
                    } while(true);
                }));
            }

            for(size_t i = 0; i < threads.size(); i++) {
                threads[i].join();
            }

            for(size_t i = 0; i < moves.size(); i++) {
                accumulatedNodes += nodes[i].load();
            }
            std::cout << "\n";
        }

    #endif

    std::cout << "\n";
    std::cout << "Total: " << accumulatedNodes << "\n";
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << " Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << "\n";
    std::cout << " kN/s: " << accumulatedNodes / std::max((uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count(),
                            (uint64_t)1) << std::endl;
}