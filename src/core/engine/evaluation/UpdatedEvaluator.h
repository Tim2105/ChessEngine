#ifndef UPDATED_EVALUATOR_H
#define UPDATED_EVALUATOR_H

#define UNUSED(x) (void)(x)

#include "core/engine/evaluation/Evaluator.h"

class UpdatedEvaluator: public Evaluator {
    public:
        UpdatedEvaluator(Board& b) : Evaluator(b) {}

        virtual void updateBeforeMove(Move m) { UNUSED(m); }
        virtual void updateAfterMove() {}
        virtual void updateBeforeUndo() {}
        virtual void updateAfterUndo(Move m) { UNUSED(m); }
};

#endif