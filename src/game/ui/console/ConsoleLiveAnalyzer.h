#ifndef CONSOLE_LIVE_ANALYZER_H
#define CONSOLE_LIVE_ANALYZER_H

#include "game/LiveAnalyzer.h"

class ConsoleLiveAnalyzer : public LiveAnalyzer {

    private:
        bool maxDepthReached = false;
        bool quit = false;
        bool enterMoveMode = false;
        bool undoMoveMode = false;

        bool shouldStop() override;
        void output(SearchDetails details) override;

    public:
        ConsoleLiveAnalyzer(Board& b) : LiveAnalyzer(b) {};

        void start() override;
};

#endif