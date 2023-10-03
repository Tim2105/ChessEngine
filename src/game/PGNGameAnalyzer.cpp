#include "game/PGNGameAnalyzer.h"

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
}