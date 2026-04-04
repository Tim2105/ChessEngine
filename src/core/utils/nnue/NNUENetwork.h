#ifndef NNUE_NETWORK_H
#define NNUE_NETWORK_H

#include "core/chess/Board.h"
#include "core/utils/nnue/Layer.h"

#include <fstream>

namespace NNUE {
    class Network {
        public:
            static constexpr uint32_t SUPPORTED_VERSION = 0x7AF32F16u;
            static constexpr size_t SINGLE_SUBNET_SIZE = 256;

        private:
            HalfKPLayer<41024, SINGLE_SUBNET_SIZE> halfKPLayer;
            DenseLayer<2 * SINGLE_SUBNET_SIZE, 32, true, true> layer1;
            DenseLayer<32, 32> layer2;
            DenseLayer<32, 1, false> layer3;

            std::string header;
            uint32_t version, hash, headerSize;
            uint32_t halfKPHash, layer1Hash;

        public:
            Network();
            ~Network() = default;

            friend std::istream& operator>>(std::istream& is, Network& network);
            friend std::ostream& operator<<(std::ostream& os, const Network& network);

            constexpr const HalfKPLayer<41024, SINGLE_SUBNET_SIZE>& getHalfKPLayer() const noexcept {
                return halfKPLayer;
            }

            constexpr const DenseLayer<2 * SINGLE_SUBNET_SIZE, 32, true, true>& getLayer1() const noexcept {
                return layer1;
            }

            constexpr const DenseLayer<32, 32>& getLayer2() const noexcept {
                return layer2;
            }

            constexpr const DenseLayer<32, 1, false>& getLayer3() const noexcept {
                return layer3;
            }
    };

    extern Network DEFAULT_NETWORK;
}

#endif