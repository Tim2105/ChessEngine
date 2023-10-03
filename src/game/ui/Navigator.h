#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include "game/Game.h"
#include "game/PGNGameAnalyzer.h"

class Navigator {

    public:
        Navigator() = default;

        virtual void navigate() = 0;

        void playGame(Game& game);
        void analyzeGame(PGNGameAnalyzer& analyzer);

};

#endif