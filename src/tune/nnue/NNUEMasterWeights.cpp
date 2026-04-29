#include "tune/nnue/NNUEMasterWeights.h"
#include "core/utils/nnue/NNUEUtils.h"

#include <cmath>

NNUE::MasterWeights::MasterWeights(const NNUE::Network& network) {
    const auto& halfKP = network.getHalfKPLayer();

    // Biases, dequantisiert
    for(size_t i = 0; i < NNUE::Network::SINGLE_SUBNET_SIZE; i++)
        halfKPLayer.bias(i) = halfKP.getBias(i) / 128.0f;

    // Gewichte, dequantisiert
    for(size_t i = 0; i < NNUE::Network::INPUT_SIZE; i++) {
        for(size_t j = 0; j < NNUE::Network::SINGLE_SUBNET_SIZE; j++)
            halfKPLayer.weights(i, j) = halfKP.getWeight(i, j) / 128.0f;
    }

    const auto& layer1 = network.getLayer1();
    const auto& layer2 = network.getLayer2();
    const auto& layer3 = network.getLayer3();

    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[1]; i++) {
        denseLayers[0].bias(i) = layer1.getBias(i) / (128.0f * 128.0f);

        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[0]; j++)
            denseLayers[0].weights(i, j) = layer1.getWeight(j, i) / 128.0f;
    }

    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[2]; i++) {
        denseLayers[1].bias(i) = layer2.getBias(i) / (128.0f * 128.0f);

        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[1]; j++)
            denseLayers[1].weights(i, j) = layer2.getWeight(j, i) / 128.0f;
    }

    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[3]; i++) {
        denseLayers[2].bias(i) = layer3.getBias(i) / (128.0f * 128.0f);

        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[2]; j++)
            denseLayers[2].weights(i, j) = layer3.getWeight(j, i) / 128.0f;
    }
}

NNUE::Network* NNUE::MasterWeights::toNetwork() const {
    NNUE::Network* network = new NNUE::Network;

    auto& halfKP = network->getHalfKPLayer();

    int16_t* biasPtrHalfKP = (int16_t*)halfKP.getBiasPtr();
    for(size_t i = 0; i < NNUE::Network::SINGLE_SUBNET_SIZE; i++)
        biasPtrHalfKP[i] = (int16_t)(std::round(halfKPLayer.bias(i) * 128.0f));

    for(size_t i = 0; i < NNUE::Network::INPUT_SIZE; i++) {
        int16_t* weightPtr = (int16_t*)halfKP.getWeightPtr(i);
        for(size_t j = 0; j < NNUE::Network::SINGLE_SUBNET_SIZE; j++)
            weightPtr[j] = (int16_t)(std::round(halfKPLayer.weights(i, j) * 128.0f));
    }

    auto& layer1 = network->getLayer1();
    auto& layer2 = network->getLayer2();
    auto& layer3 = network->getLayer3();

    int32_t* biasPtr = (int32_t*)layer1.getBiasPtr();
    int8_t* weightPtr;
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[1]; i++) {
        biasPtr[i] = (int32_t)(std::round(denseLayers[0].bias(i) * 128.0f * 128.0f));

        weightPtr = (int8_t*)layer1.getWeightPtr(i);
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[0]; j++)
            weightPtr[j] = (int8_t)(std::round(denseLayers[0].weights(i, j) * 128.0f));
    }

    biasPtr = (int32_t*)layer2.getBiasPtr();
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[2]; i++) {
        biasPtr[i] = (int32_t)(std::round(denseLayers[1].bias(i) * 128.0f * 128.0f));

        weightPtr = (int8_t*)layer2.getWeightPtr(i);
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[1]; j++)
            weightPtr[j] = (int8_t)(std::round(denseLayers[1].weights(i, j) * 128.0f));
    }

    biasPtr = (int32_t*)layer3.getBiasPtr();
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[3]; i++) {
        biasPtr[i] = (int32_t)(std::round(denseLayers[2].bias(i) * 128.0f * 128.0f));

        weightPtr = (int8_t*)layer3.getWeightPtr(i);
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[2]; j++)
            weightPtr[j] = (int8_t)(std::round(denseLayers[2].weights(i, j) * 128.0f));
    }

    return network;
}

NNUE::NetworkActivations NNUE::MasterWeights::forward(const Board& board, bool fakeQuantization) const {
    NNUE::NetworkActivations activations;

    activations.halfKPActivations = halfKPLayer.forward(board, fakeQuantization);
    activations.denseLayerOutputs[0] = denseLayers[0].forward(activations.halfKPActivations.output, fakeQuantization);
    activations.denseLayerOutputs[1] = denseLayers[1].forward(activations.denseLayerOutputs[0].output, fakeQuantization);
    activations.denseLayerOutputs[2] = denseLayers[2].forward(activations.denseLayerOutputs[1].output, fakeQuantization);

    return activations;
}

NNUE::Gradients NNUE::MasterWeights::backward(const Board& board, const NetworkActivations& activations, float outputGrad, bool fakeQuantization) const {
    ML::Vector outputGradVec(1);
    outputGradVec(0) = outputGrad;
    Gradients gradients;

    gradients.denseLayerGradients[2] = denseLayers[2].backward(activations.denseLayerOutputs[1].output,
        activations.denseLayerOutputs[2], outputGradVec, fakeQuantization);

    gradients.denseLayerGradients[1] = denseLayers[1].backward(activations.denseLayerOutputs[0].output,
        activations.denseLayerOutputs[1], gradients.denseLayerGradients[2].inputGrad, fakeQuantization);

    gradients.denseLayerGradients[0] = denseLayers[0].backward(activations.halfKPActivations.output,
        activations.denseLayerOutputs[0], gradients.denseLayerGradients[1].inputGrad, fakeQuantization);

    gradients.halfKAGradients = halfKPLayer.backward(board,
        activations.halfKPActivations, gradients.denseLayerGradients[0].inputGrad, fakeQuantization);

    return gradients;
}