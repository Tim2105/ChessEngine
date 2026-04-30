#ifndef REN_REN_H
#define REN_REN_H

#include "tune/ml/DenseLayer.h"
#include "tune/ml/HalfKAv2_hm.h"
#include "tune/ren/RENLayer.h"

namespace REN {
    static constexpr size_t REN_SIZE = 128;

    struct Gradients {
        ML::HalfKAv2_hmLayer::Gradients halfKAGradients{REN_SIZE};
        RENLayer::Gradients renGradients{REN_SIZE};
        ML::DenseLayer::Gradients outputLayerGradients{REN_SIZE, 1};
    };

    struct NetworkActivations {
        ML::HalfKAv2_hmLayer::ForwardResult halfKPActivations{REN_SIZE};
        RENLayer::ForwardResult renActivations{REN_SIZE};
        ML::DenseLayer::ForwardResult outputLayerActivations{1};

        inline float output() const noexcept {
            return outputLayerActivations.output(0);
        }
    };

    struct MasterWeights {
        ML::HalfKAv2_hmLayer halfKAv2Layer{REN_SIZE};
        RENLayer renLayer{REN_SIZE};
        ML::DenseLayer outputLayer{REN_SIZE, 1, false};
        
        inline MasterWeights() = default;

        NetworkActivations forward(const Board& board, bool fakeQuantization) const;
        Gradients backward(const Board& board, const NetworkActivations& activations, float outputGrad, bool fakeQuantization) const;
    };
}


#endif