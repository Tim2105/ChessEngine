#include "core/utils/MoveNotations.h"
#include "game/Referee.h"
#include "game/TimeControlledGame.h"

#include <chrono>
#include <iostream>

void TimeControlledGame::start() {
    while (!isGameOver(board)) {
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
                    std::cout << "White ran out of time" << std::endl;
                    break;
                }
                whiteTime -= duration;
            }
        } else {
            if (blackTimeControlled) {
                if(blackTime < duration) {
                    std::cout << "Black ran out of time" << std::endl;
                    break;
                }
                blackTime -= duration;
            }
        }

        bool isTimeControlled = board.getSideToMove() == WHITE ? whiteTimeControlled : blackTimeControlled;

        std::cout << "Move played: " << toFigurineAlgebraicNotation(move, board) << std::endl;

        if(isTimeControlled) {
            uint32_t remainingTime = board.getSideToMove() == WHITE ? whiteTime : blackTime;
            std::cout << "Remaining time: " << remainingTime << "ms" << std::endl;
        }

        std::cout << std::endl << std::endl;

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

