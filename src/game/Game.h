#ifndef GAME_H
#define GAME_H

#include "core/chess/Board.h"
#include "game/Player.h"

class Game {

    protected:
        Board& board;
        Player& whitePlayer;
        Player& blackPlayer;

    public:
        Game(Board& board, Player& whitePlayer, Player& blackPlayer) : board(board), whitePlayer(whitePlayer), blackPlayer(blackPlayer) {};
        ~Game() = default;
        virtual void start();

};

#endif