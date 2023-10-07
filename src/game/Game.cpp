#include "core/chess/Referee.h"
#include "core/utils/MoveNotations.h"
#include "game/Game.h"

#include <filesystem>
#include <fstream>

void Game::outputGameState() {
    gameStateOutput.outputGameState(getWhiteAdditionalInfo(), getBlackAdditionalInfo());
}

void Game::saveGameToFile() {
    std::filesystem::create_directory("pgn");
    std::ofstream pgnFile("pgn/recentGame.pgn");
    pgnFile << board.pgnString();
    pgnFile.close();
}

void Game::start() {
    outputGameState();

    while (!Referee::isGameOver(board)) {
        Move move;

        if (board.getSideToMove() == WHITE) 
            move = whitePlayer.getMove();
        else
            move = blackPlayer.getMove();

        board.makeMove(move);

        outputGameState();
    }

    saveGameToFile();

    if (Referee::isCheckmate(board)) {
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