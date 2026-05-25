#include "tune/ren/RENMasterWeights.h"

#include <cmath>

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