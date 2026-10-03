#ifndef REN_EVALUATOR_H
#define REN_EVALUATOR_H

#include "core/chess/Referee.h"
#include "core/engine/evaluation/Evaluator.h"
#include "core/utils/ren/RenInstance.h"
#include "core/utils/nnue/NNUEUtils.h"

class RENEvaluator: public Evaluator {
    private:
        REN::Instance networkInstance;

    public:
        RENEvaluator(Board& board) : Evaluator(board), networkInstance(REN::DEFAULT_NETWORK) {
            networkInstance.initializeFromBoard(board);
        }

        RENEvaluator(Board& board, const REN::Network& renNetwork) : Evaluator(board), networkInstance(renNetwork) {
            networkInstance.initializeFromBoard(board);
        }

        ~RENEvaluator() {}

        inline int evaluate() override {
            return networkInstance.evaluate(board.getSideToMove(), 4);
        }

        inline int evaluate(EvalType type) override {
            switch(type) {
                case EvalType::NULL_WINDOW:
                    return networkInstance.evaluate(board.getSideToMove(), 0);
                case EvalType::PV_NODE:
                    return networkInstance.evaluate(board.getSideToMove(), 2);
                case EvalType::HIGH_DEPTH:
                    return networkInstance.evaluate(board.getSideToMove(), 4);
                default:
                    return networkInstance.evaluate(board.getSideToMove(), 0);
            }
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