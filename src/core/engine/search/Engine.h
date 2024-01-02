#ifndef ENGINE_H
#define ENGINE_H

#include "core/chess/Board.h"
#include "core/engine/evaluation/Evaluator.h"
#include "core/engine/search/Variation.h"

#include <functional>
#include <stdint.h>

#define SCORE_MIN -32000
#define SCORE_MAX 32000

#define SCORE_MATE 21000
#define IS_MATE_SCORE(x) (std::abs(x) > SCORE_MATE - 1000)

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

        virtual void setHashTableSize(uint32_t size) = 0;
        virtual size_t getHashTableSize() = 0;
        virtual void clearHashTable() = 0;

        virtual inline void setNumVariations(uint32_t numVariations) {
            this->numVariations = numVariations;
        }

        virtual inline void setBoard(Board& board) {
            this->board = &board;
            evaluator.setBoard(board);
        }

        virtual inline Board& getBoard() {
            return *board;
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