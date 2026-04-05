#ifndef NNUE_MASTER_WEIGHTS_H
#define NNUE_MASTER_WEIGHTS_H

#include "core/utils/nnue/NNUENetwork.h"

#include <cmath>
#include <limits>
#include <unordered_map>
#include <vector>

namespace NNUE {
    /**
     * @brief Kapselt die Gradienten eines Rückwärtspasses durch das Netzwerk mit Master-Parametern.
     */
    struct Gradients {
        std::vector<float> halfKPBiases = std::vector<float>(NNUE::Network::SINGLE_SUBNET_SIZE, 0);
        std::unordered_map<size_t, float> halfKPWeights;
        std::array<std::vector<float>, NNUE::Network::NUM_LAYERS> denseLayerBiases;
        std::array<std::vector<float>, NNUE::Network::NUM_LAYERS> denseLayerWeights;

        inline Gradients() {
            for(size_t i = 0; i < NNUE::Network::NUM_LAYERS; i++) {
                denseLayerBiases[i] = std::vector<float>(NNUE::Network::LAYER_SIZES[i + 1], 0);
                denseLayerWeights[i] = std::vector<float>(NNUE::Network::LAYER_SIZES[i] * NNUE::Network::LAYER_SIZES[i + 1], 0);
            }
        }
    };

    /**
     * @brief Kapselt die Aktivierungen eines Vorwärtspasses durch das Netzwerk mit Master-Parametern.
     */
    struct NetworkActivations {
        std::vector<float> halfKPOutputs = std::vector<float>(NNUE::Network::LAYER_SIZES[0], 0);
        std::array<std::vector<float>, NNUE::Network::NUM_LAYERS> denseLayerOutputs;

        inline NetworkActivations() {
            for(size_t i = 0; i < NNUE::Network::NUM_LAYERS; i++)
                denseLayerOutputs[i] = std::vector<float>(NNUE::Network::LAYER_SIZES[i + 1], 0);
        }

        inline float output() const noexcept {
            return denseLayerOutputs[NNUE::Network::NUM_LAYERS - 1][0];
        }
    };

    /**
     * @brief Kapselt die Master-Parameter eines NNUE-Netzwerks.
     */
    struct MasterWeights {
        static constexpr float HALF_KP_MIN = std::numeric_limits<int16_t>::min() / 64.0f;
        static constexpr float HALF_KP_MAX = std::numeric_limits<int16_t>::max() / 64.0f;
        static constexpr float DENSE_BIAS_MIN = std::numeric_limits<int32_t>::min() / (64.0f * 64.0f);
        static constexpr float DENSE_BIAS_MAX = (std::numeric_limits<int32_t>::max() - 65) / (64.0f * 64.0f);
        static constexpr float DENSE_WEIGHT_MIN = std::numeric_limits<int8_t>::min() / 64.0f;
        static constexpr float DENSE_WEIGHT_MAX = std::numeric_limits<int8_t>::max() / 64.0f;

        // Master-Parameter des HalfKP-Layers
        std::vector<float> exactHalfKPBiases = std::vector<float>(NNUE::Network::SINGLE_SUBNET_SIZE, 0);
        std::vector<float> exactHalfKP = std::vector<float>(NNUE::Network::INPUT_SIZE * NNUE::Network::SINGLE_SUBNET_SIZE, 0);

        // Master-Parameter der Dense-Layer
        std::array<std::vector<float>, NNUE::Network::NUM_LAYERS> exactDenseLayerBiases;
        std::array<std::vector<float>, NNUE::Network::NUM_LAYERS> exactDenseLayerWeights;

        inline MasterWeights() {
            for(size_t i = 0; i < NNUE::Network::NUM_LAYERS; i++) {
                exactDenseLayerBiases[i] = std::vector<float>(NNUE::Network::LAYER_SIZES[i + 1], 0);
                exactDenseLayerWeights[i] = std::vector<float>(NNUE::Network::LAYER_SIZES[i] * NNUE::Network::LAYER_SIZES[i + 1], 0);
            }
        }

        MasterWeights(const NNUE::Network& network);

        /**
         * @brief Konvertiert die Master-Parameter zurück in ein NNUE-Netzwerk. Dabei werden die Parameter quantisiert.
         * Das zurückgegebene Objekt muss vom Aufrufer freigegeben werden.
         */
        NNUE::Network* toNetwork() const;

        /**
         * @brief Führt einen Vorwärtspass durch das Netzwerk mit den Master-Parametern durch und gibt die Aktivierungen zurück.
         * Der Vorwärtspass führt "Fake-Quantisierungen" durch um das Verhalten des quantisierten Netzwerks besser zu approximieren.
         */
        NetworkActivations forward(const Board& board) const;

        /**
         * @brief Führt einen Rückwärtspass durch das Netzwerk mit den Master-Parametern durch und gibt die Gradienten zurück.
         * Der Rückwärtspass führt "Fake-Quantisierungen" durch um das Verhalten des quantisierten Netzwerks besser zu approximieren.
         */
        Gradients backward(const Board& board, const NetworkActivations& activations, float outputGrad) const;
    };
}

#endif