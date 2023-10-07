#ifndef CONSOLE_GAME_STATE_OUTPUT_H
#define CONSOLE_GAME_STATE_OUTPUT_H

#include "game/ui/GameStateOutput.h"

#include <string>

class ConsoleGameStateOutput : public GameStateOutput {

    private:
        static constexpr size_t BOARD_WIDTH = 39;

        bool flipBoard = false;

    public:
        ConsoleGameStateOutput(Board& board, bool flipBoard = false) : GameStateOutput(board), flipBoard(flipBoard) {};
        void outputGameState(std::string whiteAdditionalInfo, std::string blackAdditionalInfo) override;
};

#endif