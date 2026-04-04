#include "tune/nnue/Train.h"

using namespace Train;

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
        biasPtr[i] = (int32_t)(std::round(exactDenseLayerBiases[0][i] * 64.0f));

        weightPtr = (int8_t*)layer1.getWeightPtr(i);
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[0]; j++)
            weightPtr[j] = (int8_t)(std::round(exactDenseLayerWeights[0][j * NNUE::Network::LAYER_SIZES[1] + i] * 64.0f));
    }

    biasPtr = (int32_t*)layer2.getBiasPtr();
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[2]; i++) {
        biasPtr[i] = (int32_t)(std::round(exactDenseLayerBiases[1][i] * 64.0f));

        weightPtr = (int8_t*)layer2.getWeightPtr(i);
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[1]; j++)
            weightPtr[j] = (int8_t)(std::round(exactDenseLayerWeights[1][j * NNUE::Network::LAYER_SIZES[2] + i] * 64.0f));
    }

    biasPtr = (int32_t*)layer3.getBiasPtr();
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[3]; i++) {
        biasPtr[i] = (int32_t)(std::round(exactDenseLayerBiases[2][i] * 64.0f));

        weightPtr = (int8_t*)layer3.getWeightPtr(i);
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[2]; j++)
            weightPtr[j] = (int8_t)(std::round(exactDenseLayerWeights[2][j * NNUE::Network::LAYER_SIZES[3] + i] * 64.0f));
    }

    return network;
}