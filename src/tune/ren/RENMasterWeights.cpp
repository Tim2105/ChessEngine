#include "tune/ren/RENMasterWeights.h"

#include <cmath>

REN::Network* REN::MasterWeights::toNetwork() const {
    REN::Network* network = new REN::Network;

    // HalfKAv2_hmLayer
    auto& halfKAv2_hm = network->getHalfKAv2_hmLayer();

    int16_t* biasPtrHalfKP = (int16_t*)halfKAv2_hm.getBiasPtr();
    for(size_t i = 0; i < REN::HALF_KA_OUTPUT_SIZE; i++)
        biasPtrHalfKP[i] = (int16_t)(std::round(halfKAv2Layer.bias(i) * 128.0f));

    for(size_t i = 0; i < NNUE::INPUT_SIZE; i++) {
        int16_t* weightPtr = (int16_t*)halfKAv2_hm.getWeightPtr(i);
        for(size_t j = 0; j < REN::REN_SIZE / 2; j++)
            weightPtr[j] = (int16_t)(std::round(halfKAv2Layer.weights(i, j) * 128.0f));
    }

    // SparseRENLayer
    auto& renLayerNet = network->getRENLayer();

    int32_t* biasPtrREN = (int32_t*)renLayerNet.getBiasPtr();
    for(size_t i = 0; i < REN::REN_SIZE; i++)
        biasPtrREN[i] = (int32_t)(std::round(renLayer.bias(i) * 128.0f * 128.0f));

    for(size_t block = 0; block < REN::SQRT_REN_SIZE; block++) {
        for(size_t in = 0; in < REN::SQRT_REN_SIZE; in++) {
            int8_t* weightPtr = (int8_t*)renLayerNet.getWeightPtr(block, in);
            for(size_t out = 0; out < REN::SQRT_REN_SIZE; out++)
                weightPtr[out] = (int8_t)(std::round(renLayer.transform[block](in, out) * 128.0f));
        }
    }

    // Output Layer
    auto& outputLayerNet = network->getOutputLayer();

    int32_t* biasPtrOutput = (int32_t*)outputLayerNet.getBiasPtr();
    biasPtrOutput[0] = (int32_t)(std::round(outputLayer.bias(0) * 128.0f * 128.0f));

    int8_t* weightPtrOutput = (int8_t*)outputLayerNet.getWeightPtr(0);
    for(size_t j = 0; j < REN::REN_SIZE; j++)
        weightPtrOutput[j] = (int8_t)(std::round(outputLayer.weights(0, j) * 128.0f));

    return network;
}

REN::NetworkActivations REN::MasterWeights::forward(const Board& board, bool fakeQuantization,
    size_t maxIterations, float tol) const {

    NetworkActivations activations;

    activations.halfKPActivations = halfKAv2Layer.forward(board, fakeQuantization);
    activations.renActivations = renLayer.forward(activations.halfKPActivations.output, fakeQuantization, maxIterations, tol);
    activations.outputLayerActivations = outputLayer.forward(activations.renActivations.h_opt, fakeQuantization);

    return activations;
}

REN::Gradients REN::MasterWeights::backward(const Board& board, const NetworkActivations& activations, const ML::DenseLayer::ForwardResult& encActivations,
    float outputGrad, float encOutputGrad, bool fakeQuantization) const {

    Gradients gradients;

    // Pfad A: outputGrad -> outputLayer -> renLayer -> halfKAv2Layer
    ML::Vector mainGradVec(1);
    mainGradVec(0) = outputGrad;

    gradients.outputLayerGradients = outputLayer.backward(activations.renActivations.h_opt,
        activations.outputLayerActivations, mainGradVec, fakeQuantization);

    gradients.renGradients = renLayer.backward(activations.renActivations,
        gradients.outputLayerGradients.inputGrad, fakeQuantization);

    // Pfad B: encOutputGrad -> outputLayer -> halfKAv2Layer
    ML::Vector encGradVec(1);
    encGradVec(0) = encOutputGrad;

    ML::DenseLayer::Gradients encOutputLayerGradients = outputLayer.backward(activations.halfKPActivations.output,
        encActivations, encGradVec, fakeQuantization);

    gradients.outputLayerGradients.bias += encOutputLayerGradients.bias;
    gradients.outputLayerGradients.weights += encOutputLayerGradients.weights;

    // Gradienten für Encoder zusammenführen
    gradients.renGradients.inputGrad += encOutputLayerGradients.inputGrad;

    gradients.halfKAGradients = halfKAv2Layer.backward(board,
        activations.halfKPActivations, gradients.renGradients.inputGrad, fakeQuantization);

    return gradients;
}