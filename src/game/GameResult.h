#ifndef GAME_RESULT_H
#define GAME_RESULT_H

class GameResult {
    
    public:
        enum Result {
            RUNNING = -1,
            DRAW,
            WHITE_WON,
            WHITE_WON_BY_TIME,
            BLACK_WON,
            BLACK_WON_BY_TIME
        };

    private:
        Result result;

    public:
        GameResult() = default;
        GameResult(Result result) : result(result) {};

        constexpr operator Result() const { return result; };

        constexpr bool operator==(const Result& other) const { return result == other; };
        constexpr bool operator!=(const Result& other) const { return result != other; };

        constexpr bool isRunning() const { return result == RUNNING; };
        constexpr bool isDraw() const { return result == DRAW; };
        constexpr bool whiteWon() const { return result == WHITE_WON || result == WHITE_WON_BY_TIME; };
        constexpr bool blackWon() const { return result == BLACK_WON || result == BLACK_WON_BY_TIME; };

};

#endif