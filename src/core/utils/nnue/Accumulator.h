#ifndef ACCUMULATOR_H
#define ACCUMULATOR_H

#include "core/chess/BoardDefinitions.h"
#include "core/utils/nnue/Layer.h"

#include <vector>

namespace NNUE {
    template <size_t IN_SIZE, size_t OUT_SIZE>
    class Accumulator {

        private:
            int16_t accumulator[2][OUT_SIZE];
            LinearLayer<IN_SIZE, OUT_SIZE, int16_t, int16_t>* layer;

        public:
            constexpr Accumulator() : layer(nullptr) {}
            constexpr Accumulator(LinearLayer<IN_SIZE, OUT_SIZE, int16_t, int16_t>* layer) : layer(layer) {}
            constexpr ~Accumulator() {}

            constexpr void setLayer(LinearLayer<IN_SIZE, OUT_SIZE, int16_t, int16_t>* layer) noexcept {
                this->layer = layer;
            }

            constexpr void refresh(const std::vector<int32_t>& activeFeatures, int32_t color) noexcept {
                int32_t perspective = color / COLOR_MASK;

                for(size_t i = 0; i < OUT_SIZE; i++)
                    accumulator[perspective][i] = layer->getBias(i);

                for(int32_t activeFeature : activeFeatures)
                    for(size_t i = 0; i < OUT_SIZE; i++)
                        accumulator[perspective][i] += layer->getWeight(activeFeature, i);
            }

            constexpr void update(const std::vector<int32_t>& addedFeatures, const std::vector<int32_t>& removedFeatures, int32_t color) noexcept {
                int32_t perspective = color / COLOR_MASK;

                for(int32_t addedFeature : addedFeatures)
                    for(size_t i = 0; i < OUT_SIZE; i++)
                        accumulator[perspective][i] += layer->getWeight(addedFeature, i);

                for(int32_t removedFeature : removedFeatures)
                    for(size_t i = 0; i < OUT_SIZE; i++)
                        accumulator[perspective][i] -= layer->getWeight(removedFeature, i);
            }

            constexpr const int16_t* getOutput(int32_t color) const noexcept {
                return accumulator[color / COLOR_MASK];
            }

            constexpr void setOutput(int32_t color, const int16_t* output) noexcept {
                int32_t perspective = color / COLOR_MASK;

                for(size_t i = 0; i < OUT_SIZE; i++)
                    accumulator[perspective][i] = output[i];
            }

    };

    using DefaultAccumulator = Accumulator<41024, 256>;
}

#endif