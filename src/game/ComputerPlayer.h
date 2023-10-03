#ifndef COMPUTER_PLAYER_H
#define COMPUTER_PLAYER_H

#include "core/engine/Engine.h"
#include "game/Player.h"

class ComputerPlayer : public Player {

    private:
        Engine& engine;
        static constexpr uint32_t DEFAULT_SEARCH_TIME = 1000;

    public:
        ComputerPlayer(Engine& engine) : Player(engine.getBoard()), engine(engine) {};
        ~ComputerPlayer() = default;
        Move getMove() override;
        Move getMove(uint32_t remainingTime) override;

};

#endif