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

    if (isCheckmate(board)) {
        if (board.getSideToMove() == WHITE) {
            whitePlayer.onGameEnd(BLACK_WON);
            blackPlayer.onGameEnd(BLACK_WON);
        } else {
            whitePlayer.onGameEnd(WHITE_WON);
            blackPlayer.onGameEnd(WHITE_WON);
        }
    } else {
        whitePlayer.onGameEnd(DRAW);
        blackPlayer.onGameEnd(DRAW);
    }
}