#include "tune/ml/HalfKAv2_hm.h"
#include "tune/ml/Quantization.h"

using namespace ML;

template <typename Q>
HalfKAv2_hmLayer::ForwardResult HalfKAv2_hmLayer::forwardImpl(const Board& board, Q q) const {
    HalfKAv2_hmLayer::ForwardResult result(subnetSize * 2);

    int color = board.getSideToMove();

    Array<int, 68> activeFeatures = NNUE::getHalfKPFeatures(board, color);
    Array<int, 68> oppActiveFeatures = NNUE::getHalfKPFeatures(board, color ^ COLOR_MASK);

    // Kopiere Biases
    for(size_t i = 0; i < subnetSize; i++) {
        float qBias = q(bias(i));
        result.preActivations(i) = qBias;
        result.preActivations(i + subnetSize) = qBias;
    }

    // Subnetz des Spielers am Zug
    for(int feature : activeFeatures)
        for(size_t i = 0; i < subnetSize; i++)
            result.preActivations(i) += q(weights(feature, i));

    // Subnetz des Gegners
    for(int feature : oppActiveFeatures)
        for(size_t i = 0; i < subnetSize; i++)
            result.preActivations(i + subnetSize) += q(weights(feature, i));

    // Aktivierungsfunktion (clipped ReLU)
    for(size_t i = 0; i < subnetSize * 2; i++)
        result.output(i) = clippedReLU(result.preActivations(i), Q::CLIPPED_RELU_MAX);

    return result;
}

template <typename Q>
HalfKAv2_hmLayer::Gradients HalfKAv2_hmLayer::backwardImpl(const Board& board,
    const HalfKAv2_hmLayer::ForwardResult& forwardResult, const Vector& outputGrad) const {

    Gradients grads(subnetSize);

    Vector preActivationGrad(subnetSize * 2);
    for(size_t i = 0; i < subnetSize * 2; i++)
        preActivationGrad(i) = outputGrad(i) * (float)clippedReLUDerivative(forwardResult.preActivations(i), Q::CLIPPED_RELU_MAX);

    int color = board.getSideToMove();
    Array<int, 68> activeFeatures = NNUE::getHalfKPFeatures(board, color);
    Array<int, 68> oppActiveFeatures = NNUE::getHalfKPFeatures(board, color ^ COLOR_MASK);

    // Bias ist in beiden Subnetzen gleich, Gradienten addieren
    for(size_t i = 0; i < subnetSize; i++)
        grads.bias(i) = preActivationGrad(i) + preActivationGrad(i + subnetSize);

    // Betrachte nur Gewichte zu aktiven Features,
    // alle anderen sind 0

    for(int feature : activeFeatures)
        for(size_t i = 0; i < subnetSize; i++)
            grads.weights[feature * subnetSize + i] += preActivationGrad(i);

    for(int feature : oppActiveFeatures)
        for(size_t i = 0; i < subnetSize; i++)
            grads.weights[feature * subnetSize + i] += preActivationGrad(i + subnetSize);

    return grads;
}

HalfKAv2_hmLayer::ForwardResult HalfKAv2_hmLayer::forward(const Board& board, bool fakeQuant) const {
    if(fakeQuant)
        return forwardImpl(board, ML::FakeQuantizationI16());
    else
        return forwardImpl(board, ML::Identity());
}

HalfKAv2_hmLayer::Gradients HalfKAv2_hmLayer::backward(const Board& board,
    const HalfKAv2_hmLayer::ForwardResult& forwardResult, const Vector& outputGrad, bool fakeQuant) const {

    if(fakeQuant)
        return backwardImpl<ML::FakeQuantizationI16>(board, forwardResult, outputGrad);
    else
        return backwardImpl<ML::Identity>(board, forwardResult, outputGrad);
}