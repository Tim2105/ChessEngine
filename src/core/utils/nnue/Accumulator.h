#ifndef ACCUMULATOR_H
#define ACCUMULATOR_H

#include "core/chess/BoardDefinitions.h"
#include "core/utils/Array.h"
#include "core/utils/nnue/Layer.h"

namespace NNUE {
    template <size_t IN_SIZE, size_t OUT_SIZE>
    class Accumulator {

        private:
            int16_t accumulator[2][OUT_SIZE];
            ColMajorLinearLayer<IN_SIZE, OUT_SIZE, int16_t, int16_t>* layer;

        public:
            constexpr Accumulator() : layer(nullptr) {}
            constexpr Accumulator(ColMajorLinearLayer<IN_SIZE, OUT_SIZE, int16_t, int16_t>* layer) : layer(layer) {}
            constexpr ~Accumulator() {}

            constexpr void setLayer(ColMajorLinearLayer<IN_SIZE, OUT_SIZE, int16_t, int16_t>* layer) noexcept {
                this->layer = layer;
            }

            constexpr void refresh(const Array<int32_t, 63>& activeFeatures, int32_t color) noexcept {
                int32_t perspective = color / COLOR_MASK;

                std::copy(layer->getBiasPtr(), layer->getBiasPtr() + OUT_SIZE, accumulator[perspective]);

                for(int32_t activeFeature : activeFeatures)
                    for(size_t i = 0; i < OUT_SIZE; i += 16)
                        add16i16(layer->getWeightPtr(activeFeature) + i, accumulator[perspective] + i);
            }

            constexpr void update(const Array<int32_t, 3>& addedFeatures, const Array<int32_t, 3>& removedFeatures, int32_t color) noexcept {
                int32_t perspective = color / COLOR_MASK;

                for(int32_t addedFeature : addedFeatures)
                    for(size_t i = 0; i < OUT_SIZE; i += 16)
                        add16i16(layer->getWeightPtr(addedFeature) + i, accumulator[perspective] + i);

                for(int32_t removedFeature : removedFeatures)
                    for(size_t i = 0; i < OUT_SIZE; i += 16)
                        sub16i16(layer->getWeightPtr(removedFeature) + i, accumulator[perspective] + i);
            }

            constexpr const int16_t* getOutput(int32_t color) const noexcept {
                return accumulator[color / COLOR_MASK];
            }

            constexpr void setOutput(int32_t color, const int16_t* output) noexcept {
                int32_t perspective = color / COLOR_MASK;

                std::copy(output, output + OUT_SIZE, accumulator[perspective]);
            }

    };

    using DefaultAccumulator = Accumulator<41024, 256>;
}

#endif