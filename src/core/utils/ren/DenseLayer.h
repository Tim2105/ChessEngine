#ifndef REN_DENSE_LAYER_H
#define REN_DENSE_LAYER_H

#include "core/utils/ren/Math.h"

namespace REN {
    class DenseLayer {
        public:
            struct Gradients {
                REN::Matrix weights;
                REN::Vector bias;
                REN::Vector inputGrad;

                inline Gradients(size_t inputSize, size_t outputSize) :
                    weights(inputSize, outputSize), bias(outputSize), inputGrad(inputSize) {}
            };

            struct ForwardResult {
                Vector preActivations;
                Vector output;

                inline ForwardResult(size_t s) : preActivations(s), output(s) {}
            };

            REN::Matrix weights;
            REN::Vector bias;
            size_t inputSize;
            size_t outputSize;
            bool useActivation;

            inline DenseLayer(size_t inputSize, size_t outputSize, bool useActivation = true) :
                weights(outputSize, inputSize), bias(outputSize), inputSize(inputSize),
                outputSize(outputSize), useActivation(useActivation) {}

            ForwardResult forward(const Vector& input, bool fakeQuant) const;
            Gradients backward(const Vector& input, const ForwardResult& forwardResult, const Vector& outputGrad, bool fakeQuant) const;

        private:
            template <bool UseActivation, typename Q>
            ForwardResult forwardImpl(const Vector& input, Q q) const;

            template <bool UseActivation, typename Q>
            Gradients backwardImpl(const Vector& input, const ForwardResult& forwardResult, const Vector& outputGrad, Q q) const;
    };
}

#endif