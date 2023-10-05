#include "core/engine/StaticEvaluator.h"

#include "game/LiveAnalyzer.h"

void LiveAnalyzer::analyse() {
    std::function<void()> checkupCallback = [&]() {
        if (shouldStop())
            engine->stop();
    };

    std::function<void()> newDepthCallback = [&]() {
        SearchDetails details = engine->getSearchDetails();

        // Drehe die Vorzeichen der Bewertung um, wenn der Spieler Schwarz ist
        modifyScoreIfBlack(details);

        output(details);
    };
    
    StaticEvaluator evaluator(board);
    engine = new InterruptedEngine(evaluator, checkupCallback, newDepthCallback, 3);

    engine->search(0);
}

void LiveAnalyzer::modifyScoreIfBlack(SearchDetails& details) {
    if(board.getSideToMove() == BLACK) {
        for(Variation& variation : details.variations)
            variation.score *= -1;
    }
}