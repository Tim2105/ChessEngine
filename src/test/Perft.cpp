#include <chrono>
#include <iomanip>
#include <iostream>

#include "Perft.h"
#include "core/utils/Array.h"
#include "core/utils/MoveNotations.h"

void perftImpl(Board& board, int depth, uint64_t& count) {
    if(depth <= 1) {
        count += board.generateLegalMoves().size();
        return;
    }

    for(Move m : board.generateLegalMoves()) {
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

    if(depth <= 1) {
        Array<Move, 256> moves = board.generateLegalMoves();
        for(Move m : moves)
            std::cout << std::setw(7) << toStandardAlgebraicNotation(m, board) << ": 1" << std::endl;

        std::cout << "Total: " << moves.size() << std::endl;

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;
        return;
    }

    uint64_t accumulatedNodes = 0;

    for(Move m : board.generateLegalMoves()) {
        board.makeMove(m);
        uint64_t nodes = perft(board, depth - 1);
        board.undoMove();

        accumulatedNodes += nodes;
        std::cout << std::setw(7) << toStandardAlgebraicNotation(m, board) << ": " << nodes << std::endl;
    }

    std::cout << "  Total: " << accumulatedNodes << std::endl;
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "   Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;
}