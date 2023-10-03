#include "game/ComputerPlayer.h"

Move ComputerPlayer::getMove() {
    engine.search(DEFAULT_SEARCH_TIME);

    return engine.getBestMove();
}

Move ComputerPlayer::getMove(uint32_t remainingTime) {
    engine.search(remainingTime, true);

    return engine.getBestMove();
}