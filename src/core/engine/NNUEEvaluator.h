#ifndef NNUE_EVALUATOR_H
#define NNUE_EVALUATOR_H

#include "core/engine/SimpleUpdatedEvaluator.h"
#include "core/utils/tables/HeapHashTable.h"
#include "core/utils/nnue/NNUENetwork.h"

class NNUEEvaluator: public UpdatedEvaluator {

    private:
        NNUE::Network network;
        HeapHashTable<uint64_t, int32_t, 131072, 2> evaluationCache;

    public:
        NNUEEvaluator(Board& board, std::istream& networkStream) : UpdatedEvaluator(board) {
            networkStream >> network;
        }

        ~NNUEEvaluator() {}

        inline int32_t evaluate() override {
            uint64_t hash = b->getHashValue();
            int32_t evaluation;

            if(!evaluationCache.probe(hash, evaluation)) {
                evaluation = network.evaluate(b->getSideToMove());
                evaluationCache.put(hash, evaluation);
            }

            return evaluation;
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