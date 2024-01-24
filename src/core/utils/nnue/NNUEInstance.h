#ifndef NNUE_INSTANCE_H
#define NNUE_INSTANCE_H

#include "core/utils/nnue/Accumulator.h"
#include "core/utils/nnue/NNUENetwork.h"

#include <vector>

namespace NNUE {
    class Instance {
        private:
            DefaultAccumulator accumulator;
            std::vector<Array<int16_t, 2 * Network::SINGLE_SUBNET_SIZE>> pastAccumulators;

            void initializeFromBoard(const Board& board, int32_t color) noexcept;
            void updateAfterOppKingMove(const Board& board, int32_t color, Move move) noexcept;

        public:
            Instance() noexcept;
            ~Instance() noexcept;

            int32_t evaluate(int32_t color) const noexcept;
            void initializeFromBoard(const Board& board) noexcept;
            void updateAfterMove(const Board& board) noexcept;
            void undoMove() noexcept;

            inline void clearPastAccumulators() noexcept {
                pastAccumulators.clear();
            }

    };
}

#endif