#include <iostream>
#include <iomanip>
#include "Board.h"
#include "GameTreeSearch.h"
#include <chrono>
#include "EvaluationDefinitions.h"


// perft(4) dauert knapp 0.37s
void perft(Board& board, int depth, int& count) {
    if(depth == 1) {
        count += board.generateLegalMoves().size();
        return;
    }

    for(Move m : board.generateLegalMoves()) {
        board.makeMove(m);
        perft(board, depth - 1, count);
        board.undoMove();
    }
}

int main() {
    
    Board board;

    // for(int i = 1; i < 7; i++) {
    //     int count = 0;
    //     auto start = std::chrono::high_resolution_clock::now();
    //     perft(board, i, count);
    //     auto end = std::chrono::high_resolution_clock::now();

    //     double time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    //     std::cout << "Perft(Depth: " << i << "): " << std::endl;
    //     std::cout << "Nodes: " << count << std::endl;
    //     std::cout << "Time: " << std::setprecision(0) << std::fixed << time << "ms" << std::endl;
    //     std::cout << "Nodes/s: " << std::setprecision(0) << std::fixed << count / (time / 1000) << std::endl << std::endl;
    // }

    GameTreeSearch search(board);

    auto start = std::chrono::high_resolution_clock::now();
    Array pv = search.search(5);
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    std::cout << "PV:" << std::endl;

    for(Move m : pv) {
        std::cout << m.toString() << std::endl;
    }


    // Board boardOtherSide = board;
    // boardOtherSide.side ^= COLOR_MASK;

    // BoardEvaluator evaluator;

    // std::cout << "Evaluation for moving color: " << evaluator.evaluate(board) << std::endl;
    // std::cout << "Evaluation for other color: " << evaluator.evaluate(boardOtherSide) << std::endl;
    

    return 0;
}