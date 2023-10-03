#ifndef CONSOLE_PGN_GAME_ANALYZER_H
#define CONSOLE_PGN_GAME_ANALYZER_H

#include "game/PGNGameAnalyzer.h"

#include <iostream>

class ConsolePGNGameAnalyzer : public PGNGameAnalyzer {
    public:
        ConsolePGNGameAnalyzer(std::string pgn, Engine& engine, uint32_t searchTime) : PGNGameAnalyzer(pgn, engine, searchTime) {};

        void output() override;
};

#endif