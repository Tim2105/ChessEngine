#ifndef CONSOLE_PLAYER_H
#define CONSOLE_PLAYER_H

#include "game/ui/UserPlayer.h"

class ConsolePlayer : public UserPlayer {

    public:
        ConsolePlayer(Board& b) : UserPlayer(b) {};
        ~ConsolePlayer() = default;
        Move getMove() override;
        Move getMove(uint32_t remainingTime) override;

        void onGameEnd(uint8_t result) override;

};

#endif