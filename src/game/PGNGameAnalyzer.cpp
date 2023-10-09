#include "core/chess/Referee.h"

#include "game/PGNGameAnalyzer.h"

void PGNGameAnalyzer::analyzeNextMove() {
    if(!hasNextMove())
        return;

    engine->search(timePerMove * 1.5);

    int8_t scoreMultiplier = board.getSideToMove() == WHITE ? 1 : -1;

    BoardStateAnalysis analysis;
    analysis.score = engine->getBestMoveScore() * scoreMultiplier;
    analysis.variations = engine->getVariations();

    for(uint32_t i = 0; i < analysis.variations.size(); i++)
        analysis.variations[i].score *= scoreMultiplier;

    boardStateAnalyses.push_back(analysis);

    board.makeMove(moves[currentMoveIndex++]);

    // Wenn das Spiel vorbei ist, füge noch eine Analyse mit dem Ergebnis hinzu
    if(Referee::isGameOver(board)) {
        uint32_t score = 0;

        if(Referee::isCheckmate(board)) {
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