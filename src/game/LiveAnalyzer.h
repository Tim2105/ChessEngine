#ifndef LIVE_ANALYZER_H
#define LIVE_ANALYZER_H

#include "game/InterruptedEngine.h"

class LiveAnalyzer {

    protected:
        Board& board;
        InterruptedEngine* engine;

        void analyse();
        void modifyScoreIfBlack(SearchDetails& details);

    public:
        LiveAnalyzer(Board& board) : board(board) {};

        virtual bool shouldStop() = 0;
        virtual void output(SearchDetails details) = 0;

        virtual inline void start() {
            analyse();
        };

};

#endif