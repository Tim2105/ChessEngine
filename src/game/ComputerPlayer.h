#ifndef COMPUTER_PLAYER_H
#define COMPUTER_PLAYER_H

#include "core/engine/Engine.h"
#include "game/Player.h"

#define UNUSED(x) (void)(x)

class ComputerPlayer : public Player {

    private:
        Engine& engine;
        static constexpr uint32_t DEFAULT_SEARCH_TIME = 5000;

    public:
        ComputerPlayer(Engine& engine) : Player(engine.getBoard()), engine(engine) {};
        ~ComputerPlayer() {};
        Move getMove() override;
        Move getMove(uint32_t remainingTime) override;

        inline void onGameEnd(uint8_t result) override {
            UNUSED(result);
        };

};

#endif