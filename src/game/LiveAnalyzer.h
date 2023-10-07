#ifndef LIVE_ANALYZER_H
#define LIVE_ANALYZER_H

#include "core/engine/MinimaxEngine.h"
#include "core/engine/StaticEvaluator.h"

class LiveAnalyzer {
    private:
        int32_t depth;

    protected:
        Board& board;
        StaticEvaluator evaluator;
        MinimaxEngine* engine;

        void analyse();
        void modifyScoreIfBlack(SearchDetails& details);

    public:
        LiveAnalyzer(Board& board) : board(board), evaluator(board) {
            std::function<void()> checkupCallback = [&]() {
                if(shouldStop())
                    engine->stop();
                else {
                    if(depth != engine->getLastSearchDepth()) {
                        SearchDetails details = engine->getSearchDetails();
                        depth = details.depth;

                        // Drehe die Vorzeichen der Bewertung um, wenn der Spieler Schwarz ist
                        modifyScoreIfBlack(details);

                        output(details);
                    }
                }
            };

            engine = new MinimaxEngine(evaluator, 3, 10, checkupCallback);
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