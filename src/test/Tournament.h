#ifndef TOURNAMENT_H
#define TOURNAMENT_H

#include "core/chess/Board.h"
#include "core/engine/Engine.h"

#include <vector>
#include <string>

class Tournament {

    private:
        class TournamentGameStateOutput : public GameStateOutput {
            public:
                TournamentGameStateOutput(Board& board) : GameStateOutput(board) {};
                inline void outputGameState(std::string whiteAdditionalInfo, std::string blackAdditionalInfo) override {
                    UNUSED(whiteAdditionalInfo);
                    UNUSED(blackAdditionalInfo);
                };
        };

        Engine& st1;
        Engine& st2;
        std::string engineName1;
        std::string engineName2;

        static const std::vector<std::string> openings;
        static const std::vector<int32_t> timeControls;
        static const std::vector<int32_t> numGames;

        GameResult runGame(Board& board, Engine& st1, Engine& st2, int32_t time);

    public:
        Tournament(Engine& st1, Engine& st2, std::string engineName1, std::string engineName2) : st1(st1), st2(st2), engineName1(engineName1), engineName2(engineName2) {};
        ~Tournament() = default;

        void run();
};

#endif