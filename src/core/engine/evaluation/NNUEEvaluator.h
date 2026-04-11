#ifndef NNUE_EVALUATOR_H
#define NNUE_EVALUATOR_H

#include "core/chess/Referee.h"
#include "core/engine/evaluation/Evaluator.h"
#include "core/utils/nnue/NNUEInstance.h"
#include "core/utils/nnue/NNUEUtils.h"

class NNUEEvaluator: public Evaluator {
    private:
        NNUE::Instance networkInstance;

    public:
        NNUEEvaluator(Board& board) : Evaluator(board), networkInstance(NNUE::DEFAULT_NETWORK) {
            networkInstance.initializeFromBoard(board);
        }

        NNUEEvaluator(Board& board, const NNUE::Network& nnueNetwork) : Evaluator(board), networkInstance(nnueNetwork) {
            networkInstance.initializeFromBoard(board);
        }

        ~NNUEEvaluator() {}

        inline int evaluate() override {
            int16_t additionalInput[NNUE::Network::INPUT_ADDITION];
            NNUE::fillAdditionalInput(board, additionalInput);

            return networkInstance.evaluate(board.getSideToMove(), additionalInput);
        }

        inline void updateAfterMove() override {
            networkInstance.updateAfterMove(board);
        }

        inline void updateBeforeUndo() override {
            networkInstance.undoMove();
        }

        inline void setBoard(Board& board) override {
            Evaluator::setBoard(board);
            networkInstance.clearPastAccumulators();
            networkInstance.initializeFromBoard(board);
        }
};

#endif