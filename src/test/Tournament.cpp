#include "test/Tournament.h"
#include <random>
#include <iostream>
#include <iomanip>
#include <fstream>
#include "core/utils/MoveNotations.h"

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
};

const std::vector<int32_t> Tournament::timeControls = {
    100, 200, 300, 500, 1000
};

const std::vector<int32_t> Tournament::numGames = {
    20, 15, 12, 10, 5
};

int32_t Tournament::runGame(Board& board, SearchTree& st1, SearchTree& st2, int32_t time) {
    // Übergebe beiden Engines das Spielfeld
    st1.setBoard(board);
    st2.setBoard(board);

    TournamentEvaluator evaluator(board);

    int32_t playingEngine = 1;

    while(!(evaluator.isDraw()) && board.generateLegalMoves().size() > 0) {
        // Engine 1 ist am Zug
        if(playingEngine == 1) {
            // Berechne Zug
            st1.search(time);
            // Führe Zug aus
            Move bestMove = st1.getPrincipalVariation()[0];

            if(!board.isMoveLegal(bestMove))
                throw std::runtime_error("Illegal move by " + engineName1 + "! " + bestMove.toString() + " Move " + std::to_string(board.getMoveHistory().size() + 1));

            board.makeMove(bestMove);

            // Wechsle Engine
            playingEngine = 2;
        }
        // Engine 2 ist am Zug
        else {
            // Berechne Zug
            st2.search(time);
            // Führe Zug aus
            Move bestMove = st2.getPrincipalVariation()[0];

            if(!board.isMoveLegal(bestMove))
                throw std::runtime_error("Illegal move by " + engineName2 + "! " + bestMove.toString() + " Move " + std::to_string(board.getMoveHistory().size() + 1));

            board.makeMove(bestMove);

            // Wechsle Engine
            playingEngine = 1;
        }
    }

    // Auswertung
    if(evaluator.isDraw()) {
        return 0;
    }
    else if(playingEngine == 1) {
        return 2;
    }
    else {
        return 1;
    }
}

void Tournament::run() {
    std::vector<int32_t> engine1Wins = std::vector<int32_t>(timeControls.size(), 0);
    std::vector<int32_t> engine2Wins = std::vector<int32_t>(timeControls.size(), 0);
    std::vector<int32_t> draws = std::vector<int32_t>(timeControls.size(), 0);

    int32_t timeControlIndex = 0;

    for(int32_t timeControl : timeControls) {
        
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
            std::cout << "Game " << i * 2 + 1 << " of " << numGames[timeControlIndex] * 2 << " (" << openingName << "): ";

            // Spiele Partie
            int32_t result = runGame(board, st1, st2, timeControl);


            // Speichere die Partie als PGN in Datei
            std::string filename;

            filename = "pgn/";
            filename += std::to_string(timeControl);
            filename += "ms_";
            filename += openingName;
            filename += "_";
            filename += std::to_string(i * 2 + 1);
            filename += "_";
            filename += engineName1;
            filename += "vs";
            filename += engineName2;
            filename += ".pgn";

            std::ofstream file;

            file.open(filename, std::ios::out);

            if(!file.is_open())
                throw std::runtime_error("Could not open file " + filename + " for writing! (Tournament::run())");

            file << board.pgnString();

            file.close();

            // Auswertung
            if(result == 1) {
                engine1Wins[timeControlIndex]++;
            }
            else if(result == 2) {
                engine2Wins[timeControlIndex]++;
            }
            else {
                draws[timeControlIndex]++;
            }

            // Ausgabe
            if(result == 1) {
                std::cout << engineName1 << " wins";
                if(engine1White) {
                    std::cout << " with white";
                }
                else {
                    std::cout << " with black";
                }
            }
            else if(result == 2) {
                std::cout << engineName2 << " wins";
                if(engine1White) {
                    std::cout << " with black";
                }
                else {
                    std::cout << " with white";
                }
            }
            else {
                std::cout << "Draw";
            }

            std::cout << std::endl;

            // Wechsele Seiten

            // Erstelle Spielfeld
            board = Board::fromPGN(pgn);

            bool engine2White = board.getSideToMove() == WHITE;

            // Ausgabe
            std::cout << "Game " << i * 2 + 2 << " of " << numGames[timeControlIndex] * 2 << " (" << openingName << "): ";

            // Spiele Partie
            result = runGame(board, st2, st1, timeControl);

            // Speichere die Partie als PGN in Datei
            filename = "pgn/";
            filename += std::to_string(timeControl);
            filename += "ms_";
            filename += openingName;
            filename += "_";
            filename += std::to_string(i * 2 + 2);
            filename += "_";
            filename += engineName1;
            filename += "vs";
            filename += engineName2;
            filename += ".pgn";

            file.open(filename, std::ios::out);

            file << board.pgnString();

            file.close();

            // Auswertung
            if(result == 1) {
                engine2Wins[timeControlIndex]++;
            }
            else if(result == 2) {
                engine1Wins[timeControlIndex]++;
            }
            else {
                draws[timeControlIndex]++;
            }

            // Ausgabe
            if(result == 1) {
                std::cout << engineName2 << " wins";
                if(engine2White) {
                    std::cout << " with white";
                }
                else {
                    std::cout << " with black";
                }
            }
            else if(result == 2) {
                std::cout << engineName1 << " wins";
                if(engine2White) {
                    std::cout << " with black";
                }
                else {
                    std::cout << " with white";
                }
            }
            else {
                std::cout << "Draw";
            }

            std::cout << std::endl;
        }

        timeControlIndex++;
    }

    // Ausgabe
    std::cout << std::endl << std::endl;
    std::cout << "Results:" << std::endl;
    std::cout << std::setw(15) << "Time control" << std::setw(15) << engineName1 << std::setw(15) << engineName2 << std::setw(15) << "Draws" << std::endl;
    for(size_t i = 0; i < timeControls.size(); i++) {
        std::cout << std::setw(15) << timeControls[i] << std::setw(15) << engine1Wins[i] << std::setw(15) << engine2Wins[i] << std::setw(15) << draws[i] << std::endl;
    }

    std::cout << std::endl << std::endl;
}