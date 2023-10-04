#include "game/ui/console/ConsoleGameStateOutput.h"
#include "game/ui/console/ConsoleNavigator.h"
#include "game/ui/console/ConsolePGNGameAnalyzer.h"
#include "game/ui/console/ConsolePlayer.h"
#include "game/ui/console/PortabilityHelper.h"

#include "core/chess/Board.h"
#include "core/engine/StaticEvaluator.h"
#include "core/engine/MinimaxEngine.h"

#include "game/ComputerPlayer.h"
#include "game/TimeControlledGame.h"

#include <fstream>

void ConsoleNavigator::startGame() {
    clearScreen();

    std::cout <<  "Play as white? (y/n)" << std::endl;
    int input = ngetch();

    bool isWhite = input == 'y';

    std::cout << "Time Control for white (in seconds, 0 = no time control):" << std::endl;
    int whiteTime;
    std::cin >> whiteTime;

    std::cout << "Time Control for black (in seconds, 0 = no time control):" << std::endl;
    int blackTime;
    std::cin >> blackTime;

    Board board;
    StaticEvaluator evaluator(board);
    MinimaxEngine engine(evaluator);

    ComputerPlayer computerPlayer(engine);
    ConsolePlayer consolePlayer(board);

    ConsoleGameStateOutput output(board, !isWhite);

    if(isWhite) {
        TimeControlledGame game(board, consolePlayer, computerPlayer, output, whiteTime * 1000, blackTime * 1000);
        playGame(game);
    } else {
        TimeControlledGame game(board, computerPlayer, consolePlayer, output, whiteTime * 1000, blackTime * 1000);
        playGame(game);
    }
}

void ConsoleNavigator::startAnalysis() {
    clearScreen();

    std::cout << "Please enter the path to the PGN file(or r for the most recently saved game):" << std::endl;
    std::string path;
    std::cin >> path;

    if(path == "r")
        path = "pgn/recentGame.pgn";

    std::ifstream pgnFile(path);
    if(!pgnFile.good()) {
        std::cout << "File not found!" << std::endl << std::endl;
        return;
    }

    std::cout << "How long should the engine analyse? (in seconds)" << std::endl;
    int32_t time;
    std::cin >> time;

    try {
        std::string pgn((std::istreambuf_iterator<char>(pgnFile)), std::istreambuf_iterator<char>());

        Board board;
        StaticEvaluator evaluator(board);
        MinimaxEngine engine(evaluator, 5);

        ConsolePGNGameAnalyzer analyzer(pgn, engine, time * 1000);
        analyzeGame(analyzer);
    } catch(std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

void ConsoleNavigator::navigate() {
    while(true) {
        clearScreen();

        std::cout << "1. Play a game" << std::endl;
        std::cout << "2. Analyze a game" << std::endl;
        std::cout << "3. Exit" << std::endl;

        std::cout << std::string(2, '\n');

        int input = ngetch();

        if(input == '1') {
            startGame();
        } else if(input == '2') {
            startAnalysis();
        } else if(input == '3' || input == 27 || input == 'q') {
            break;
        }
    }
}