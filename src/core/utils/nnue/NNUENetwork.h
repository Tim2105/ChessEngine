#ifndef NNUE_NETWORK_H
#define NNUE_NETWORK_H

#include "core/chess/Board.h"
#include "core/utils/nnue/Accumulator.h"
#include "core/utils/nnue/Layer.h"

#include <fstream>

namespace NNUE {
    class Network {

        private:
            static constexpr uint32_t SUPPORTED_VERSION = 0x7AF32F16u;
            static constexpr size_t SINGLE_SUBNET_SIZE = 256;

            DefaultAccumulator accumulator;
            std::vector<Array<int16_t, 2 * SINGLE_SUBNET_SIZE>> pastAccumulators;

            HalfKPLayer* halfKPLayer;
            ClippedReLULayer halfKPActivation;
            DenseLayer<2 * SINGLE_SUBNET_SIZE, 32> layer1;
            ScaledClippedReLULayer activation1;
            DenseLayer<32, 32> layer2;
            ScaledClippedReLULayer activation2;
            DenseLayer<32, 1> layer3;

            std::string header;
            uint32_t version, hash, headerSize;
            uint32_t halfKPHash, layer1Hash;

        public:
            Network();
            ~Network();

            int32_t evaluate(int32_t color) const noexcept;
            void initializeFromBoard(const Board& board) noexcept;
            void updateAfterMove(const Board& board) noexcept;
            void undoMove() noexcept;

            inline void clearPastAccumulators() noexcept {
                pastAccumulators.clear();
            }

            friend std::istream& operator>>(std::istream& is, Network& network);
            friend std::ostream& operator<<(std::ostream& os, const Network& network);
    };
}

#endif