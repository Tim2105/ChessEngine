#include "core/engine/StaticEvaluator.h"

#include "game/LiveAnalyzer.h"

void LiveAnalyzer::analyse() {
    engine->search(0);
}

void LiveAnalyzer::modifyScoreIfBlack(SearchDetails& details) {
    if(board.getSideToMove() == BLACK) {
        for(Variation& variation : details.variations)
            variation.score *= -1;
    }
}