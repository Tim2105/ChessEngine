#include "core/utils/nnue/NNUEMasterWeights.h"
#include "core/utils/nnue/NNUEUtils.h"

#include <cmath>

NNUE::MasterWeights::MasterWeights(const NNUE::Network& network) : MasterWeights() {
    const auto& halfKP = network.getHalfKPLayer();

    // Biases, dequantisiert
    for(size_t i = 0; i < NNUE::Network::SINGLE_SUBNET_SIZE; i++)
        exactHalfKPBiases[i] = halfKP.getBias(i) / 64.0f;

    // Gewichte, dequantisiert
    for(size_t i = 0; i < NNUE::Network::INPUT_SIZE; i++) {
        for(size_t j = 0; j < NNUE::Network::SINGLE_SUBNET_SIZE; j++)
            exactHalfKP[i * NNUE::Network::SINGLE_SUBNET_SIZE + j] = halfKP.getWeight(i, j) / 64.0f;
    }

    const auto& layer1 = network.getLayer1();
    const auto& layer2 = network.getLayer2();
    const auto& layer3 = network.getLayer3();

    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[1]; i++) {
        exactDenseLayerBiases[0][i] = layer1.getBias(i) / (64.0f * 64.0f);

        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[0]; j++)
            exactDenseLayerWeights[0][i * NNUE::Network::LAYER_SIZES[0] + j] = layer1.getWeight(j, i) / 64.0f;
    }

    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[2]; i++) {
        exactDenseLayerBiases[1][i] = layer2.getBias(i) / (64.0f * 64.0f);

        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[1]; j++)
            exactDenseLayerWeights[1][i * NNUE::Network::LAYER_SIZES[1] + j] = layer2.getWeight(j, i) / 64.0f;
    }

    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[3]; i++) {
        exactDenseLayerBiases[2][i] = layer3.getBias(i) / (64.0f * 64.0f);

        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[2]; j++)
            exactDenseLayerWeights[2][i * NNUE::Network::LAYER_SIZES[2] + j] = layer3.getWeight(j, i) / 64.0f;
    }
}

NNUE::Network* NNUE::MasterWeights::toNetwork() const {
    NNUE::Network* network = new NNUE::Network;

    auto& halfKP = network->getHalfKPLayer();

    int16_t* biasPtrHalfKP = (int16_t*)halfKP.getBiasPtr();
    for(size_t i = 0; i < NNUE::Network::SINGLE_SUBNET_SIZE; i++)
        biasPtrHalfKP[i] = (int16_t)(std::round(exactHalfKPBiases[i] * 64.0f));

    for(size_t i = 0; i < NNUE::Network::INPUT_SIZE; i++) {
        int16_t* weightPtr = (int16_t*)halfKP.getWeightPtr(i);
        for(size_t j = 0; j < NNUE::Network::SINGLE_SUBNET_SIZE; j++)
            weightPtr[j] = (int16_t)(std::round(exactHalfKP[i * NNUE::Network::SINGLE_SUBNET_SIZE + j] * 64.0f));
    }

    auto& layer1 = network->getLayer1();
    auto& layer2 = network->getLayer2();
    auto& layer3 = network->getLayer3();

    int32_t* biasPtr = (int32_t*)layer1.getBiasPtr();
    int8_t* weightPtr;
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[1]; i++) {
        biasPtr[i] = (int32_t)(std::round(exactDenseLayerBiases[0][i] * 64.0f * 64.0f));

        weightPtr = (int8_t*)layer1.getWeightPtr(i);
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[0]; j++)
            weightPtr[j] = (int8_t)(std::round(exactDenseLayerWeights[0][i * NNUE::Network::LAYER_SIZES[0] + j] * 64.0f));
    }

    biasPtr = (int32_t*)layer2.getBiasPtr();
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[2]; i++) {
        biasPtr[i] = (int32_t)(std::round(exactDenseLayerBiases[1][i] * 64.0f * 64.0f));

        weightPtr = (int8_t*)layer2.getWeightPtr(i);
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[1]; j++)
            weightPtr[j] = (int8_t)(std::round(exactDenseLayerWeights[1][i * NNUE::Network::LAYER_SIZES[1] + j] * 64.0f));
    }

    biasPtr = (int32_t*)layer3.getBiasPtr();
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[3]; i++) {
        biasPtr[i] = (int32_t)(std::round(exactDenseLayerBiases[2][i] * 64.0f * 64.0f));

        weightPtr = (int8_t*)layer3.getWeightPtr(i);
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[2]; j++)
            weightPtr[j] = (int8_t)(std::round(exactDenseLayerWeights[2][i * NNUE::Network::LAYER_SIZES[2] + j] * 64.0f));
    }

    return network;
}

