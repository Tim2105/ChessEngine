#ifndef PLAYER_H
#define PLAYER_H

#include "core/chess/Board.h"

#define DRAW 0
#define WHITE_WON 1
#define BLACK_WON 2
#define WHITE_WON_BY_TIME 3
#define BLACK_WON_BY_TIME 4

class Player {
    protected:
        Board& board;

    public:
        Player(Board& board) : board(board) {}
        virtual ~Player() = default;
        virtual Move getMove() = 0;
        virtual Move getMove(uint32_t remainingTime) = 0;

        virtual void onGameEnd(uint8_t result) = 0;
};

#endif