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

    template <typename T>
    concept Numeric = std::is_arithmetic_v<T>;

    template <size_t IN_SIZE, size_t OUT_SIZE, Numeric IN_T, Numeric OUT_T,
            Numeric WEIGHT_T = IN_T, Numeric BIAS_T = OUT_T>
    class ColMajorLinearLayer {
        alignas(CACHE_LINE_ALIGNMENT) BIAS_T bias[OUT_SIZE];
        alignas(CACHE_LINE_ALIGNMENT) WEIGHT_T weights[IN_SIZE][OUT_SIZE];

        public:
            constexpr ColMajorLinearLayer() {}
            constexpr ~ColMajorLinearLayer() {}

            inline void forward(const IN_T input[IN_SIZE], OUT_T output[OUT_SIZE]) const noexcept {
                // Setze Biases
                std::copy(bias, bias + OUT_SIZE, output);

                // Skalarprodukte
                for(size_t i = 0; i < IN_SIZE; i++)
                    for(size_t j = 0; j < OUT_SIZE; j++)
                        output[j] += input[i] * weights[i][j];
            }

            inline BIAS_T getBias(size_t i) const noexcept {
                return bias[i];
            }

            inline WEIGHT_T getWeight(size_t in, size_t out) const noexcept {
                return weights[in][out];
            }

            constexpr BIAS_T& getBias(size_t i) noexcept {
                return bias[i];
            }

            constexpr WEIGHT_T& getWeight(size_t in, size_t out) noexcept {
                return weights[in][out];
            }

            constexpr const BIAS_T* getBiasPtr() const noexcept {
                return bias;
            }

            constexpr const WEIGHT_T* getWeightPtr(size_t in) const noexcept {
                return weights[in];
            }
    };

    template <size_t IN_SIZE, size_t OUT_SIZE, Numeric IN_T, Numeric OUT_T,
              Numeric WEIGHT_T, Numeric BIAS_T>
    inline std::istream& operator>>(std::istream& is, ColMajorLinearLayer<IN_SIZE, OUT_SIZE, IN_T, OUT_T, WEIGHT_T, BIAS_T>& layer) {
        for(size_t i = 0; i < OUT_SIZE && is.good(); i++)
            readLittleEndian(is, layer.getBias(i));

        for(size_t i = 0; i < IN_SIZE && is.good(); i++)
            for(size_t j = 0; j < OUT_SIZE && is.good(); j++)
                readLittleEndian(is, layer.getWeight(i, j));

        if(!is.good())
            throw std::runtime_error("Error while reading layer(" + std::to_string(IN_SIZE) +
                                     "x" + std::to_string(OUT_SIZE) + ", bias_t=" +
                                     typeid(BIAS_T).name() + ", weight_t=" + typeid(WEIGHT_T).name() + ")");

        return is;
    }

    template <size_t IN_SIZE, size_t OUT_SIZE, Numeric IN_T, Numeric OUT_T,
              Numeric WEIGHT_T = IN_T, Numeric BIAS_T = OUT_T>
    class RowMajorLinearLayer {
        alignas(CACHE_LINE_ALIGNMENT) BIAS_T bias[OUT_SIZE];
        alignas(CACHE_LINE_ALIGNMENT) WEIGHT_T weights[OUT_SIZE][IN_SIZE];

        public:
            constexpr RowMajorLinearLayer() {}
            constexpr ~RowMajorLinearLayer() {}

            inline void forward(const IN_T input[IN_SIZE], OUT_T output[OUT_SIZE]) const noexcept {
                // Setze Biases
                std::copy(bias, bias + OUT_SIZE, output);

                // Skalarprodukte
                for(size_t i = 0; i < OUT_SIZE; i++)
                    for(size_t j = 0; j < IN_SIZE; j++)
                        output[i] += input[j] * weights[i][j];
            }

            inline BIAS_T getBias(size_t i) const noexcept {
                return bias[i];
            }

            inline WEIGHT_T getWeight(size_t in, size_t out) const noexcept {
                return weights[out][in];
            }

            constexpr BIAS_T& getBias(size_t i) noexcept {
                return bias[i];
            }

            constexpr WEIGHT_T& getWeight(size_t in, size_t out) noexcept {
                return weights[out][in];
            }

            constexpr const BIAS_T* getBiasPtr() const noexcept {
                return bias;
            }

            constexpr const WEIGHT_T* getWeightPtr(size_t in) const noexcept {
                return weights[in];
            }
    };

    template <size_t IN_SIZE, size_t OUT_SIZE, Numeric IN_T, Numeric OUT_T,
              Numeric WEIGHT_T, Numeric BIAS_T>
    inline std::istream& operator>>(std::istream& is, RowMajorLinearLayer<IN_SIZE, OUT_SIZE, IN_T, OUT_T, WEIGHT_T, BIAS_T>& layer) {
        for(size_t i = 0; i < OUT_SIZE && is.good(); i++)
            readLittleEndian(is, layer.getBias(i));

        for(size_t i = 0; i < OUT_SIZE && is.good(); i++)
            for(size_t j = 0; j < IN_SIZE && is.good(); j++)
                readLittleEndian(is, layer.getWeight(j, i));

        if(!is.good())
            throw std::runtime_error("Error while reading layer(" + std::to_string(IN_SIZE) +
                                     "x" + std::to_string(OUT_SIZE) + ", bias_t=" +
                                     typeid(BIAS_T).name() + ", weight_t=" + typeid(WEIGHT_T).name() + ")");

        return is;
    }

    template <size_t IN_SIZE, size_t OUT_SIZE>
    class RowMajorLinearLayerI8ToI32 {
        alignas(CACHE_LINE_ALIGNMENT) int32_t bias[OUT_SIZE];
        alignas(CACHE_LINE_ALIGNMENT) int8_t weights[OUT_SIZE][IN_SIZE];

        public:
            constexpr RowMajorLinearLayerI8ToI32() {}
            constexpr ~RowMajorLinearLayerI8ToI32() {}

            inline void forward(const int8_t input[IN_SIZE], int32_t output[OUT_SIZE]) const noexcept {
                linearI8ToI32<IN_SIZE, OUT_SIZE>(input, (int8_t*)&weights, bias, output);
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

            constexpr const int32_t* getBiasPtr() const noexcept {
                return bias;
            }

            constexpr const int8_t* getWeightPtr(size_t in) const noexcept {
                return weights[in];
            }
    };

    template <size_t IN_SIZE, size_t OUT_SIZE>
    inline std::istream& operator>>(std::istream& is, RowMajorLinearLayerI8ToI32<IN_SIZE, OUT_SIZE>& layer) {
        for(size_t i = 0; i < OUT_SIZE && is.good(); i++)
            readLittleEndian(is, layer.getBias(i));

        for(size_t i = 0; i < OUT_SIZE && is.good(); i++) {
            for(size_t j = 0; j < IN_SIZE && is.good(); j++) {
                // Lies ein Byte und konvertiere es zu int16_t
                int8_t weight;
                readLittleEndian(is, weight);
                layer.getWeight(j, i) = weight;
            }
        }

        if(!is.good())
            throw std::runtime_error("Error while reading layer(" + std::to_string(IN_SIZE) +
                                     "x" + std::to_string(OUT_SIZE) + ", bias_t=" +
                                     typeid(int32_t).name() + ", weight_t=" + typeid(int8_t).name() + ")");

        return is;
    }

    template <Numeric IN_T, Numeric OUT_T>
    using ActivationFunction = OUT_T (*)(IN_T);

    template <size_t N, Numeric IN_T, Numeric OUT_T,
              ActivationFunction<IN_T, OUT_T> F>
    class ActivationLayer {
        public:
            constexpr ActivationLayer() {}
            constexpr ~ActivationLayer() {}

            inline void forward(const IN_T input[N], OUT_T output[N]) const noexcept {
                for(size_t i = 0; i < N; i++)
                    output[i] = F(input[i]);
            }
    };

    class ClippedReLULayer {
        public:
            constexpr ClippedReLULayer() {}
            constexpr ~ClippedReLULayer() {}

            inline void forward(const int16_t input[256], int8_t output[256]) const noexcept {
                for(size_t i = 0; i < 256; i += 32)
                    clippedReLU32i16(input + i, output + i);
            }
    };

    class ScaledClippedReLULayer {
        public:
            constexpr ScaledClippedReLULayer() {}
            constexpr ~ScaledClippedReLULayer() {}

            inline void forward(const int32_t input[32], int8_t output[32]) const noexcept {
                scaledClippedReLU32i32(input, output);
            }
    };

    using HalfKPLayer = ColMajorLinearLayer<41024, 256, int16_t, int16_t>;
    
    template <size_t IN_SIZE, size_t OUT_SIZE>
    using DenseLayer = RowMajorLinearLayerI8ToI32<IN_SIZE, OUT_SIZE>;
}

#endif