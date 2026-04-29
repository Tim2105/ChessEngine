#ifndef ML_QUANTIZATION_H
#define ML_QUANTIZATION_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

constexpr float clippedReLU(float x, float maxVal) {
    return std::clamp(x, 0.0f, maxVal);
}

constexpr bool clippedReLUDerivative(float x, float maxVal) {
    return x > 0.0f && x < maxVal;
}

namespace ML {
    struct FakeQuantizationI8 {
        constexpr static float ONE = 128.0f;
        constexpr static float MIN_VALUE = std::numeric_limits<int8_t>::min() / ONE;
        constexpr static float MAX_VALUE = std::numeric_limits<int8_t>::max() / ONE;
        constexpr static float CLIPPED_RELU_MAX = (std::numeric_limits<int8_t>::max() / ONE);

        /**
         * @brief Quantisierungsfunktion,
         * die einen Wert auf einen darstellbaren Wert im quantisierten Format rundet.
         * 
         * @param x Der Wert.
         * @param minVal Das Minimum des Intervalls.
         * @param maxVal Das Maximum des Intervalls.
         * @param scale Der Skalierungsfaktor der Quantisierung.
         */
        constexpr float operator()(float x) const {
            return std::clamp(std::round(x * ONE) / ONE, MIN_VALUE, MAX_VALUE);
        }
    };

    struct FakeQuantizationI16 {
        constexpr static float ONE = 128.0f;
        constexpr static float MIN_VALUE = std::numeric_limits<int16_t>::min() / ONE;
        constexpr static float MAX_VALUE = std::numeric_limits<int16_t>::max() / ONE;
        constexpr static float CLIPPED_RELU_MAX = FakeQuantizationI8::CLIPPED_RELU_MAX;

        /**
         * @brief Quantisierungsfunktion,
         * die einen Wert auf einen darstellbaren Wert im quantisierten Format rundet.
         * 
         * @param x Der Wert.
         * @param minVal Das Minimum des Intervalls.
         * @param maxVal Das Maximum des Intervalls.
         * @param scale Der Skalierungsfaktor der Quantisierung.
         */
        constexpr float operator()(float x) const {
            return std::clamp(std::round(x * ONE) / ONE, MIN_VALUE, MAX_VALUE);
        }
    };

    struct Identity {
        constexpr static float CLIPPED_RELU_MAX = FakeQuantizationI8::CLIPPED_RELU_MAX;

        /**
         * @brief Identitätsfunktion, die als Quantisierungsfunktion verwendet werden kann,
         * wenn keine Quantisierung durchgeführt werden soll.
         */
        constexpr float operator()(float x) const {
            return x;
        }
    };
}

#endif