#ifndef GAME_STATE_OUTPUT_H
#define GAME_STATE_OUTPUT_H

#include "core/chess/Board.h"

class GameStateOutput {
    protected:
        Board& board;

    public:
        GameStateOutput(Board& board) : board(board) {};
        virtual void outputGameState(std::string whiteAdditionalInfo, std::string blackAdditionalInfo) = 0;
};

#endif