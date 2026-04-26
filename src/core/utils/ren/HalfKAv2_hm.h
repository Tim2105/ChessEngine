#ifndef REN_HALFKAV2_HM_H
#define REN_HALFKAV2_HM_H

#include <unordered_map>

#include "core/chess/Board.h"
#include "core/utils/nnue/NNUEUtils.h"
#include "core/utils/ren/Math.h"

namespace REN {
    /**
     * @brief Ein HalfKAv2_hmLayer mit Gewichten in voller Präzision.
     */
    class HalfKAv2_hmLayer {
        public:
            struct Gradients {
                // Map (Index -> Gewicht), da die meisten Gradienten 0 sind
                std::unordered_map<size_t, float> weights;
                REN::Vector bias;

                inline Gradients(size_t outputSize) : bias(outputSize) {}
            };

            struct ForwardResult {
                Vector preActivations;
                Vector output;

                inline ForwardResult(size_t s) : preActivations(s), output(s) {}
            };

            REN::Matrix weights;
            REN::Vector bias;
            constexpr static size_t INPUT_SIZE = NNUE::INPUT_SIZE;
            size_t subnetSize;

            inline HalfKAv2_hmLayer(size_t outputSize) :
                weights(INPUT_SIZE, outputSize / 2), bias(outputSize / 2), subnetSize(outputSize / 2) {

                assert(outputSize % 2 == 0);
            }

            ForwardResult forward(const Board& board, bool fakeQuant) const;

            Gradients backward(const Board& board, const ForwardResult& forwardResult, const Vector& outputGrad, bool fakeQuant) const;

        private:
            template <typename Q>
            ForwardResult forwardImpl(const Board& board, Q q) const;

            template <typename Q>
            Gradients backwardImpl(const Board& board, const ForwardResult& forwardResult, const Vector& outputGrad) const;
    };          
}

#endif