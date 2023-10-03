#ifndef USER_PLAYER_H
#define USER_PLAYER_H

#include "game/Player.h"

class UserPlayer : public Player {

    public:
        UserPlayer(Board& b) : Player(b) {};
        ~UserPlayer() = default;

};

#endif