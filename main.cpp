#include <iostream>
#include "Board.h"
#include <chrono>
#include "SearchTree.h"
#include <iomanip>


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

Move getUserMove(Board& board) {
    while(true) {
        std::string move;
        std::cout << "Enter move: ";
        std::cin >> move;

        for(Move m : board.generateLegalMoves()) {
            if(move == m.toString()) {
                return m;
            }
        }

        std::cout << "Invalid move" << std::endl;
    }
}

int main() {

    Board board;
    // SearchTree st(board);

    // while(board.generateLegalMoves().size() != 0) {
    //     board.makeMove(getUserMove(board));

    //     if(board.generateLegalMoves().size() == 0)
    //         break;

    //     int16_t score = st.search(100);
    //     Move m = st.getPrincipalVariation()[0];
    //     std::cout << "Playing " << m << " Eval " << score << std::endl;
    //     board.makeMove(m);
    // }

    // //st.search(3000);

    for(int i = 1; i <= 6; i++) {
        int count = 0;
        auto start = std::chrono::high_resolution_clock::now();
        perft(board, i, count);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Depth " << i << " Nodes " << count << " Time " << duration.count() << "ms "
                << "N/s " << std::fixed << std::setprecision(0) << count / (duration.count() / 1000.0) << std::endl;
    }

    return 0;
}