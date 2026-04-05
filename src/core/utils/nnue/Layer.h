#ifndef LAYER_H
#define LAYER_H

#include <algorithm>
#include <cstddef>
#include <istream>
#include <stdint.h>

#include "core/utils/nnue/FileUtils.h"
#include "core/utils/nnue/Vectorized.h"

namespace NNUE {

    template <size_t IN_SIZE, size_t OUT_SIZE>
    class HalfKPLayer {
        alignas(REQUIRED_ALIGNMENT) int16_t bias[OUT_SIZE];
        alignas(REQUIRED_ALIGNMENT) int16_t weights[IN_SIZE][OUT_SIZE];

        public:
            constexpr HalfKPLayer() {}
            constexpr ~HalfKPLayer() {}

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


    template <size_t IN_SIZE, size_t OUT_SIZE>
    inline std::ostream& operator<<(std::ostream& os, const HalfKPLayer<IN_SIZE, OUT_SIZE>& layer) {
        writeLittleEndian(os, layer.getBiasPtr(), OUT_SIZE);
        writeLittleEndian(os, layer.getWeightPtr(0), IN_SIZE * OUT_SIZE);

        if(!os.good())
            throw std::runtime_error("Error while writing layer (" + std::to_string(IN_SIZE) +
                                     "x" + std::to_string(OUT_SIZE) + ")");

        return os;
    }

    template <size_t IN_SIZE, size_t OUT_SIZE>
    class DenseLayer {
        alignas(REQUIRED_ALIGNMENT) int32_t bias[OUT_SIZE];
        alignas(REQUIRED_ALIGNMENT) int8_t weights[OUT_SIZE][IN_SIZE];

        public:
            using in_size = std::integral_constant<size_t, IN_SIZE>;
            using out_size = std::integral_constant<size_t, OUT_SIZE>;

            constexpr DenseLayer() {}
            constexpr ~DenseLayer() {}

            inline void forward(const int8_t input[IN_SIZE], int32_t output[OUT_SIZE]) const noexcept {
                linearI8ToI32<IN_SIZE, OUT_SIZE>(input, (int8_t*)&weights, bias, output);
            }

            inline void forward(const int8_t input[IN_SIZE], int8_t output[OUT_SIZE]) const noexcept {
                linearReLUI8ToI8<IN_SIZE, OUT_SIZE>(input, (int8_t*)&weights, bias, output);
            }

            inline void forward(const int16_t input[IN_SIZE], int8_t output[OUT_SIZE]) const noexcept {
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

    template <size_t IN_SIZE, size_t OUT_SIZE>
    inline std::istream& operator>>(std::istream& is, DenseLayer<IN_SIZE, OUT_SIZE>& layer) {
        readLittleEndian(is, layer.getBiasPtr(), OUT_SIZE);
        readLittleEndian(is, layer.getWeightPtr(0), IN_SIZE * OUT_SIZE);

        if(!is.good())
            throw std::runtime_error("Error while reading layer (" + std::to_string(IN_SIZE) +
                                     "x" + std::to_string(OUT_SIZE) + ")");

        return is;
    }

    template <size_t IN_SIZE, size_t OUT_SIZE>
    inline std::ostream& operator<<(std::ostream& os, const DenseLayer<IN_SIZE, OUT_SIZE>& layer) {
        writeLittleEndian(os, layer.getBiasPtr(), OUT_SIZE);
        writeLittleEndian(os, layer.getWeightPtr(0), IN_SIZE * OUT_SIZE);

        if(!os.good())
            throw std::runtime_error("Error while writing layer (" + std::to_string(IN_SIZE) +
                                     "x" + std::to_string(OUT_SIZE) + ")");

        return os;
    }
}

#endif