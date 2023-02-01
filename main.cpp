#include <iostream>
#include "Board.h"
#include <chrono>
#include "SearchTree.h"


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
    SearchTree st(board);

    // while(board.generateLegalMoves().size() != 0) {
    //     int16_t score = st.search(5000);
    //     Move m = st.getPrincipalVariation()[0];
    //     std::cout << "Playing " << m << " Eval " << score << std::endl;
    //     board.makeMove(m);
    //     if(board.generateLegalMoves().size() == 0)
    //         break;
        
    //     board.makeMove(getUserMove(board));
    // }

    st.search(2500);

    return 0;
}