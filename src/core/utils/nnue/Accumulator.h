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

            constexpr const HalfKPLayer<41024, Network::SINGLE_SUBNET_SIZE>& getHalfKPLayer() const noexcept {
                return network.getHalfKPLayer();
            }

        public:
            constexpr Accumulator(const Network& net) : network(net) {}
            constexpr ~Accumulator() {}

            constexpr void refresh(const Array<int32_t, 63>& activeFeatures, int32_t color) noexcept {
                int32_t perspective = color / COLOR_MASK;

                for(size_t i = 0; i < Network::SINGLE_SUBNET_SIZE; i += 16)
                    copy16i16(getHalfKPLayer().getBiasPtr() + i, accumulator[perspective] + i);

                for(int32_t activeFeature : activeFeatures)
                    for(size_t i = 0; i < Network::SINGLE_SUBNET_SIZE; i += 16)
                        add16i16(getHalfKPLayer().getWeightPtr(activeFeature) + i, accumulator[perspective] + i);
            }

            constexpr void update(const Array<int32_t, 3>& addedFeatures, const Array<int32_t, 3>& removedFeatures, int32_t color) noexcept {
                int32_t perspective = color / COLOR_MASK;

                for(int32_t addedFeature : addedFeatures)
                    for(size_t i = 0; i < Network::SINGLE_SUBNET_SIZE; i += 16)
                        add16i16(getHalfKPLayer().getWeightPtr(addedFeature) + i, accumulator[perspective] + i);

                for(int32_t removedFeature : removedFeatures)
                    for(size_t i = 0; i < Network::SINGLE_SUBNET_SIZE; i += 16)
                        sub16i16(getHalfKPLayer().getWeightPtr(removedFeature) + i, accumulator[perspective] + i);
            }

            constexpr const int16_t* getOutput(int32_t color) const noexcept {
                return accumulator[color / COLOR_MASK];
            }

            constexpr void setOutput(int32_t color, const int16_t* output) noexcept {
                int32_t perspective = color / COLOR_MASK;

                for(size_t i = 0; i < Network::SINGLE_SUBNET_SIZE; i += 16)
                    copy16i16(output + i, accumulator[perspective] + i);
            }

    };
}

#endif