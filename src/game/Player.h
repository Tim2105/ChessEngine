#ifndef PLAYER_H
#define PLAYER_H

#include "core/chess/Board.h"

class Player {
    protected:
        Board& board;

    public:
        Player(Board& board) : board(board) {}
        virtual ~Player() = default;
        virtual Move getMove() = 0;
        virtual Move getMove(uint32_t remainingTime) = 0;

};

#endif