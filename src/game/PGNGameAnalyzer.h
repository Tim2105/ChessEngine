#ifndef PGN_GAME_ANALYZER_H
#define PGN_GAME_ANALYZER_H

#include "core/chess/Board.h"
#include "core/chess/Move.h"

#include "core/engine/MinimaxEngine.h"
#include "core/engine/StaticEvaluator.h"

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
        class PGNGameAnalyzerEngine : public MinimaxEngine {
            private:
                static constexpr double EST_BRANCHING_FACTOR = 1.6;

                double timePerMove;
                uint16_t lastRecordedDepth = 0;

                inline void checkupCallback() {
                    SearchDetails details = getSearchDetails();
                    uint16_t depth = details.depth;
                    uint64_t timeSpent = details.timeTaken.count();

                    if(depth > lastRecordedDepth && timeSpent > (timePerMove / EST_BRANCHING_FACTOR))
                        stop();

                    lastRecordedDepth = depth;
                }

            public:
                PGNGameAnalyzerEngine(Evaluator& e, double timePerMove)
                    : MinimaxEngine(e, 3, 100, std::bind(&PGNGameAnalyzerEngine::checkupCallback, this))
                    , timePerMove(timePerMove) {}

        };

        StaticEvaluator* evaluator;
        PGNGameAnalyzerEngine* engine;
        double timePerMove;

    public:
        PGNGameAnalyzer(std::string pgn, uint32_t searchTime) : searchTime(searchTime) {

            board = Board::fromPGN(pgn);

            std::vector<MoveHistoryEntry> history = board.getMoveHistory();
            for(MoveHistoryEntry entry : history)
                moves.push_back(entry.move);

            timePerMove = searchTime / (double)history.size();

            board = Board();
            evaluator = new StaticEvaluator(board);
            engine = new PGNGameAnalyzerEngine(*evaluator, timePerMove);
        };

        ~PGNGameAnalyzer() {
            delete evaluator;
            delete engine;
        };

        void analyzeNextMove();
        inline bool hasNextMove() { return currentMoveIndex < moves.size(); };

        virtual void output() = 0;
};

#endif