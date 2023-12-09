#include "game/ui/Navigator.h"

#include <fstream>
#include <string>

void Navigator::playGame(Game& game) {
    game.start();

    std::string pgn = game.getBoard().toPgn();

    std::ofstream pgnFile("pgn/recentGame.pgn");
    pgnFile << pgn;

    pgnFile.close();
}

void Navigator::analyzeGame(PGNGameAnalyzer& analyzer) {
    analyzer.output();
}