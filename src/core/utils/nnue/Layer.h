#ifndef LAYER_H
#define LAYER_H

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <istream>
#include <stdint.h>

#include "core/utils/nnue/FileUtils.h"

namespace NNUE {

    template <typename T>
    concept Numeric = std::is_arithmetic_v<T>;

    template <typename T>
    concept Integral = std::is_integral_v<T>;

    template <size_t IN_SIZE, size_t OUT_SIZE, Numeric IN_T, Numeric OUT_T>
    class Layer {
        public:
            constexpr Layer() {}
            virtual constexpr ~Layer() {}

            virtual void forward(const IN_T input[IN_SIZE], OUT_T output[OUT_SIZE]) const noexcept = 0;
    };

    template <size_t IN_SIZE, size_t OUT_SIZE, Numeric IN_T, Numeric OUT_T,
              Numeric WEIGHT_T = IN_T, Numeric BIAS_T = OUT_T>
    class LinearLayer : public Layer<IN_SIZE, OUT_SIZE, IN_T, OUT_T> {
        public:
            constexpr LinearLayer() {}
            virtual constexpr ~LinearLayer() {}

            virtual BIAS_T getBias(size_t i) const noexcept = 0;
            virtual WEIGHT_T getWeight(size_t in, size_t out) const noexcept = 0;
            virtual BIAS_T& getBias(size_t i) noexcept = 0;
            virtual WEIGHT_T& getWeight(size_t in, size_t out) noexcept = 0;
    };

    template <size_t IN_SIZE, size_t OUT_SIZE, Numeric IN_T, Numeric OUT_T,
            Numeric WEIGHT_T = IN_T, Numeric BIAS_T = OUT_T>
    class ColMajorLinearLayer : public LinearLayer<IN_SIZE, OUT_SIZE, IN_T, OUT_T, WEIGHT_T, BIAS_T> {
        alignas(64) BIAS_T bias[OUT_SIZE];

        alignas(64) WEIGHT_T weights[IN_SIZE][OUT_SIZE];

        public:
            constexpr ColMajorLinearLayer() {}
            virtual constexpr ~ColMajorLinearLayer() {}

            inline void forward(const IN_T input[IN_SIZE], OUT_T output[OUT_SIZE]) const noexcept override {
                // Setze Biases
                for(size_t i = 0; i < OUT_SIZE; i++)
                    output[i] = bias[i];

                // Skalarprodukte
                for(size_t i = 0; i < IN_SIZE; i++)
                    for(size_t j = 0; j < OUT_SIZE; j++)
                        output[j] += input[i] * weights[i][j];
            }

            inline BIAS_T getBias(size_t i) const noexcept override {
                return bias[i];
            }

            inline WEIGHT_T getWeight(size_t in, size_t out) const noexcept override {
                return weights[in][out];
            }

            constexpr BIAS_T& getBias(size_t i) noexcept override {
                return bias[i];
            }

            constexpr WEIGHT_T& getWeight(size_t in, size_t out) noexcept override {
                return weights[in][out];
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
    class RowMajorLinearLayer : public LinearLayer<IN_SIZE, OUT_SIZE, IN_T, OUT_T, WEIGHT_T, BIAS_T> {
        alignas(64) BIAS_T bias[OUT_SIZE];

        alignas(64) WEIGHT_T weights[OUT_SIZE][IN_SIZE];

        public:
            constexpr RowMajorLinearLayer() {}
            virtual constexpr ~RowMajorLinearLayer() {}

            inline void forward(const IN_T input[IN_SIZE], OUT_T output[OUT_SIZE]) const noexcept override {
                // Setze Biases
                for(size_t i = 0; i < OUT_SIZE; i++)
                    output[i] = bias[i];

                // Skalarprodukte
                for(size_t i = 0; i < OUT_SIZE; i++)
                    for(size_t j = 0; j < IN_SIZE; j++)
                        output[i] += input[j] * weights[i][j];
            }

            inline BIAS_T getBias(size_t i) const noexcept override {
                return bias[i];
            }

            inline WEIGHT_T getWeight(size_t in, size_t out) const noexcept override {
                return weights[out][in];
            }

            constexpr BIAS_T& getBias(size_t i) noexcept override {
                return bias[i];
            }

            constexpr WEIGHT_T& getWeight(size_t in, size_t out) noexcept override {
                return weights[out][in];
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

    template <Numeric IN_T, Numeric OUT_T>
    using ActivationFunction = OUT_T (*)(IN_T);

    template <size_t N, Numeric IN_T, Numeric OUT_T,
              ActivationFunction<IN_T, OUT_T> F>
    class ActivationLayer : public Layer<N, N, IN_T, OUT_T> {

        public:
            constexpr ActivationLayer() {}
            virtual constexpr ~ActivationLayer() {}

            inline void forward(const IN_T input[N], OUT_T output[N]) const noexcept override {
                for(size_t i = 0; i < N; i++)
                    output[i] = F(input[i]);
            }
    };

    using HalfKPLayer = ColMajorLinearLayer<41024, 256, int16_t, int16_t>;
    
    template <size_t IN_SIZE, size_t OUT_SIZE>
    using DenseLayer = RowMajorLinearLayer<IN_SIZE, OUT_SIZE, int8_t, int32_t>;

    constexpr int8_t clippedReLU(int16_t x) noexcept {
        return std::clamp(x, (int16_t)0, (int16_t)127);
    }

    using ClippedReLULayer = ActivationLayer<256, int16_t, int8_t, clippedReLU>;

    constexpr int8_t scaledClippedReLU(int32_t x) noexcept {
        return std::clamp(x >> 6, 0, 127);
    }

    using ScaledClippedReLULayer = ActivationLayer<32, int32_t, int8_t, scaledClippedReLU>;
}

#endif