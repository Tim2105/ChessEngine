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

        if(!board.isMoveLegal(move)) {
            std::cout << "Illegal move!" << std::endl;
            continue;
        }

        board.makeMove(move);

        outputGameState();
    }

    saveGameToFile();

    if (Referee::isCheckmate(board)) {
        if (board.getSideToMove() == WHITE) {
            result = GameResult::BLACK_WON;
            whitePlayer.onGameEnd(GameResult::BLACK_WON);
            blackPlayer.onGameEnd(GameResult::BLACK_WON);
        } else {
            result = GameResult::WHITE_WON;
            whitePlayer.onGameEnd(GameResult::WHITE_WON);
            blackPlayer.onGameEnd(GameResult::WHITE_WON);
        }
    } else {
        result = GameResult::DRAW;
        whitePlayer.onGameEnd(GameResult::DRAW);
        blackPlayer.onGameEnd(GameResult::DRAW);
    }
}