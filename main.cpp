#include <iostream>
#include "Board.h"
#include <chrono>
#include "SearchTree.h"
#include <iomanip>
#include "EvaluationDefinitions.h"
#include "MoveNotations.h"


void perft(Board& board, int depth, int& count) {
    if(depth <= 1) {
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
    BoardEvaluator evaluator(board);

    while(board.generateLegalMoves().size() != 0 && !evaluator.isDraw()) {
        int16_t score = st.search(6000);
        Move m = st.getPrincipalVariation()[0];
        std::cout << "Playing " << toFigurineAlgebraicNotation(m, board) << " Eval " << score << std::endl;
        board.makeMove(m);
        

        if(board.generateLegalMoves().size() == 0)
            break;

        board.makeMove(getUserMove(board));
    }

    // st.search(50000);

    return 0;
}