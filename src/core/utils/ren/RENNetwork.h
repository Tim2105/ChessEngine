#ifndef REN_NETWORK_H
#define REN_NETWORK_H

#include "core/chess/Board.h"
#include "core/utils/ren/Layer.h"

#include <fstream>
#include <tuple>

using namespace NNUE;

namespace REN {
    class Network {
        public:
            static constexpr uint32_t SUPPORTED_VERSION = 0x2Au;
            static constexpr size_t INPUT_SIZE = 22540;
            static constexpr size_t REN_BLOCK_SIZE = 32;
            static constexpr size_t REN_SIZE = REN_BLOCK_SIZE * REN_BLOCK_SIZE;

        private:
            HalfKAv2_hmLayer<INPUT_SIZE, REN_SIZE / 2> halfKAv2_hmLayer;
            SparseLayer<REN_BLOCK_SIZE> renLayer;
            DenseLayer<REN_SIZE, 1> outputLayer;

        public:
            Network();
            ~Network() = default;

            friend std::istream& operator>>(std::istream& is, Network& network);
            friend std::ostream& operator<<(std::ostream& os, const Network& network);

            constexpr const HalfKAv2_hmLayer<INPUT_SIZE, REN_SIZE / 2>& getHalfKAv2_hmLayer() const noexcept {
                return halfKAv2_hmLayer;
            }

            constexpr const SparseLayer<REN_BLOCK_SIZE>& getRENLayer() const noexcept {
                return renLayer;
            }

            constexpr const DenseLayer<REN_SIZE, 1>& getOutputLayer() const noexcept {
                return outputLayer;
            }
    };

    extern Network DEFAULT_NETWORK;
}

#endif