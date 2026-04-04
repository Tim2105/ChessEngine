#ifndef LAYER_H
#define LAYER_H

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <istream>
#include <stdint.h>

#include "core/utils/nnue/FileUtils.h"
#include "core/utils/nnue/Vectorized.h"

namespace NNUE {

    template <size_t IN_SIZE, size_t OUT_SIZE>
    class HalfKPLayer {
        alignas(CACHE_LINE_ALIGNMENT) int16_t bias[OUT_SIZE];
        alignas(CACHE_LINE_ALIGNMENT) int16_t weights[IN_SIZE][OUT_SIZE];

        public:
            constexpr HalfKPLayer() {}
            constexpr ~HalfKPLayer() {}

            inline void forward(const int16_t input[IN_SIZE], int16_t output[OUT_SIZE]) const noexcept {
                // Setze Biases
                std::copy(bias, bias + OUT_SIZE, output);

                // Skalarprodukte
                for(size_t i = 0; i < IN_SIZE; i++)
                    for(size_t j = 0; j < OUT_SIZE; j++)
                        output[j] += input[i] * weights[i][j];
            }

            inline int16_t getBias(size_t i) const noexcept {
                return bias[i];
            }

            inline int16_t getWeight(size_t in, size_t out) const noexcept {
                return weights[in][out];
            }

            constexpr int16_t& getBias(size_t i) noexcept {
                return bias[i];
            }

            constexpr int16_t& getWeight(size_t in, size_t out) noexcept {
                return weights[in][out];
            }

            constexpr int16_t* getBiasPtr() noexcept {
                return bias;
            }

            constexpr int16_t* getWeightPtr(size_t in) noexcept {
                return weights[in];
            }

            constexpr const int16_t* getBiasPtr() const noexcept {
                return bias;
            }

            constexpr const int16_t* getWeightPtr(size_t in) const noexcept {
                return weights[in];
            }
    };

    template <size_t IN_SIZE, size_t OUT_SIZE>
    inline std::istream& operator>>(std::istream& is, HalfKPLayer<IN_SIZE, OUT_SIZE>& layer) {
        readLittleEndian(is, layer.getBiasPtr(), OUT_SIZE);
        readLittleEndian(is, layer.getWeightPtr(0), IN_SIZE * OUT_SIZE);

        if(!is.good())
            throw std::runtime_error("Error while reading layer (" + std::to_string(IN_SIZE) +
                                     "x" + std::to_string(OUT_SIZE) + ")");

        return is;
    }


    template <size_t IN_SIZE, size_t OUT_SIZE, bool POST_ACTIVATION = true, bool PRE_ACTIVATION = false>
    class DenseLayer {
        alignas(CACHE_LINE_ALIGNMENT) int32_t bias[OUT_SIZE];
        alignas(CACHE_LINE_ALIGNMENT) int8_t weights[OUT_SIZE][IN_SIZE];

        public:
            constexpr DenseLayer() {}
            constexpr ~DenseLayer() {}

            inline void forward(const int8_t input[IN_SIZE], int32_t output[OUT_SIZE]) const noexcept
                requires(!POST_ACTIVATION && !PRE_ACTIVATION) {
                linearI8ToI32<IN_SIZE, OUT_SIZE>(input, (int8_t*)&weights, bias, output);
            }

            inline void forward(const int8_t input[IN_SIZE], int8_t output[OUT_SIZE]) const noexcept
                requires(POST_ACTIVATION && !PRE_ACTIVATION) {
                linearReLUI8ToI8<IN_SIZE, OUT_SIZE>(input, (int8_t*)&weights, bias, output);
            }

            inline void forward(const int16_t input[IN_SIZE], int8_t output[OUT_SIZE]) const noexcept
                requires(POST_ACTIVATION && PRE_ACTIVATION) {
                halfKPOutputForwardI16ToI8<IN_SIZE, OUT_SIZE>(input, (int8_t*)&weights, bias, output);
            }

            inline int32_t getBias(size_t i) const noexcept {
                return bias[i];
            }

            inline int8_t getWeight(size_t in, size_t out) const noexcept {
                return weights[out][in];
            }

            constexpr int32_t& getBias(size_t i) noexcept {
                return bias[i];
            }

            constexpr int8_t& getWeight(size_t in, size_t out) noexcept {
                return weights[out][in];
            }

            constexpr int32_t* getBiasPtr() noexcept {
                return bias;
            }

            constexpr int8_t* getWeightPtr(size_t in) noexcept {
                return weights[in];
            }

            constexpr const int32_t* getBiasPtr() const noexcept {
                return bias;
            }

            constexpr const int8_t* getWeightPtr(size_t out) const noexcept {
                return weights[out];
            }
    };

    template <size_t IN_SIZE, size_t OUT_SIZE, bool POST_ACTIVATION, bool PRE_ACTIVATION>
    inline std::istream& operator>>(std::istream& is, DenseLayer<IN_SIZE, OUT_SIZE, POST_ACTIVATION, PRE_ACTIVATION>& layer) {
        readLittleEndian(is, layer.getBiasPtr(), OUT_SIZE);
        readLittleEndian(is, layer.getWeightPtr(0), IN_SIZE * OUT_SIZE);

        if(!is.good())
            throw std::runtime_error("Error while reading layer (" + std::to_string(IN_SIZE) +
                                     "x" + std::to_string(OUT_SIZE) + ")");

        return is;
    }

    // using HalfKPLayer = ColMajorLinearLayer<41024, 256, int16_t, int16_t>;
    
    // template <size_t IN_SIZE, size_t OUT_SIZE>
    // using DenseLayer = RowMajorLinearLayerI8ToI32<IN_SIZE, OUT_SIZE>;
}

#endif