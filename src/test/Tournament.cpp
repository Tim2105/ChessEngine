#include "test/Tournament.h"
#include <random>
#include <iostream>
#include <iomanip>
#include <fstream>

const std::vector<std::string> Tournament::openings = {
    "r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 0 1", "Ruy Lopez Opening",
    "rnbqkbnr/pppp1ppp/4p3/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1", "French Defense",
    "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 1", "Sicilian Defense",
    "rnbqkbnr/pp1ppppp/2p5/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1", "Caro-Kann Defense",
    "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 0 1", "Italian Game",
    "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 1", "Scandinavian Defense",
    "rnbqkb1r/ppp1pppp/3p1n2/8/3PP3/8/PPP2PPP/RNBQKBNR w KQkq - 0 1", "Pirc Defense",
    "rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 1", "Queen's Gambit",
    "rnbqkb1r/pppppp1p/5np1/8/2PP4/8/PP2PPPP/RNBQKBNR w KQkq - 0 1", "King's Indian Defense",
    "rnbqk2r/pppp1ppp/4pn2/8/1bPP4/2N5/PP2PPPP/R1BQKBNR w KQkq - 0 1", "Nimzo-Indian Defense",
    "rnbqkb1r/ppp1pp1p/5np1/3p4/2PP4/2N5/PP2PPPP/R1BQKBNR w KQkq d6 0 1", "Grünfeld Defense",
    "rnbqkb1r/ppp1pppp/5n2/3p4/3P1B2/5N2/PPP1PPPP/RN1QKB1R b KQkq - 0 1", "London System",
    "rnbqkbnr/pppppppp/8/8/2P5/8/PP1PPPPP/RNBQKBNR b KQkq c3 0 1", "English Opening",
    "rnbqkbnr/ppp1pppp/8/3p4/8/5NP1/PPPPPP1P/RNBQKB1R b KQkq - 0 1", "King's Indian Attack",
    "rnbqkbnr/pppp1ppp/8/4p3/4P3/2N5/PPPP1PPP/R1BQKBNR b KQkq - 0 1", "Vienna Game",
    "rnbqkbnr/pp2pppp/2p5/3p4/2PP4/8/PP2PPPP/RNBQKBNR w KQkq - 0 1", "Slav Defense",
    "rnbqkb1r/pppp1ppp/4pn2/8/2PP4/6P1/PP2PP1P/RNBQKBNR b KQkq - 0 1", "Catalan Opening",
    "rnbqkb1r/pp3ppp/3p1n2/2pP4/8/2N5/PP2PPPP/R1BQKBNR w KQkq - 0 1", "Benoni Defense: Modern Variation",
};

const std::vector<int32_t> Tournament::timeControls = {
    100, 200, 300, 500
};

const std::vector<int32_t> Tournament::numGames = {
    3, 2, 1, 1
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
            std::string fen = openings[openingIndex * 2];
            std::string openingName = openings[openingIndex * 2 + 1];

            // Erstelle Spielfeld
            Board board(fen);

            bool engine1White = board.getSideToMove() == WHITE;

            // Ausgabe
            std::cout << "Game " << i * 2 + 1 << " of " << numGames[timeControlIndex] * 2 << " (" << openingName << "): ";

            // Spiele Partie
            int32_t result = runGame(board, st1, st2, timeControl);

            // Speichere die Partie als PGN in Datei
            std::string filename;

            filename = "pgn/";
            filename += engineName1;
            filename += "vs";
            filename += engineName2;
            filename += "_";
            filename += std::to_string(timeControl);
            filename += "ms_";
            filename += std::to_string(i + 1);
            filename += "_";
            filename += openingName;
            filename += ".pgn";

            std::ofstream file(filename);

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
                std::cout << "Draw" << std::endl;
            }

            // Wechsele Seiten

            // Erstelle Spielfeld
            board = Board(fen);

            bool engine2White = board.getSideToMove() == WHITE;

            // Ausgabe
            std::cout << "Game " << i * 2 + 2 << " of " << numGames[timeControlIndex] * 2 << " (" << openingName << "): ";

            // Spiele Partie
            result = runGame(board, st2, st1, timeControl);

            // Speichere die Partie als PGN in Datei
            filename = "";

            filename = "pgn/";
            filename += engineName2;
            filename += "vs";
            filename += engineName1;
            filename += "_";
            filename += std::to_string(timeControl);
            filename += "ms_";
            filename += std::to_string(i + 1);
            filename += "_";
            filename += openingName;
            filename += ".pgn";

            file = std::ofstream(filename);

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
                std::cout << "Draw" << std::endl;
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