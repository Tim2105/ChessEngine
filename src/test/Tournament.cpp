#include "core/utils/MoveNotations.h"
#include "game/TimeControlledGame.h"
#include "game/ComputerPlayer.h"

#include "test/Tournament.h"

#include <random>
#include <iostream>
#include <iomanip>
#include <fstream>

const std::vector<std::string> Tournament::openings = {
    "1.e4 e5 2.Nf3 Nc6 3.Bb5", "Ruy Lopez Opening",
    "1.e4 e6", "French Defense",
    "1.e4 c5", "Sicilian Defense",
    "1.e4 c6", "Caro-Kann Defense",
    "1.e4 e5 2.Nf3 Nc6 3.Bc4", "Italian Game",
    "1.e4 d5", "Scandinavian Defense",
    "1.e4 d6 2.d4 Nf6", "Pirc Defense",
    "1.d4 d5 2.c4", "Queen's Gambit",
    "1.d4 Nf6 2.c4 g6", "King's Indian Defense",
    "1.d4 Nf6 2.c4 e6 3.Nc3 Bb4", "Nimzo-Indian Defense",
    "1.d4 Nf6 2.c4 g6 3.Nc3 d5", "Gruenfeld Defense",
    "1.d4 d5 2.Nf3 Nf6 3.Bf4", "London System",
    "1.c4", "English Opening",
    "1.Nf3 d5 2.g3", "King's Indian Attack",
    "1.e4 e5 2.Nc3", "Vienna Game",
    "1.d4 d5 2.c4 c6", "Slav Defense",
    "1.d4 Nf6 2.c4 e6 3.g3", "Catalan Opening",
    "1.d4 Nf6 2.c4 c5 3.d5 e6 4.Nc3 exd5 5.cxd5 d6", "Benoni Defense; Modern Variation",
    "1.e4 e5 2.Nf3 Nc6 3.d4", "Scotch Game",
    "1.d4 f5", "Dutch Defense",
    "1.d4 Nf6 2.Bg5", "Trompowsky Attack",
    "1.d4 Nf6 2.c4 e6 3.Nf3 b6", "Queen's Indian Defense",
    "1.Nf3", "Reti Opening",
    "1.e4 Nf6", "Alekhine's Defense"
};

const std::vector<int32_t> Tournament::timeControls = {
    1, 30
};

const std::vector<int32_t> Tournament::numGames = {
    200, 60
};

enum class TournamentGameResult {
    DRAW = 0,
    ENGINE1_WON,
    ENGINE1_WON_ON_TIME,
    ENGINE2_WON,
    ENGINE2_WON_ON_TIME
};

GameResult Tournament::runGame(Board& board, Engine& st1, Engine& st2, int32_t time) {
    st1.setBoard(board);
    st2.setBoard(board);
    ComputerPlayer p1(st1);
    ComputerPlayer p2(st2);
    TournamentGameStateOutput output(board);

    TimeControlledGame game(board, p1, p2, output, time, time);
    game.start();

    return game.getResult();
}

void writePGN(std::string filename, std::string pgn) {
    std::ofstream file;

    file.open(filename, std::ios::out);

    if(!file.is_open())
        throw std::runtime_error("Could not open file " + filename + " for writing! (Tournament::run())");

    file << pgn;

    file.close();
}

