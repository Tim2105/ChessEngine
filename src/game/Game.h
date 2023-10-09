#ifndef GAME_H
#define GAME_H

#include "core/chess/Board.h"
#include "game/ui/GameStateOutput.h"
#include "game/GameResult.h"
#include "game/Player.h"

class Game {
    private:
        GameStateOutput& gameStateOutput;

    protected:
        Board& board;
        Player& whitePlayer;
        Player& blackPlayer;

        GameResult result = GameResult::RUNNING;

        void outputGameState();
        void saveGameToFile();

    public:
        Game(Board& board, Player& whitePlayer, Player& blackPlayer, GameStateOutput& gst) :
            gameStateOutput(gst), board(board), whitePlayer(whitePlayer), blackPlayer(blackPlayer)  {};

        ~Game() = default;

        virtual void start();

        virtual inline std::string getWhiteAdditionalInfo() const { return ""; };
        virtual inline std::string getBlackAdditionalInfo() const { return ""; };

        constexpr Board& getBoard() const { return board; };
        constexpr Player& getWhitePlayer() const { return whitePlayer; };
        constexpr Player& getBlackPlayer() const { return blackPlayer; };

        constexpr GameResult getResult() const { return result; };
};

#endif