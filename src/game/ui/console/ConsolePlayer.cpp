#include "core/utils/MoveNotations.h"
#include "game/ui/console/ConsolePlayer.h"

#include <iostream>
#include <string>

Move ConsolePlayer::getMove() {
    while(true) {
        std::string move;
        std::cout << "Enter move: " << std::endl;
        std::cin >> move;

        for(Move m : board.generateLegalMoves()) {
            if(move == m.toString() || move == toStandardAlgebraicNotation(m, board)) {
                return m;
            }
        }

        std::cout << "Invalid move" << std::endl;
    }
}

Move ConsolePlayer::getMove(uint32_t remainingTime) {
    while(true) {
        std::string move;
        std::cout << "Enter move (" << remainingTime << "ms remaining):" << std::endl;
        std::cin >> move;

        for(Move m : board.generateLegalMoves()) {
            if(move == m.toString() || move == toStandardAlgebraicNotation(m, board)) {
                return m;
            }
        }

        std::cout << "Invalid move" << std::endl;
    }
}