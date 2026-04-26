#include "core/utils/ren/DenseLayer.h"
#include "core/utils/ren/Quantization.h"

using namespace REN;

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
            return forwardImpl<true>(input, Quantization::FakeQuantizationI8());
        else
            return forwardImpl<false>(input, Quantization::FakeQuantizationI8());
    } else {
        if(useActivation)
            return forwardImpl<true>(input, Quantization::Identity());
        else
            return forwardImpl<false>(input, Quantization::Identity());
    }
}

DenseLayer::Gradients DenseLayer::backward(const Vector& input, const ForwardResult& forwardResult, const Vector& outputGrad, bool fakeQuant) const {
    if(fakeQuant) {
        if(useActivation)
            return backwardImpl<true>(input, forwardResult, outputGrad, Quantization::FakeQuantizationI8());
        else
            return backwardImpl<false>(input, forwardResult, outputGrad, Quantization::FakeQuantizationI8());
    } else {
        if(useActivation)
            return backwardImpl<true>(input, forwardResult, outputGrad, Quantization::Identity());
        else
            return backwardImpl<false>(input, forwardResult, outputGrad, Quantization::Identity());
    }
}