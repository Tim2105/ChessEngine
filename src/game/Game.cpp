#include "core/utils/MoveNotations.h"
#include "game/Game.h"
#include "game/Referee.h"

void Game::start() {
    while (!isGameOver(board)) {
        Move move;

        if (board.getSideToMove() == WHITE) 
            move = whitePlayer.getMove();
        else
            move = blackPlayer.getMove();

        std::cout << "Move played: " << toFigurineAlgebraicNotation(move, board) << std::endl;

        board.makeMove(move);
    }

    std::cout << "Game over" << std::endl;
    if (isCheckmate(board)) {
        if (board.getSideToMove() == WHITE)
            std::cout << "Black wins" << std::endl;
        else
            std::cout << "White wins" << std::endl;
    } else
        std::cout << "Draw" << std::endl;
}