constexpr float clippedReLU(float x) {
    return std::clamp(x, 0.0f, (float)(std::numeric_limits<int8_t>::max() / 64.0));
}

constexpr bool clippedReLUMask(float x) {
    return (x > 0.0f && x < (float)(std::numeric_limits<int8_t>::max() / 64.0f));
}

constexpr float clippedReLUGrad(float x) {
    return clippedReLUMask(x) ? 1.0f : 0.0f;
}

namespace Quantization {
    struct FakeQuantization {
        /**
         * @brief Quantisierungsfunktion,
         * die einen Wert auf einen darstellbaren Wert im quantisierten Format rundet.
         * 
         * @param x Der Wert.
         * @param minVal Das Minimum des Intervalls.
         * @param maxVal Das Maximum des Intervalls.
         * @param scale Der Skalierungsfaktor der Quantisierung.
         */
        constexpr float operator()(float x, float minVal, float maxVal, float scale = 64.0f) const {
            return std::clamp(std::round(x * scale) / scale, minVal, maxVal);
        }
    };

    struct Identity {
        /**
         * @brief Identitätsfunktion, die als Quantisierungsfunktion verwendet werden kann,
         * wenn keine Quantisierung durchgeführt werden soll.
         */
        constexpr float operator()(float x, float, float, float = 64.0f) const {
            return x;
        }
    };
}

