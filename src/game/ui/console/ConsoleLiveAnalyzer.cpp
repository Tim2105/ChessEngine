#include "core/utils/MoveNotations.h"
#include "game/Referee.h"
#include "game/ui/console/BoardVisualizer.h"
#include "game/ui/console/ConsoleLiveAnalyzer.h"
#include "game/ui/console/ConsolePlayer.h"
#include "game/ui/console/PortabilityHelper.h"

#include <iomanip>
#include <iostream>
#include <sstream>

void ConsoleLiveAnalyzer::start() {
    while(!quit) {
        if(!isGameOver(board)) {
            maxDepthReached = true;
            analyse();
        } else {
            SearchDetails details;
            details.depth = 0;
            details.nodesSearched = 0;
            details.timeTaken = std::chrono::milliseconds(1);
            details.variations = std::vector<Variation>();
            details.variations.push_back(Variation());
            
            if(isDraw(board))
                details.variations[0].score = 0;
            else if(board.getSideToMove() == WHITE)
                details.variations[0].score = MATE_SCORE;
            else
                details.variations[0].score = -MATE_SCORE;

            output(details);

            while(true) {
                int ch = ngetch();

                if(ch == 'q') {
                    quit = true;
                    enterMoveMode = false;
                    undoMoveMode = false;
                    break;
                } else if(ch == 'u') {
                    if(board.getMoveHistory().size() > 0) {
                        undoMoveMode = true;
                        enterMoveMode = false;
                        quit = false;
                        break;
                    }
                }
            }
        }

        if(maxDepthReached) {
            SearchDetails details = engine->getSearchDetails();
            modifyScoreIfBlack(details);
            output(details);

            while(true) {
                int ch = ngetch();

                if(ch == 'q') {
                    quit = true;
                    enterMoveMode = false;
                    undoMoveMode = false;
                    break;
                } else if(ch == 'm') {
                    enterMoveMode = true;
                    quit = false;
                    undoMoveMode = false;
                    break;
                } else if(ch == 'u') {
                    if(board.getMoveHistory().size() > 0) {
                        undoMoveMode = true;
                        enterMoveMode = false;
                        quit = false;
                        break;
                    }
                }
            }
        }

        if(enterMoveMode) {
            #ifndef _WIN32
                SearchDetails details = engine->getSearchDetails();
                modifyScoreIfBlack(details);
                output(details);
            #endif

            std::cout << "Live Analysis paused." << std::endl;

            ConsolePlayer player(board);
            Move move = player.getMove();

            board.makeMove(move);
        } else if(undoMoveMode)
            board.undoMove();
    }
}

bool ConsoleLiveAnalyzer::shouldStop() {
    if(nkbhit()) {
        int ch = ngetch();

        if(ch == 'q') {
            quit = true;
            enterMoveMode = false;
            undoMoveMode = false;
            maxDepthReached = false;
            return true;
        } else if(ch == 'm') {
            enterMoveMode = true;
            quit = false;
            undoMoveMode = false;
            maxDepthReached = false;
            return true;
        } else if(ch == 'u') {
            if(board.getMoveHistory().size() > 0) {
                undoMoveMode = true;
                enterMoveMode = false;
                quit = false;
                maxDepthReached = false;
                return true;
            }
        }
    }

    return false;
}

void ConsoleLiveAnalyzer::output(SearchDetails details) {
    clearScreen();

    std::cout << "Live analysis. Showing interactive output." << std::endl << std::endl;
    std::cout << "Press q to quit. Press m to enter a move. Press u to undo the last move." << std::endl << std::endl;

    std::string scoreStr;
    if(!IS_MATE_SCORE(details.getBestMoveScore())) {
        if(isGameOver(board)) {
            scoreStr = "1/2-1/2";
        } else {
            std::stringstream scoreStream;
            scoreStream << std::fixed << std::setprecision(2) << details.getBestMoveScore() / 100.0;
            scoreStr = scoreStream.str();
        }
    } else {
        int32_t scoreAbs = std::abs(details.getBestMoveScore());
        int32_t mateIn = (MATE_SCORE - scoreAbs + 1) / 2;

        if(mateIn == 0) {
            if(details.getBestMoveScore() > 0)
                scoreStr = "1-0";
            else
                scoreStr = "0-1";
        } else {
            if(details.getBestMoveScore() > 0)
                scoreStr = "M" + std::to_string(mateIn);
            else
                scoreStr = "-M" + std::to_string(mateIn);
        }
    }

    std::cout << "Score: " << scoreStr << std::endl;
    std::cout << "Depth: " << details.depth << ", Nodes: "
        << details.nodesSearched << ", kN/s: " << std::fixed << std::setprecision(2)
        << details.kiloNodesPerSecond() << std::endl << std::endl;

    if(details.getBestMove() != Move()) {
        std::cout << "Variations: " << std::endl;

        for(uint32_t i = 0; i < details.variations.size(); i++) {
            Variation& variation = details.variations[i];

            std::string scoreStr;
            if(!IS_MATE_SCORE(variation.score)) {
                std::stringstream scoreStream;
                scoreStream << std::fixed << std::setprecision(2) << variation.score / 100.0;
                scoreStr = scoreStream.str();
            } else {
                int32_t scoreAbs = std::abs(variation.score);
                int32_t mateIn = (MATE_SCORE - scoreAbs + 1) / 2;

                if(variation.score > 0)
                    scoreStr = "M" + std::to_string(mateIn);
                else
                    scoreStr = "-M" + std::to_string(mateIn);
            }

            std::cout << std::setw(3) << i + 1 << ". Score: " << std::setw(5) << scoreStr << " - ";
            for(std::string move : variationToFigurineAlgebraicNotation(variation.moves, board))
                std::cout << move << " ";

            std::cout << std::endl;
        }
    }

    std::cout << std::string(5, '\n');

    std::cout << visualizeBoardWithFigurines(board);

    std::cout << std::endl;
}