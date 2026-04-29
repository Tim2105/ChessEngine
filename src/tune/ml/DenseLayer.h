#ifndef ML_DENSE_LAYER_H
#define ML_DENSE_LAYER_H

#include "tune/ml/Math.h"

namespace ML {
    class DenseLayer {
        public:
            struct Gradients {
                Matrix weights;
                Vector bias;
                Vector inputGrad;

                inline Gradients(size_t inputSize, size_t outputSize) :
                    weights(inputSize, outputSize), bias(outputSize), inputGrad(inputSize) {}
            };

            struct ForwardResult {
                Vector preActivations;
                Vector output;

                inline ForwardResult(size_t s) : preActivations(s), output(s) {}
            };

            Matrix weights;
            Vector bias;
            size_t inputSize;
            size_t outputSize;
            bool useActivation;

            inline DenseLayer(size_t inputSize, size_t outputSize, bool useActivation = true) :
                weights(inputSize, outputSize), bias(outputSize), inputSize(inputSize),
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