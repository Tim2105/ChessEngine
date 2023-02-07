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

    Board board("r1bqkb1r/ppppnppp/2n5/4P3/8/1Q3N2/PPP2PPP/RNB1KB1R w KQkq - 0 1");
    SearchTree st(board);
    BoardEvaluator evaluator(board);

    // while(board.generateLegalMoves().size() != 0 && !evaluator.isDraw()) {
    //     int16_t score = st.search(3000);
    //     Move m = st.getPrincipalVariation()[0];
    //     std::cout << "Playing " << m << " Eval " << score << std::endl;
    //     board.makeMove(m);
        
    //     if(board.generateLegalMoves().size() == 0)
    //         break;

    //     board.makeMove(getUserMove(board));
    // }

    st.search(20000);

    return 0;
}