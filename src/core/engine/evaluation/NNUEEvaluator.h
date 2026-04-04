#ifndef NNUE_EVALUATOR_H
#define NNUE_EVALUATOR_H

#include "core/chess/Referee.h"
#include "core/utils/nnue/NNUEInstance.h"

class NNUEEvaluator: public Evaluator {
    private:
        NNUE::Instance networkInstance;

    public:
        NNUEEvaluator(Board& board) : Evaluator(board) {
            networkInstance.initializeFromBoard(board);
        }

        ~NNUEEvaluator() {}

        inline int evaluate() override {
            int32_t evaluation = networkInstance.evaluate(board.getSideToMove());

            // Skaliere die Bewertung in Richtung 0, wenn wir uns der 50-Züge-Regel annähern.
            // (Starte erst nach 10 Zügen, damit die Bewertung nicht zu früh verzerrt wird.)
            int32_t fiftyMoveCounter = board.getFiftyMoveCounter();
            if(fiftyMoveCounter > 20)
                evaluation = (int32_t)evaluation * (100 - fiftyMoveCounter) / 80;

            return evaluation;
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