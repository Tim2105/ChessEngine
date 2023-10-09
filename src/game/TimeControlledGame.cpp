#include "core/chess/Referee.h"

#include "core/utils/MoveNotations.h"
#include "game/TimeControlledGame.h"
#include "game/ui/console/BoardVisualizer.h"

#include <chrono>
#include <iostream>

void TimeControlledGame::start() {
    while (!Referee::isGameOver(board)) {
        outputGameState();

        Move move;

        auto start = std::chrono::high_resolution_clock::now();

        if (board.getSideToMove() == WHITE) {
            if (whiteTimeControlled)
                move = whitePlayer.getMove(whiteTime);
            else
                move = whitePlayer.getMove();
        } else {
            if (blackTimeControlled)
                move = blackPlayer.getMove(blackTime);
            else
                move = blackPlayer.getMove();
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        if (board.getSideToMove() == WHITE) {
            if (whiteTimeControlled) {
                if(whiteTime < duration) {
                    whiteTime = 0;
                    break;
                }

                whiteTime -= duration;
            }
        } else {
            if (blackTimeControlled) {
                if(blackTime < duration) {
                    blackTime = 0;
                    break;
                }

                blackTime -= duration;
            }
        }

        if(!board.isMoveLegal(move))
            throw std::runtime_error("Illegal move!");

        board.makeMove(move);
    }

    outputGameState();

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
        if(board.getSideToMove() == WHITE &&
          (whiteTimeControlled && whiteTime == 0)) {
            result = GameResult::BLACK_WON_BY_TIME;
            whitePlayer.onGameEnd(GameResult::BLACK_WON_BY_TIME);
            blackPlayer.onGameEnd(GameResult::BLACK_WON_BY_TIME);
          }
        else if(board.getSideToMove() == BLACK &&
          (blackTimeControlled && blackTime == 0)) {
            result = GameResult::WHITE_WON_BY_TIME;
            whitePlayer.onGameEnd(GameResult::WHITE_WON_BY_TIME);
            blackPlayer.onGameEnd(GameResult::WHITE_WON_BY_TIME);
          }
        else {
            result = GameResult::DRAW;
            whitePlayer.onGameEnd(GameResult::DRAW);
            blackPlayer.onGameEnd(GameResult::DRAW);
        }
    }
}

std::string millisecondsToTimeString(uint32_t timeInMs) {
    uint32_t milliseconds = timeInMs % 1000;
    uint32_t seconds = timeInMs / 1000;
    uint32_t minutes = seconds / 60;

    seconds %= 60;

    std::string timeString = std::to_string(minutes) + ":";
    if (seconds < 10)
        timeString += "0";

    timeString += std::to_string(seconds);

    if(timeInMs < 10000) {
        timeString += ".";

        if (milliseconds < 100)
            timeString += "0";

        if (milliseconds < 10)
            timeString += "0";

        timeString += std::to_string(milliseconds);
    }

    return timeString;
}

std::string TimeControlledGame::getWhiteAdditionalInfo() const {
    std::string additionalInfo = "White - ";

    if (whiteTimeControlled)
        additionalInfo += millisecondsToTimeString(whiteTime);
    else
        additionalInfo += "∞";

    return additionalInfo;
}

std::string TimeControlledGame::getBlackAdditionalInfo() const {
    std::string additionalInfo = "Black - ";

    if (blackTimeControlled)
        additionalInfo += millisecondsToTimeString(blackTime);
    else
        additionalInfo += "∞";

    return additionalInfo;
}
