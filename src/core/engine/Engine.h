#ifndef ENGINE_H
#define ENGINE_H

#include "core/chess/Board.h"
#include "core/engine/Evaluator.h"
#include "core/engine/Variation.h"

#include <functional>
#include <stdint.h>

#define MIN_SCORE -32000
#define MAX_SCORE 32000

#define MATE_SCORE 21000
#define IS_MATE_SCORE(x) (std::abs(x) > MATE_SCORE - 1000)

struct MoveScorePair {
    Move move;
    int32_t score;
};

template<>
struct std::greater<MoveScorePair> {
    bool operator()(const MoveScorePair& lhs, const MoveScorePair& rhs) {
        return lhs.score > rhs.score;
    }
};

template<>
struct std::less<MoveScorePair> {
    bool operator()(const MoveScorePair& lhs, const MoveScorePair& rhs) {
        return lhs.score < rhs.score;
    }
};

class Engine {

    protected:
        Board* board;
        Evaluator& evaluator;

        std::vector<Variation> variations;
        uint32_t numVariations = 0;

    public:
        Engine(Evaluator& evaluator, uint32_t numVariations = 1) : board(&(evaluator.getBoard())),
                                                                       evaluator(evaluator),
                                                                       numVariations(numVariations) {};

        virtual ~Engine() {};

        virtual void search(uint32_t time, bool treatAsTimeControl = false) = 0;
        virtual void stop() = 0;

        virtual inline void setNumVariations(uint32_t numVariations) {
            this->numVariations = numVariations;
        }

        virtual inline void setBoard(Board& board) {
            this->board = &board;
            evaluator.setBoard(board);
        }

        virtual inline Move getBestMove() {
            return variations.empty() ? Move() : variations[0].moves[0];
        }

        virtual inline int16_t getBestMoveScore() {
            return variations.empty() ? 0 : variations[0].score;
        }

        virtual inline std::vector<Variation> getVariations() {
            return variations;
        }

        virtual inline std::vector<Move> getPrincipalVariation() {
            return variations.empty() ? std::vector<Move>() : variations[0].moves;
        }
};

#endif