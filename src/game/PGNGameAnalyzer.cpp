#include "game/PGNGameAnalyzer.h"
#include "game/Referee.h"

void PGNGameAnalyzer::analyzeNextMove() {
    if(!hasNextMove())
        return;

    int8_t scoreMultiplier = board.getSideToMove() == WHITE ? 1 : -1;

    engine.search(timePerMove);

    BoardStateAnalysis analysis;
    analysis.score = engine.getBestMoveScore() * scoreMultiplier;
    analysis.variations = engine.getVariations();

    for(uint32_t i = 0; i < analysis.variations.size(); i++)
        analysis.variations[i].score *= scoreMultiplier;

    boardStateAnalyses.push_back(analysis);

    board.makeMove(moves[currentMoveIndex++]);

    // If the game has ended, add the final board state analysis
    if(isGameOver(board)) {
        uint32_t score = 0;

        if(isCheckmate(board)) {
            if(board.getSideToMove() == WHITE)
                score = MATE_SCORE;
            else
                score = -MATE_SCORE;
        }

        BoardStateAnalysis finalAnalysis;
        finalAnalysis.score = score;
        boardStateAnalyses.push_back(finalAnalysis);
    }
}