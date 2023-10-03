#include "core/utils/MoveNotations.h"
#include "game/Game.h"
#include "game/Referee.h"

void Game::outputGameState() {
    gameStateOutput.outputGameState(getWhiteAdditionalInfo(), getBlackAdditionalInfo());
}

void Game::start() {
    outputGameState();

    while (!isGameOver(board)) {
        Move move;

        if (board.getSideToMove() == WHITE) 
            move = whitePlayer.getMove();
        else
            move = blackPlayer.getMove();

        board.makeMove(move);

        outputGameState();
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