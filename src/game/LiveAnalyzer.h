#ifndef LIVE_ANALYZER_H
#define LIVE_ANALYZER_H

#include "core/engine/StaticEvaluator.h"

#include "game/InterruptedEngine.h"

class LiveAnalyzer {

    protected:
        Board& board;
        StaticEvaluator evaluator;
        InterruptedEngine* engine;

        void analyse();
        void modifyScoreIfBlack(SearchDetails& details);

    public:
        LiveAnalyzer(Board& board) : board(board), evaluator(board) {
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

            engine = new InterruptedEngine(evaluator, checkupCallback, newDepthCallback, 3);
        };

        virtual ~LiveAnalyzer() {
            delete engine;
        };

        virtual bool shouldStop() = 0;
        virtual void output(SearchDetails details) = 0;

        virtual inline void start() {
            analyse();
        };

};

#endif