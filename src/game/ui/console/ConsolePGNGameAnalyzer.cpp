#include "core/utils/MoveNotations.h"
#include "game/ui/console/BoardVisualizer.h"
#include "game/ui/console/ConsolePGNGameAnalyzer.h"
#include "game/ui/console/PortabilityHelper.h"

#include <chrono>
#include <cmath>
#include <iomanip>
#include <sstream>

void ConsolePGNGameAnalyzer::output() {
    std::stringstream analysisHeader;
    analysisHeader << "Analyzing game with restraint: " << searchTime / 1000.0 << "s time" << std::endl;
    std::cout << analysisHeader.str() << "Analyzing moves: " << currentMoveIndex + 1 << "/" << moves.size() << std::endl;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    while(hasNextMove()) {
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        if(std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() > 1000) {
            begin = end;
            clearScreen();
            std::cout << analysisHeader.str() << "Analyzing moves: " << currentMoveIndex + 1 << "/" << moves.size() << std::endl;
        }

        analyzeNextMove();
    }

    board = Board();

    currentMoveIndex = 0;

    clearScreen();

    std::stringstream outputHeader;
    outputHeader << "Analysis complete. Showing interactive output." << std::endl << std::endl;
    outputHeader << "Use the arrow keys to navigate through the game. Press q to quit." << std::endl;

    while(true) {
        clearScreen();

        std::cout << outputHeader.str();
        std::cout << "Move " << currentMoveIndex + 1 << " of " << moves.size() << std::endl;

        std::cout << std::endl;

        std::string scoreStr;
        if(!IS_MATE_SCORE(boardStateAnalyses[currentMoveIndex].score)) {
            std::stringstream scoreStream;
            scoreStream << std::fixed << std::setprecision(2) << boardStateAnalyses[currentMoveIndex].score / 100.0;
            scoreStr = scoreStream.str();
        }
        else {
            int32_t scoreAbs = std::abs(boardStateAnalyses[currentMoveIndex].score);

            int32_t mateIn = (MATE_SCORE - scoreAbs + 1) / 2;

            if(boardStateAnalyses[currentMoveIndex].score > 0)
                scoreStr = "M" + std::to_string(mateIn);
            else
                scoreStr = "-M" + std::to_string(mateIn);
        }

        Move nextMove = moves[currentMoveIndex];
        std::cout << "Next move: " << toFigurineAlgebraicNotation(nextMove, board) << " ";

        if(currentMoveIndex < moves.size() - 1) {
            int32_t nextMoveScore = 0;
            if(currentMoveIndex < boardStateAnalyses.size() - 1)
                nextMoveScore = boardStateAnalyses[currentMoveIndex + 1].score;

            Move bestMove = boardStateAnalyses[currentMoveIndex].variations[0].moves[0];

            int32_t currentScore = boardStateAnalyses[currentMoveIndex].score;

            int32_t side = board.getSideToMove();

            if(side == WHITE) {
                if(nextMove == bestMove)
                    std::cout << " (Best)";
                else {
                    if(currentScore - nextMoveScore > BLUNDER_SCORE_DIFF)
                        std::cout << " (Blunder)";
                    else if(currentScore - nextMoveScore > MISTAKE_SCORE_DIFF)
                        std::cout << " (Mistake)";
                    else if(currentScore - nextMoveScore > OK_SCORE_DIFF)
                        std::cout << " (Inaccuracy)";
                    else
                        std::cout << " (Good)";
                }
            } else {
                if(nextMove == bestMove)
                    std::cout << " (Best)";
                else {
                    if(nextMoveScore - currentScore > BLUNDER_SCORE_DIFF)
                        std::cout << " (Blunder)";
                    else if(nextMoveScore - currentScore > MISTAKE_SCORE_DIFF)
                        std::cout << " (Mistake)";
                    else if(nextMoveScore - currentScore > OK_SCORE_DIFF)
                        std::cout << " (Inaccuracy)";
                    else
                        std::cout << " (Good)";
                }
            } 
        }

        std::cout << std::endl << std::endl;

        std::cout << "Score: " << scoreStr << std::endl;
        std::cout << std::endl;

        std::cout << "Variations:" << std::endl;
        for(uint32_t i = 0; i < boardStateAnalyses[currentMoveIndex].variations.size(); i++) {
            std::string scoreStr;
            if(!IS_MATE_SCORE(boardStateAnalyses[currentMoveIndex].variations[i].score)) {
                std::stringstream scoreStream;
                scoreStream << std::fixed << std::setprecision(2) << boardStateAnalyses[currentMoveIndex].variations[i].score / 100.0;
                scoreStr = scoreStream.str();
            }
            else {
                int32_t scoreAbs = std::abs(boardStateAnalyses[currentMoveIndex].variations[i].score);

                int32_t mateIn = (MATE_SCORE - scoreAbs + 1) / 2;

                if(boardStateAnalyses[currentMoveIndex].variations[i].score > 0)
                    scoreStr = "M" + std::to_string(mateIn);
                else
                    scoreStr = "-M" + std::to_string(mateIn);
            }

            std::cout << std::setw(3) << i + 1 << ". Score: " << std::setw(5) << scoreStr << " - ";
            for(std::string move : variationToFigurineAlgebraicNotation(boardStateAnalyses[currentMoveIndex].variations[i].moves, board, currentMoveIndex))
                std::cout << move << " ";

            std::cout << std::endl;
        }

        std::cout << std::string(5, '\n');

        std::cout << visualizeBoardWithFigurines(board);

        std::cout << std::endl;
        
        int input = getchArrowKey();

        if(input == 'q')
            break;


        if(input == KEY_ARROW_LEFT) {
            if(currentMoveIndex > 0) {
                currentMoveIndex--;
                board.undoMove();
            }
        } else if(input == KEY_ARROW_RIGHT) {
            if(currentMoveIndex < moves.size() - 1)
                board.makeMove(moves[currentMoveIndex++]);
        }
    }
}