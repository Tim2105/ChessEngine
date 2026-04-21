#ifndef ACCUMULATOR_H
#define ACCUMULATOR_H

#include "core/chess/BoardDefinitions.h"

#include "core/utils/Array.h"
#include "core/utils/nnue/NNUENetwork.h"
#include "core/utils/nnue/Layer.h"

namespace NNUE {
    class Accumulator {
        private:
            alignas(CACHE_LINE_ALIGNMENT) int16_t accumulator[2][Network::SINGLE_SUBNET_SIZE];
            const Network& network;

            constexpr const HalfKPLayer<Network::INPUT_SIZE, Network::SINGLE_SUBNET_SIZE>& getHalfKPLayer() const noexcept {
                return network.getHalfKPLayer();
            }

        public:
            constexpr Accumulator(const Network& net) : network(net) {}
            constexpr ~Accumulator() {}

            constexpr void refresh(const Array<int, 68>& activeFeatures, int color) noexcept {
                int perspective = color / COLOR_MASK;

                std::copy(getHalfKPLayer().getBiasPtr(), getHalfKPLayer().getBiasPtr() + Network::SINGLE_SUBNET_SIZE, accumulator[perspective]);

                for(int activeFeature : activeFeatures)
                    for(size_t i = 0; i < Network::SINGLE_SUBNET_SIZE; i += 16)
                        add16i16(getHalfKPLayer().getWeightPtr(activeFeature) + i, accumulator[perspective] + i);
            }

            constexpr void update(const Array<int, 8>& addedFeatures, const Array<int, 8>& removedFeatures, int color) noexcept {
                int perspective = color / COLOR_MASK;

                for(int addedFeature : addedFeatures)
                    for(size_t i = 0; i < Network::SINGLE_SUBNET_SIZE; i += 16)
                        add16i16(getHalfKPLayer().getWeightPtr(addedFeature) + i, accumulator[perspective] + i);

                for(int removedFeature : removedFeatures)
                    for(size_t i = 0; i < Network::SINGLE_SUBNET_SIZE; i += 16)
                        sub16i16(getHalfKPLayer().getWeightPtr(removedFeature) + i, accumulator[perspective] + i);
            }

            constexpr const int16_t* getOutput(int color) const noexcept {
                return accumulator[color / COLOR_MASK];
            }

            constexpr void setOutput(int color, const int16_t* output) noexcept {
                int perspective = color / COLOR_MASK;
                std::copy(output, output + Network::SINGLE_SUBNET_SIZE, accumulator[perspective]);
            }

    };
}

#endif