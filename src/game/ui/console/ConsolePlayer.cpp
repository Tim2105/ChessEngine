#include "core/utils/MoveNotations.h"
#include "game/ui/console/ConsolePlayer.h"

#include <iostream>
#include <string>

#define UNUSED(x) (void)(x)

Move ConsolePlayer::getMove() {
    return getMove(0);
}

Move ConsolePlayer::getMove(uint32_t remainingTime) {
    UNUSED(remainingTime);

    while(true) {
        std::string move;
        std::cout << "Enter move: ";
        std::cin >> move;

        for(Move m : board.generateLegalMoves()) {
            if(move == m.toString() || move == toStandardAlgebraicNotation(m, board)) {
                return m;
            }
        }

        std::cout << "Invalid move" << std::endl << std::endl;
    }
}