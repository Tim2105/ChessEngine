#ifndef CONSOLE_PGN_GAME_ANALYZER_H
#define CONSOLE_PGN_GAME_ANALYZER_H

#include "game/PGNGameAnalyzer.h"

#include <iostream>

class ConsolePGNGameAnalyzer : public PGNGameAnalyzer {
    private:
        static constexpr int32_t OK_SCORE_DIFF = 20;
        static constexpr int32_t MISTAKE_SCORE_DIFF = 100;
        static constexpr int32_t BLUNDER_SCORE_DIFF = 300;

    public:
        ConsolePGNGameAnalyzer(std::string pgn, uint32_t searchTime) : PGNGameAnalyzer(pgn, searchTime) {};

        void output() override;
};

#endif