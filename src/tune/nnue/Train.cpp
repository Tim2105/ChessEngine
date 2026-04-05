#include "tune/nnue/Train.h"
#include "core/utils/nnue/NNUEUtils.h"

#include <algorithm>
#include <iostream>

using namespace Train;

TrainingSession trainingSession;

NNUE::Network* MasterWeights::toNetwork() const {
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

float NetworkActivations::compute(const MasterWeights& masterWeights, const Board& board) {
    constexpr size_t SUBNET_SIZE = NNUE::Network::SINGLE_SUBNET_SIZE;

    int color = board.getSideToMove();
    int oppColor = color ^ COLOR_MASK;

    // Berechnung der HalfKP-Features
    Array<int32_t, 63> activeFeatures = NNUE::getHalfKPFeatures(board, color);
    Array<int32_t, 63> oppActiveFeatures = NNUE::getHalfKPFeatures(board, oppColor);

    // Biases
    std::copy(masterWeights.exactHalfKPBiases.begin(), masterWeights.exactHalfKPBiases.end(), halfKPOutputs.begin());
    std::copy(masterWeights.exactHalfKPBiases.begin(), masterWeights.exactHalfKPBiases.end(), halfKPOutputs.begin() + SUBNET_SIZE);

    // Gewichte
    for(int32_t feature : activeFeatures)
        for(size_t i = 0; i < SUBNET_SIZE; i++)
            halfKPOutputs[i] += masterWeights.exactHalfKP[feature * SUBNET_SIZE + i];

    for(int32_t feature : oppActiveFeatures)
        for(size_t i = 0; i < SUBNET_SIZE; i++)
            halfKPOutputs[i + SUBNET_SIZE] += masterWeights.exactHalfKP[feature * SUBNET_SIZE + i];

    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[0]; i++)
        halfKPOutputs[i] = clippedReLU(halfKPOutputs[i]);

    // Berechnung der Dense-Layer-Ausgaben

    // Biases
    std::copy(masterWeights.exactDenseLayerBiases[0].begin(), masterWeights.exactDenseLayerBiases[0].end(), denseLayerOutputs[0].begin());

    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[1]; i++) {
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[0]; j++)
            denseLayerOutputs[0][i] += masterWeights.exactDenseLayerWeights[0][i * NNUE::Network::LAYER_SIZES[0] + j] * halfKPOutputs[j];

        denseLayerOutputs[0][i] = clippedReLU(denseLayerOutputs[0][i]);
    }

    // Biases
    std::copy(masterWeights.exactDenseLayerBiases[1].begin(), masterWeights.exactDenseLayerBiases[1].end(), denseLayerOutputs[1].begin());

    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[2]; i++) {
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[1]; j++)
            denseLayerOutputs[1][i] += masterWeights.exactDenseLayerWeights[1][i * NNUE::Network::LAYER_SIZES[1] + j] * denseLayerOutputs[0][j];

        denseLayerOutputs[1][i] = clippedReLU(denseLayerOutputs[1][i]);
    }

    // Biases
    std::copy(masterWeights.exactDenseLayerBiases[2].begin(), masterWeights.exactDenseLayerBiases[2].end(), denseLayerOutputs[2].begin());

    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[3]; i++) {
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[2]; j++)
            denseLayerOutputs[2][i] += masterWeights.exactDenseLayerWeights[2][i * NNUE::Network::LAYER_SIZES[2] + j] * denseLayerOutputs[1][j];
    }

    return denseLayerOutputs[2][0];
}