template <typename Q>
NNUE::NetworkActivations NNUE::MasterWeights::forwardImpl(const Board& board, Q q) const {
    constexpr size_t SUBNET_SIZE = NNUE::Network::SINGLE_SUBNET_SIZE;

    NNUE::NetworkActivations activations;

    int color = board.getSideToMove();
    int oppColor = color ^ COLOR_MASK;

    // Berechnung der HalfKP-Features
    Array<int, 68> activeFeatures = NNUE::getHalfKPFeatures(board, color);
    Array<int, 68> oppActiveFeatures = NNUE::getHalfKPFeatures(board, oppColor);

    // Biases
    for(size_t i = 0; i < SUBNET_SIZE; i++) {
        float qBias = q(exactHalfKPBiases[i],
                        NNUE::MasterWeights::HALF_KP_MIN,
                        NNUE::MasterWeights::HALF_KP_MAX);
        activations.halfKPOutputs[i] = qBias;
        activations.halfKPOutputs[i + SUBNET_SIZE] = qBias;
    }

    // Gewichte
    for(int feature : activeFeatures)
        for(size_t i = 0; i < SUBNET_SIZE; i++)
            activations.halfKPOutputs[i] += q(exactHalfKP[feature * SUBNET_SIZE + i],
                                                NNUE::MasterWeights::HALF_KP_MIN,
                                                NNUE::MasterWeights::HALF_KP_MAX);

    for(int feature : oppActiveFeatures)
        for(size_t i = 0; i < SUBNET_SIZE; i++)
            activations.halfKPOutputs[i + SUBNET_SIZE] += q(exactHalfKP[feature * SUBNET_SIZE + i],
                                                            NNUE::MasterWeights::HALF_KP_MIN,
                                                            NNUE::MasterWeights::HALF_KP_MAX);

    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[0]; i++)
        activations.halfKPOutputs[i] = clippedReLU(activations.halfKPOutputs[i]);

    // Berechnung der Dense-Layer-Ausgaben

    // Biases
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[1]; i++)
        activations.denseLayerOutputs[0][i] = q(exactDenseLayerBiases[0][i],
                                    NNUE::MasterWeights::DENSE_BIAS_MIN,
                                    NNUE::MasterWeights::DENSE_BIAS_MAX, 64.0f * 64.0f);

    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[1]; i++) {
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[0]; j++)
            activations.denseLayerOutputs[0][i] +=
                q(exactDenseLayerWeights[0][i * NNUE::Network::LAYER_SIZES[0] + j], NNUE::MasterWeights::DENSE_WEIGHT_MIN, NNUE::MasterWeights::DENSE_WEIGHT_MAX) *
                activations.halfKPOutputs[j];

        activations.denseLayerOutputs[0][i] = clippedReLU(activations.denseLayerOutputs[0][i]);
        activations.denseLayerOutputs[0][i] = q(activations.denseLayerOutputs[0][i], 0.0f, 127.0f / 64.0f, 64.0f);
    }

    // Biases
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[2]; i++)
        activations.denseLayerOutputs[1][i] = q(exactDenseLayerBiases[1][i], NNUE::MasterWeights::DENSE_BIAS_MIN, NNUE::MasterWeights::DENSE_BIAS_MAX, 64.0f * 64.0f);

    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[2]; i++) {
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[1]; j++)
            activations.denseLayerOutputs[1][i] += q(exactDenseLayerWeights[1][i * NNUE::Network::LAYER_SIZES[1] + j],
                                            NNUE::MasterWeights::DENSE_WEIGHT_MIN,
                                            NNUE::MasterWeights::DENSE_WEIGHT_MAX) *
                                            activations.denseLayerOutputs[0][j];

        activations.denseLayerOutputs[1][i] = clippedReLU(activations.denseLayerOutputs[1][i]);
        activations.denseLayerOutputs[1][i] = q(activations.denseLayerOutputs[1][i], 0.0f, 127.0f / 64.0f, 64.0f);
    }

    // Biases
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[3]; i++)
        activations.denseLayerOutputs[2][i] = q(exactDenseLayerBiases[2][i],
                                    NNUE::MasterWeights::DENSE_BIAS_MIN,
                                    NNUE::MasterWeights::DENSE_BIAS_MAX, 64.0f * 64.0f);

    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[3]; i++) {
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[2]; j++)
            activations.denseLayerOutputs[2][i] += q(exactDenseLayerWeights[2][i * NNUE::Network::LAYER_SIZES[2] + j],
                                        NNUE::MasterWeights::DENSE_WEIGHT_MIN,
                                        NNUE::MasterWeights::DENSE_WEIGHT_MAX) *
                                        activations.denseLayerOutputs[1][j];
    }

    return activations;
}

