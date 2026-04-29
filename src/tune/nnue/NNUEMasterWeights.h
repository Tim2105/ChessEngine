#ifndef NNUE_MASTER_WEIGHTS_H
#define NNUE_MASTER_WEIGHTS_H

#include "core/utils/nnue/NNUENetwork.h"
#include "tune/ml/HalfKAv2_hm.h"
#include "tune/ml/DenseLayer.h"

#include <cmath>
#include <limits>
#include <unordered_map>
#include <vector>

namespace NNUE {
    /**
     * @brief Kapselt die Gradienten eines Rückwärtspasses durch das Netzwerk mit Master-Parametern.
     */
    struct Gradients {
        ML::HalfKAv2_hmLayer::Gradients halfKAGradients{NNUE::Network::SINGLE_SUBNET_SIZE};
        ML::DenseLayer::Gradients denseLayerGradients[NNUE::Network::NUM_LAYERS] {
            ML::DenseLayer::Gradients(NNUE::Network::LAYER_SIZES[0], NNUE::Network::LAYER_SIZES[1]),
            ML::DenseLayer::Gradients(NNUE::Network::LAYER_SIZES[1], NNUE::Network::LAYER_SIZES[2]),
            ML::DenseLayer::Gradients(NNUE::Network::LAYER_SIZES[2], NNUE::Network::LAYER_SIZES[3])
        };
    };

    /**
     * @brief Kapselt die Aktivierungen eines Vorwärtspasses durch das Netzwerk mit Master-Parametern.
     */
    struct NetworkActivations {
        ML::HalfKAv2_hmLayer::ForwardResult halfKPActivations{NNUE::Network::LAYER_SIZES[0]};
        ML::DenseLayer::ForwardResult denseLayerOutputs[NNUE::Network::NUM_LAYERS] {
            ML::DenseLayer::ForwardResult(NNUE::Network::LAYER_SIZES[1]),
            ML::DenseLayer::ForwardResult(NNUE::Network::LAYER_SIZES[2]),
            ML::DenseLayer::ForwardResult(NNUE::Network::LAYER_SIZES[3])
        };

        inline float output() const noexcept {
            return denseLayerOutputs[NNUE::Network::NUM_LAYERS - 1].output(0);
        }
    };

    /**
     * @brief Kapselt die Master-Parameter eines NNUE-Netzwerks.
     */
    struct MasterWeights {
        static constexpr float HALF_KP_MIN = std::numeric_limits<int16_t>::min() / 128.0f;
        static constexpr float HALF_KP_MAX = std::numeric_limits<int16_t>::max() / 128.0f;
        static constexpr float DENSE_BIAS_MIN = std::numeric_limits<int32_t>::min() / (128.0f * 128.0f);
        static constexpr float DENSE_BIAS_MAX = (std::numeric_limits<int32_t>::max() - 65) / (128.0f * 128.0f);
        static constexpr float DENSE_WEIGHT_MIN = std::numeric_limits<int8_t>::min() / 128.0f;
        static constexpr float DENSE_WEIGHT_MAX = std::numeric_limits<int8_t>::max() / 128.0f;

        // Master-Parameter des HalfKP-Layers
        ML::HalfKAv2_hmLayer halfKPLayer{NNUE::Network::LAYER_SIZES[0]};

        // Master-Parameter der Dense-Layer
        ML::DenseLayer denseLayers[NNUE::Network::NUM_LAYERS] {
            ML::DenseLayer(NNUE::Network::LAYER_SIZES[0], NNUE::Network::LAYER_SIZES[1]),
            ML::DenseLayer(NNUE::Network::LAYER_SIZES[1], NNUE::Network::LAYER_SIZES[2]),
            ML::DenseLayer(NNUE::Network::LAYER_SIZES[2], NNUE::Network::LAYER_SIZES[3], false)
        };

        MasterWeights() = default;

        MasterWeights(const NNUE::Network& network);

        /**
         * @brief Konvertiert die Master-Parameter zurück in ein NNUE-Netzwerk. Dabei werden die Parameter quantisiert.
         * Das zurückgegebene Objekt muss vom Aufrufer freigegeben werden.
         */
        NNUE::Network* toNetwork() const;

        /**
         * @brief Führt einen Vorwärtspass durch das Netzwerk mit den Master-Parametern durch und gibt die Aktivierungen zurück.
         * 
         * @param board Das Schachbrett, für das die Aktivierungen berechnet werden sollen.
         * @param fakeQuantization Wenn true ist, werden während des Vorwärtspasses "Fake-Quantisierungen"
         * durchgeführt um das Verhalten des quantisierten Netzwerks besser zu approximieren.
         * Andernfalls werden die genauen Werte der Master-Parameter verwendet.
         */
        NetworkActivations forward(const Board& board, bool fakeQuantization) const;

        /**
         * @brief Führt einen Rückwärtspass durch das Netzwerk mit den Master-Parametern durch und gibt die Gradienten zurück.
         * 
         * @param board Das Schachbrett, für das die Gradienten berechnet werden sollen.
         * @param activations Die Aktivierungen, die während des Vorwärtspasses berechnet wurden.
         * @param outputGrad Der Gradient des Fehlers bezüglich der Ausgabe des Netzwerks (dL/d(output)).
         * @param fakeQuantization Wenn true ist, werden während des Rückwärtspasses "Fake-Quantisierungen" durchgeführt um das Verhalten des quantisierten Netzwerks besser zu approximieren.
         * Andernfalls werden die genauen Werte der Master-Parameter verwendet.
         */
        Gradients backward(const Board& board, const NetworkActivations& activations, float outputGrad, bool fakeQuantization) const;
    };
}

#endif