#ifndef PLAYER_H
#define PLAYER_H

#include "core/chess/Board.h"

#include "game/GameResult.h"

class Player {
    protected:
        Board& board;

    public:
        Player(Board& board) : board(board) {}
        virtual ~Player() {}
        virtual Move getMove() = 0;
        virtual Move getMove(uint32_t remainingTime) = 0;

        virtual void onGameEnd(GameResult result) = 0;
};

#endif