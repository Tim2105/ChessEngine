#include "tune/ml/DenseLayer.h"
#include "tune/ml/Quantization.h"

using namespace ML;

template <bool UseActivation, typename Q>
DenseLayer::ForwardResult DenseLayer::forwardImpl(const Vector& input, Q q) const {
    ForwardResult output(outputSize);

    for(size_t i = 0; i < outputSize; i++) {
        float sum = q(bias(i));
        for(size_t j = 0; j < inputSize; j++)
            sum += q(weights(i, j)) * input(j);

        output.preActivations(i) = sum;

        if constexpr (UseActivation)
            sum = clippedReLU(sum, Q::CLIPPED_RELU_MAX);

        output.output(i) = sum;
    }

    return output;
}

template <bool UseActivation, typename Q>
DenseLayer::Gradients DenseLayer::backwardImpl(const Vector& input,
    const ForwardResult& forwardResult, const Vector& outputGrad, Q q) const {

    Gradients grads(inputSize, outputSize);

    for(size_t i = 0; i < outputSize; i++) {
        if constexpr (UseActivation) {
            if(!clippedReLUDerivative(forwardResult.preActivations(i), Q::CLIPPED_RELU_MAX))
                continue; // Gradient ist 0
        }

        const float gradOutput = outputGrad(i);
        grads.bias(i) = gradOutput;

        for(size_t j = 0; j < inputSize; j++) {
            grads.weights(i, j) = gradOutput * input(j);
            grads.inputGrad(j) += gradOutput * q(weights(i, j));
        }
    }

    return grads;
}

DenseLayer::ForwardResult DenseLayer::forward(const Vector& input, bool fakeQuant) const {
    if(fakeQuant) {
        if(useActivation)
            return forwardImpl<true>(input, ML::FakeQuantizationI8());
        else
            return forwardImpl<false>(input, ML::FakeQuantizationI8());
    } else {
        if(useActivation)
            return forwardImpl<true>(input, ML::Identity());
        else
            return forwardImpl<false>(input, ML::Identity());
    }
}

DenseLayer::Gradients DenseLayer::backward(const Vector& input, const ForwardResult& forwardResult, const Vector& outputGrad, bool fakeQuant) const {
    if(fakeQuant) {
        if(useActivation)
            return backwardImpl<true>(input, forwardResult, outputGrad, ML::FakeQuantizationI8());
        else
            return backwardImpl<false>(input, forwardResult, outputGrad, ML::FakeQuantizationI8());
    } else {
        if(useActivation)
            return backwardImpl<true>(input, forwardResult, outputGrad, ML::Identity());
        else
            return backwardImpl<false>(input, forwardResult, outputGrad, ML::Identity());
    }
}