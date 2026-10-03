#ifndef REN_LAYER_H
#define REN_LAYER_H

#include <algorithm>
#include <cstddef>
#include <istream>
#include <stdint.h>

#include "core/utils/nnue/FileUtils.h"
#include "core/utils/nnue/Layer.h"
#include "core/utils/nnue/Vectorized.h"

namespace REN {

    template <size_t SIZE>
    class SparseLayer {
        alignas(REQUIRED_ALIGNMENT) int32_t bias[SIZE * SIZE] = {0};
        alignas(REQUIRED_ALIGNMENT) int8_t weights[SIZE][SIZE][SIZE] = {{{0}}};

        public:
            constexpr SparseLayer() {}
            constexpr ~SparseLayer() {}

            static inline void createDynamicBias(const int16_t halfKPOutput[SIZE * SIZE], const int32_t bias[SIZE * SIZE], int32_t dynamicBias[SIZE * SIZE]) noexcept {
                reLUI16ToI32Bias<SIZE * SIZE>(halfKPOutput, bias, dynamicBias);
            }

            inline void forward(const int16_t input[SIZE * SIZE], const int32_t dynamicBias[SIZE * SIZE], int8_t output[SIZE * SIZE]) const noexcept {
                halfKPOutputSparseForwardI16ToI8<SIZE>(input, (int8_t*)&weights, dynamicBias, output);
            }

            inline void forward(const int8_t input[SIZE * SIZE], const int32_t dynamicBias[SIZE * SIZE], int8_t output[SIZE * SIZE]) const noexcept {
                sparseLinearReLUI8ToI8<SIZE>(input, (int8_t*)&weights, dynamicBias, output);
            }

            inline int32_t getBias(size_t i) const noexcept {
                return bias[i];
            }

            inline int8_t getWeight(size_t block, size_t in, size_t out) const noexcept {
                return weights[block][in][out];
            }

            constexpr int32_t& getBias(size_t i) noexcept {
                return bias[i];
            }

            constexpr int8_t& getWeight(size_t block, size_t in, size_t out) noexcept {
                return weights[block][in][out];
            }

            constexpr int32_t* getBiasPtr() noexcept {
                return bias;
            }

            constexpr int8_t* getWeightPtr(size_t block, size_t in) noexcept {
                return weights[block][in];
            }

            constexpr const int32_t* getBiasPtr() const noexcept {
                return bias;
            }

            constexpr const int8_t* getWeightPtr(size_t block, size_t in) const noexcept {
                return weights[block][in];
            }
    };

    template <size_t SIZE>
    inline std::istream& operator>>(std::istream& is, SparseLayer<SIZE>& layer) {
        NNUE::readLittleEndian(is, layer.getBiasPtr(), SIZE * SIZE);
        NNUE::readLittleEndian(is, layer.getWeightPtr(0, 0), SIZE * SIZE * SIZE);

        if(!is.good())
            throw std::runtime_error("Error while reading SparseLayer");

        return is;
    }

    template <size_t SIZE>
    inline std::ostream& operator<<(std::ostream& os, const SparseLayer<SIZE>& layer) {
        NNUE::writeLittleEndian(os, layer.getBiasPtr(), SIZE * SIZE);
        NNUE::writeLittleEndian(os, layer.getWeightPtr(0, 0), SIZE * SIZE * SIZE);

        if(!os.good())
            throw std::runtime_error("Error while writing SparseLayer");

        return os;
    }
}

#endif