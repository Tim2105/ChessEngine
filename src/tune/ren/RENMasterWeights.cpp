#include "tune/ren/RENMasterWeights.h"

#include <cmath>

REN::NetworkActivations REN::MasterWeights::forward(const Board& board, bool fakeQuantization) const {
    NetworkActivations activations;

    activations.halfKPActivations = halfKAv2Layer.forward(board, fakeQuantization);
    activations.renActivations = renLayer.forward(activations.halfKPActivations.output, fakeQuantization);
    activations.outputLayerActivations = outputLayer.forward(activations.renActivations.h_opt, fakeQuantization);

    return activations;
}

REN::Gradients REN::MasterWeights::backward(const Board& board, const NetworkActivations& activations, float outputGrad, bool fakeQuantization) const {
    ML::Vector outputGradVec(1);
    outputGradVec(0) = outputGrad;
    Gradients gradients;

    gradients.outputLayerGradients = outputLayer.backward(activations.renActivations.h_opt,
        activations.outputLayerActivations, outputGradVec, fakeQuantization);

    gradients.renGradients = renLayer.backward(activations.renActivations,
        gradients.outputLayerGradients.inputGrad, fakeQuantization);

    gradients.halfKAGradients = halfKAv2Layer.backward(board,
        activations.halfKPActivations, gradients.renGradients.inputGrad, fakeQuantization);

    return gradients;
}