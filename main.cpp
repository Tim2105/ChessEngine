#include <iostream>
#include <iomanip>
#include "Board.h"
#include "GameTreeSearch.h"
#include <chrono>
#include "EvaluationDefinitions.h"
#include "Test.h"


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
    Board board("8/P3k3/1R4n1/2p5/5BK1/2P5/2P5/8 w - - 0 1");

    GameTreeSearch search(board);

    std::vector<Move> pv;

    auto start = std::chrono::high_resolution_clock::now();
    int32_t score = search.search(12, pv);
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    std::cout << "Score: " << score << std::endl;

    std::cout << "PV:" << std::endl;

    for(Move m : pv) {
        std::cout << m.toString() << " ";
    }

    std::cout << std::endl;

    return 0;
}