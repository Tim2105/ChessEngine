#ifndef ENGINE_H
#define ENGINE_H

#include "core/chess/Board.h"
#include "core/engine/Evaluator.h"

#include <atomic>
#include <mutex>
#include <stdint.h>

#define MIN_SCORE -32000
#define MAX_SCORE 32000

#define MATE_SCORE 21000
#define IS_MATE_SCORE(x) (std::abs(x) > MATE_SCORE - 1000)

struct Variation {
    std::vector<Move> moves;
    int32_t score;
};

template<>
struct std::greater<Variation> {
    bool operator()(const Variation& lhs, const Variation& rhs) {
        return lhs.score > rhs.score;
    }
};

template<>
struct std::less<Variation> {
    bool operator()(const Variation& lhs, const Variation& rhs) {
        return lhs.score < rhs.score;
    }
};

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
        std::mutex variationsMutex;
        uint32_t numVariations = 0;

        inline void clearVariations() {
            variationsMutex.lock();
            variations.clear();
            variationsMutex.unlock();
        }

        inline void setVariations(std::vector<Variation> variations) {
            variationsMutex.lock();
            this->variations = variations;
            variationsMutex.unlock();
        }

    public:
        Engine(Evaluator& evaluator, uint32_t numVariations = 1) : board(&(evaluator.getBoard())),
                                                                       evaluator(evaluator),
                                                                       numVariations(numVariations) {};

        virtual ~Engine() {};

        virtual void search(uint32_t time, bool dontBlock = false) = 0;
        virtual void stop() = 0;

        inline void setNumVariations(uint32_t numVariations) {
            this->numVariations = numVariations;
        }

        inline void setBoard(Board& board) {
            this->board = &board;
            evaluator.setBoard(board);
        }

        inline Move getBestMove() {
            variationsMutex.lock();
            Move bestMove = variations.empty() ? Move() : variations[0].moves[0];
            variationsMutex.unlock();

            return bestMove;
        }

        inline int16_t getBestMoveScore() {
            variationsMutex.lock();
            int16_t bestMoveScore = variations.empty() ? 0 : variations[0].score;
            variationsMutex.unlock();

            return bestMoveScore;
        }

        inline std::vector<Variation> getVariations() {
            variationsMutex.lock();
            std::vector<Variation> v = variations;
            variationsMutex.unlock();

            return v;
        }
        inline std::vector<Move> getPrincipalVariation() {
            variationsMutex.lock();
            std::vector<Move> pv = variations.empty() ? std::vector<Move>() : variations[0].moves;
            variationsMutex.unlock();

            return pv;
        }
};

#endif