template <typename Q>
NNUE::Gradients NNUE::MasterWeights::backwardImpl(const Board& board, const NetworkActivations& activations, float outputGrad, Q q) const {
    Gradients gradients;
    std::array<float, NNUE::Network::LAYER_SIZES[0]> halfKPGrad{};

    // Layer 3
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[3]; i++) {
        gradients.denseLayerBiases[2][i] = outputGrad;

        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[2]; j++)
            gradients.denseLayerWeights[2][i * NNUE::Network::LAYER_SIZES[2] + j] = outputGrad * activations.denseLayerOutputs[1][j];
    }

    // Layer 2
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[2]; i++) {
        if(!clippedReLUMask(activations.denseLayerOutputs[1][i]))
            continue;

        float grad = 0.0f;

        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[3]; j++)
            grad += q(exactDenseLayerWeights[2][j * NNUE::Network::LAYER_SIZES[2] + i],
                        NNUE::MasterWeights::DENSE_WEIGHT_MIN,
                        NNUE::MasterWeights::DENSE_WEIGHT_MAX) * gradients.denseLayerBiases[2][j];

        gradients.denseLayerBiases[1][i] = grad;

        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[1]; j++)
            gradients.denseLayerWeights[1][i * NNUE::Network::LAYER_SIZES[1] + j] = grad * activations.denseLayerOutputs[0][j];
    }

    // Layer 1
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[1]; i++) {
        if(!clippedReLUMask(activations.denseLayerOutputs[0][i]))
            continue;

        float grad = 0.0f;

        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[2]; j++)
            grad += q(exactDenseLayerWeights[1][j * NNUE::Network::LAYER_SIZES[1] + i],
                        NNUE::MasterWeights::DENSE_WEIGHT_MIN,
                        NNUE::MasterWeights::DENSE_WEIGHT_MAX) * gradients.denseLayerBiases[1][j];

        gradients.denseLayerBiases[0][i] = grad;

        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[0]; j++)
            gradients.denseLayerWeights[0][i * NNUE::Network::LAYER_SIZES[0] + j] = grad * activations.halfKPOutputs[j];
    }

    // Gradient für die Ausgaben der HalfKP-Schicht: dL/d(halfKPOut)
    for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[0]; j++) {
        if(!clippedReLUMask(activations.halfKPOutputs[j]))
            continue;

        float grad = 0.0f;

        for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[1]; i++)
            grad += q(exactDenseLayerWeights[0][i * NNUE::Network::LAYER_SIZES[0] + j],
                        NNUE::MasterWeights::DENSE_WEIGHT_MIN,
                        NNUE::MasterWeights::DENSE_WEIGHT_MAX) * gradients.denseLayerBiases[0][i];

        halfKPGrad[j] = grad;
    }

    // HalfKP-Layer
    // Berechnung der HalfKP-Features
    int color = board.getSideToMove();
    int oppColor = color ^ COLOR_MASK;
    Array<int, 68> activeFeatures = NNUE::getHalfKPFeatures(board, color);
    Array<int, 68> oppActiveFeatures = NNUE::getHalfKPFeatures(board, oppColor);

    // Bias ist in beiden Subnetzen derselbe Vektor (256): Gradienten beider Perspektiven aufsummieren.
    for(size_t i = 0; i < NNUE::Network::SINGLE_SUBNET_SIZE; i++)
        gradients.halfKPBiases[i] = halfKPGrad[i] + halfKPGrad[i + NNUE::Network::SINGLE_SUBNET_SIZE];

    for(int feature : activeFeatures) {
        for(size_t i = 0; i < NNUE::Network::SINGLE_SUBNET_SIZE; i++)
            gradients.halfKPWeights[feature * NNUE::Network::SINGLE_SUBNET_SIZE + i] += halfKPGrad[i];
    }

    for(int feature : oppActiveFeatures) {
        for(size_t i = 0; i < NNUE::Network::SINGLE_SUBNET_SIZE; i++)
            gradients.halfKPWeights[feature * NNUE::Network::SINGLE_SUBNET_SIZE + i] += halfKPGrad[i + NNUE::Network::SINGLE_SUBNET_SIZE];
    }

    return gradients;
}

NNUE::NetworkActivations NNUE::MasterWeights::forward(const Board& board, bool fakeQuantization) const {
    if(fakeQuantization)
        return forwardImpl(board, Quantization::FakeQuantization{});
    else
        return forwardImpl(board, Quantization::Identity{});
}

NNUE::Gradients NNUE::MasterWeights::backward(const Board& board, const NetworkActivations& activations, float outputGrad, bool fakeQuantization) const {
    if(fakeQuantization)
        return backwardImpl(board, activations, outputGrad, Quantization::FakeQuantization{});
    else
        return backwardImpl(board, activations, outputGrad, Quantization::Identity{});
}