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

        std::vector<std::string> movesInSimpleSAN;
        Array<Move, 256> legalMoves = board.generateLegalMoves();

        for(Move m : legalMoves) {

            std::string san = toStandardAlgebraicNotation(m, board);
            std::string simpleSan = san;

            if(simpleSan.back() == '+' || simpleSan.back() == '#')
                simpleSan.pop_back();

            if(simpleSan.find('x') == std::string::npos)
                movesInSimpleSAN.push_back(simpleSan);
            else {
                size_t xIndex = simpleSan.find('x');
                movesInSimpleSAN.push_back(simpleSan.substr(0, xIndex) + simpleSan.substr(xIndex + 1));
            }

            if(move == san || move == m.toString() || move == movesInSimpleSAN.back())
                return m;
        }

        std::cout << "Invalid move" << std::endl << std::endl;
    }
}

void ConsolePlayer::onGameEnd(GameResult result) {
    std::cout << "Game over! ";

    switch(result) {
        case GameResult::WHITE_WON:
            std::cout << "White won!" << std::endl;
            break;
        case GameResult::BLACK_WON:
            std::cout << "Black won!" << std::endl;
            break;
        case GameResult::WHITE_WON_BY_TIME:
            std::cout << "White won by time!" << std::endl;
            break;
        case GameResult::BLACK_WON_BY_TIME:
            std::cout << "Black won by time!" << std::endl;
            break;
        case GameResult::DRAW:
            std::cout << "Draw!" << std::endl;
            break;
        default:
            break;
    }

    std::cout << std::endl << "Press any key to continue..." << std::endl;
    ngetch();
}