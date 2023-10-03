#ifndef TIME_CONTROLLED_GAME_H
#define TIME_CONTROLLED_GAME_H

#include "game/Game.h"

class TimeControlledGame : public Game {

    private:
        bool whiteTimeControlled;
        uint32_t whiteTime;
        bool blackTimeControlled;
        uint32_t blackTime;

    public:
        TimeControlledGame(Board& board, Player& whitePlayer, Player& blackPlayer, GameStateOutput& gst, uint32_t whiteTime, uint32_t blackTime)
                            : Game(board, whitePlayer, blackPlayer, gst), whiteTime(whiteTime), blackTime(blackTime) {
                                whiteTimeControlled = whiteTime > 0;
                                blackTimeControlled = blackTime > 0;
                            };

        ~TimeControlledGame() = default;

        void start() override;
        std::string getWhiteAdditionalInfo() const override;
        std::string getBlackAdditionalInfo() const override;
};

#endif