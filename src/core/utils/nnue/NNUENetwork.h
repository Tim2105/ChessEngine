#ifndef NNUE_NETWORK_H
#define NNUE_NETWORK_H

#include "core/chess/Board.h"
#include "core/utils/nnue/Layer.h"

#include <fstream>
#include <tuple>

namespace NNUE {
    class Network {
        public:
            static constexpr uint32_t SUPPORTED_VERSION = 0x7AF32F16u;
            static constexpr size_t INPUT_SIZE = 41024;
            static constexpr size_t SINGLE_SUBNET_SIZE = 256;
            static constexpr size_t LAYER_SIZES[] = {2 * SINGLE_SUBNET_SIZE, 32, 32, 1};
            static constexpr size_t NUM_LAYERS = sizeof(LAYER_SIZES) / sizeof(LAYER_SIZES[0]) - 1;

            static constexpr size_t NUM_PARAMETERS = [] {
                // HalfKP-Layer
                size_t n = SINGLE_SUBNET_SIZE + INPUT_SIZE * SINGLE_SUBNET_SIZE;

                // Dense-Layer
                for(size_t i = 0; i < NUM_LAYERS; i++)
                    n += LAYER_SIZES[i + 1] + LAYER_SIZES[i] * LAYER_SIZES[i + 1];

                return n;
            }();

        private:
            HalfKPLayer<INPUT_SIZE, SINGLE_SUBNET_SIZE> halfKPLayer;
            DenseLayer<LAYER_SIZES[0], LAYER_SIZES[1]> layer1;
            DenseLayer<LAYER_SIZES[1], LAYER_SIZES[2]> layer2;
            DenseLayer<LAYER_SIZES[2], LAYER_SIZES[3]> layer3;

            uint32_t version = SUPPORTED_VERSION;

        public:
            Network();
            ~Network() = default;

            friend std::istream& operator>>(std::istream& is, Network& network);
            friend std::ostream& operator<<(std::ostream& os, const Network& network);

            constexpr const HalfKPLayer<INPUT_SIZE, SINGLE_SUBNET_SIZE>& getHalfKPLayer() const noexcept {
                return halfKPLayer;
            }

            constexpr const DenseLayer<LAYER_SIZES[0], LAYER_SIZES[1]>& getLayer1() const noexcept {
                return layer1;
            }

            constexpr const DenseLayer<LAYER_SIZES[1], LAYER_SIZES[2]>& getLayer2() const noexcept {
                return layer2;
            }

            constexpr const DenseLayer<LAYER_SIZES[2], LAYER_SIZES[3]>& getLayer3() const noexcept {
                return layer3;
            }
    };

    extern Network DEFAULT_NETWORK;
}

#endif