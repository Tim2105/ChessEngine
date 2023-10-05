#include "core/utils/MoveNotations.h"
#include "game/ui/console/ConsolePlayer.h"
#include "game/ui/console/PortabilityHelper.h"

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

void ConsolePlayer::onGameEnd(uint8_t result) {
    std::cout << "Game over! ";

    switch(result) {
        case WHITE_WON:
            std::cout << "White won!" << std::endl;
            break;
        case BLACK_WON:
            std::cout << "Black won!" << std::endl;
            break;
        case WHITE_WON_BY_TIME:
            std::cout << "White won by time!" << std::endl;
            break;
        case BLACK_WON_BY_TIME:
            std::cout << "Black won by time!" << std::endl;
            break;
        case DRAW:
            std::cout << "Draw!" << std::endl;
            break;
    }

    std::cout << std::endl << "Press any key to continue..." << std::endl;
    ngetch();
}