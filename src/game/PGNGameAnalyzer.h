#ifndef PGN_GAME_ANALYZER_H
#define PGN_GAME_ANALYZER_H

#include "core/chess/Board.h"
#include "core/chess/Move.h"
#include "core/engine/Evaluator.h"
#include "core/engine/Engine.h"

#include <string>
#include <vector>

struct BoardStateAnalysis {
    int32_t score;
    std::vector<Variation> variations;
};

class PGNGameAnalyzer {
    protected:
        Board board;

        std::vector<Move> moves;
        std::vector<BoardStateAnalysis> boardStateAnalyses;
        uint32_t currentMoveIndex = 0;
        uint32_t searchTime;

    private:
        Engine& engine;

        uint32_t timePerMove;

    public:
        PGNGameAnalyzer(std::string pgn, Engine& engine, uint32_t searchTime) : searchTime(searchTime), engine(engine)  {
            board = Board::fromPGN(pgn);

            std::vector<MoveHistoryEntry> history = board.getMoveHistory();
            for(MoveHistoryEntry entry : history)
                moves.push_back(entry.move);

            board = Board();
            engine.setBoard(board);

            timePerMove = searchTime / history.size();
        };

        void analyzeNextMove();
        inline bool hasNextMove() { return currentMoveIndex < moves.size(); };

        virtual void output() = 0;
};

#endif