void Tournament::run() {
    std::vector<std::vector<TournamentGameResult>> results;

    int32_t timeControlIndex = 0;

    for(int32_t timeControl : timeControls) {
        results.push_back(std::vector<TournamentGameResult>());
        
        for(int32_t i = 0; i < numGames[timeControlIndex]; i++) {
            // Wähle zufällige Eröffnung
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, openings.size() / 2 - 1);

            int32_t openingIndex = dis(gen);
            std::string pgn = openings[openingIndex * 2];
            std::string openingName = openings[openingIndex * 2 + 1];

            // Erstelle Spielfeld
            Board board = Board::fromPGN(pgn);

            bool engine1White = board.getSideToMove() == WHITE;

            // Ausgabe
            std::cout << "(" << timeControl << "s) Playing Game " << i + 1 << " of " << numGames[timeControlIndex] << " (" << openingName << ")... ";
            std::cout << std::flush;

            // Spiele Partie
            
            // Münzwurf, um die Reihenfolge zu bestimmen
            std::uniform_int_distribution<> dis2(0, 1);
            bool engine1Starts = dis2(gen);

            Engine& p1 = engine1Starts ? st1 : st2;
            Engine& p2 = engine1Starts ? st2 : st1;

            GameResult result = runGame(board, p1, p2, timeControl * 1000);
            TournamentGameResult tournamentResult = TournamentGameResult::DRAW;

            switch(result) {
                case GameResult::DRAW:
                    tournamentResult = TournamentGameResult::DRAW;
                    break;
                case GameResult::WHITE_WON:
                    if(engine1White)
                        tournamentResult = TournamentGameResult::ENGINE1_WON;
                    else
                        tournamentResult = TournamentGameResult::ENGINE2_WON;
                    break;
                case GameResult::BLACK_WON:
                    if(engine1White)
                        tournamentResult = TournamentGameResult::ENGINE2_WON;
                    else
                        tournamentResult = TournamentGameResult::ENGINE1_WON;
                    break;
                case GameResult::WHITE_WON_BY_TIME:
                    if(engine1White)
                        tournamentResult = TournamentGameResult::ENGINE1_WON_ON_TIME;
                    else
                        tournamentResult = TournamentGameResult::ENGINE2_WON_ON_TIME;
                    break;
                case GameResult::BLACK_WON_BY_TIME:
                    if(engine1White)
                        tournamentResult = TournamentGameResult::ENGINE2_WON_ON_TIME;
                    else
                        tournamentResult = TournamentGameResult::ENGINE1_WON_ON_TIME;
                    break;
                default:
                    break;
            }

            results[timeControlIndex].push_back(tournamentResult);

            // Speichere die Partie als PGN in Datei
            std::string filename;

            filename = "pgn/";
            filename += std::to_string(timeControl);
            filename += "s_";
            filename += openingName;
            filename += "_";
            filename += std::to_string(i * 2 + 1);
            filename += "_";
            filename += engineName1;
            filename += "vs";
            filename += engineName2;
            filename += ".pgn";

            writePGN(filename, board.pgnString());

            std::string resultString;

            switch(tournamentResult) {
                case TournamentGameResult::DRAW:
                    resultString = "Draw";
                    break;
                case TournamentGameResult::ENGINE1_WON:
                    resultString = engineName1 + " won";
                    break;
                case TournamentGameResult::ENGINE1_WON_ON_TIME:
                    resultString = engineName1 + " won on time";
                    break;
                case TournamentGameResult::ENGINE2_WON:
                    resultString = engineName2 + " won";
                    break;
                case TournamentGameResult::ENGINE2_WON_ON_TIME:
                    resultString = engineName2 + " won on time";
                    break;
                default:
                    break;
            }

            // Ausgabe
            std::cout << " completed (" << resultString << ")" << std::endl;
        }

        timeControlIndex++;
    }

    // Ausgabe
    std::cout << std::endl << std::endl;
    std::cout << "Results:" << std::endl;
    std::cout << std::setw(15) << "Time control" << std::setw(30) << engineName1 + " (by time)"
            << std::setw(30) << engineName2 + " (by time)" << std::setw(15) << "Draws" << std::endl;

    for(size_t i = 0; i < timeControls.size(); i++) {
        std::vector<TournamentGameResult> timeControlResults = results[i];
        int32_t engine1Wins = 0;
        int32_t engine2Wins = 0;
        int32_t draws = 0;
        int32_t engine1Timeouts = 0;
        int32_t engine2Timeouts = 0;

        for(TournamentGameResult result : timeControlResults) {
            switch(result) {
                case TournamentGameResult::DRAW:
                    draws++;
                    break;
                case TournamentGameResult::ENGINE1_WON_ON_TIME:
                    engine2Timeouts++;
                    engine1Wins++;
                    break;
                case TournamentGameResult::ENGINE1_WON:
                    engine1Wins++;
                    break;
                case TournamentGameResult::ENGINE2_WON_ON_TIME:
                    engine1Timeouts++;
                    engine2Wins++;
                    break;
                case TournamentGameResult::ENGINE2_WON:
                    engine2Wins++;
                    break;
            }
        }

        std::cout << std::setw(15) << timeControls[i] << std::setw(30) << std::to_string(engine1Wins) + " (" + std::to_string(engine2Timeouts) + ")"
                << std::setw(30) << std::to_string(engine2Wins) + " (" + std::to_string(engine1Timeouts) + ")" << std::setw(15) << draws << std::endl;
    }

    std::cout << std::endl << std::endl;
}