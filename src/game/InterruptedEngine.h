#ifndef INTERRUPTED_ENGINE_H
#define INTERRUPTED_ENGINE_H

#include "core/engine/MinimaxEngine.h"

#include <functional>

class InterruptedEngine : public MinimaxEngine {

    private:
        std::function<void()> checkupCallback;
        std::function<void()> newDepthCallback;

        int32_t currentDepth = 0;

    protected:
        inline void checkup() override {
            MinimaxEngine::checkup();
            checkupCallback();

            SearchDetails details = getSearchDetails();
            if (details.depth != currentDepth) {
                currentDepth = details.depth;
                newDepthCallback();
            }
        }

    public:
        InterruptedEngine(Evaluator& e, std::function<void()> checkupCallback, std::function<void()> newDepthCallback, uint32_t numVariations = 1)
            : MinimaxEngine(e, numVariations), checkupCallback(checkupCallback), newDepthCallback(newDepthCallback) {}
};


#endif
