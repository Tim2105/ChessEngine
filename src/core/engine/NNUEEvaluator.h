#ifndef NNUE_EVALUATOR_H
#define NNUE_EVALUATOR_H

#include "core/engine/SimpleUpdatedEvaluator.h"
#include "core/utils/tables/HeapHashTable.h"
#include "core/utils/nnue/NNUENetwork.h"

class NNUEEvaluator: public UpdatedEvaluator {

    private:
        NNUE::Network network;

    public:
        NNUEEvaluator(Board& board, std::istream& networkStream) : UpdatedEvaluator(board) {
            networkStream >> network;
            network.initializeFromBoard(board);
        }

        ~NNUEEvaluator() {}

        inline int32_t evaluate() override {
            return network.evaluate(b->getSideToMove());
        }

        inline void updateAfterMove() override {
            network.updateAfterMove(*b);
        }

        inline void updateBeforeUndo() override {
            network.undoMove();
        }

        inline void setBoard(Board& board) override {
            Evaluator::setBoard(board);
            network.clearPastAccumulators();
            network.initializeFromBoard(board);
        }

};

#endif