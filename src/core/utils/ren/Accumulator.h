#ifndef REN_ACCUMULATOR_H
#define REN_ACCUMULATOR_H

#include "core/chess/BoardDefinitions.h"

#include "core/utils/Array.h"
#include "core/utils/ren/Layer.h"
#include "core/utils/ren/RENNetwork.h"

namespace REN {
    class Accumulator {
        private:
            alignas(CACHE_LINE_ALIGNMENT) int16_t accumulator[2][Network::REN_SIZE / 2];
            const Network& network;

            constexpr const HalfKAv2_hmLayer<Network::INPUT_SIZE, Network::REN_SIZE / 2>& getHalfKAv2_hmLayer() const noexcept {
                return network.getHalfKAv2_hmLayer();
            }

        public:
            constexpr Accumulator(const Network& net) : network(net) {}
            constexpr ~Accumulator() {}

            constexpr void refresh(const Array<int, 68>& activeFeatures, int color) noexcept {
                int perspective = color / COLOR_MASK;

                std::copy(getHalfKAv2_hmLayer().getBiasPtr(), getHalfKAv2_hmLayer().getBiasPtr() + Network::REN_SIZE / 2, accumulator[perspective]);

                for(int activeFeature : activeFeatures)
                    for(size_t i = 0; i < Network::REN_SIZE / 2; i += 16)
                        add16i16(getHalfKAv2_hmLayer().getWeightPtr(activeFeature) + i, accumulator[perspective] + i);
            }

            constexpr void update(const Array<int, 8>& addedFeatures, const Array<int, 8>& removedFeatures, int color) noexcept {
                int perspective = color / COLOR_MASK;

                for(int addedFeature : addedFeatures)
                    for(size_t i = 0; i < Network::REN_SIZE / 2; i += 16)
                        add16i16(getHalfKAv2_hmLayer().getWeightPtr(addedFeature) + i, accumulator[perspective] + i);

                for(int removedFeature : removedFeatures)
                    for(size_t i = 0; i < Network::REN_SIZE / 2; i += 16)
                        sub16i16(getHalfKAv2_hmLayer().getWeightPtr(removedFeature) + i, accumulator[perspective] + i);
            }

            constexpr const int16_t* getOutput(int color) const noexcept {
                return accumulator[color / COLOR_MASK];
            }

            constexpr void setOutput(int color, const int16_t* output) noexcept {
                int perspective = color / COLOR_MASK;
                std::copy(output, output + Network::REN_SIZE / 2, accumulator[perspective]);
            }

    };
}

#endif