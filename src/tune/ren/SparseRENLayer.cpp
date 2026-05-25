#include "tune/ren/SparseRENLayer.h"

#include <algorithm>
#include <chrono>
#include <limits>
#include <iostream>

#include "tune/ml/Math.h"
#include "tune/ml/Quantization.h"

using namespace REN;

constexpr float sigmoid(float x) {
    return 1.0f / (1.0f + std::exp(-x));
}

constexpr float sigmoidDerivative(float x) {
    float s = sigmoid(x);
    return s * (1.0f - s);
}

void SparseRENLayer::constructTransform() {
    for(size_t n = 0; n < size; n++)
        constructTransformBlock(n);
}

void SparseRENLayer::constructTransformBlock(size_t block) {
    // W_r = -Q Q^T + diag((1 - epsilon) * tanh(gamma_raw))
    for(size_t i = 0; i < size; i++) {
        for(size_t j = 0; j < size; j++) {
            float sum = 0.0f;
            for(size_t k = 0; k < size; k++)
                sum += surrogateWeights.q[block](i, k) * surrogateWeights.q[block](j, k);

            float val = -sum;
            if(i == j)
                val += (1.0f - epsilon) * sigmoid(surrogateWeights.gammaRaw(block * size + i));

            transform[block](i, j) = val;
            transform[block](j, i) = val; // Symmetrie ausnutzen
        }
    }
}

void SparseRENLayer::normalize(float maxSpectralRadius) {
    for(size_t n = 0; n < size; n++) {
        float norm = blockSpectralRadius(n);
        if(norm > maxSpectralRadius) {
            float minGamma = 1.0f;
            for(size_t i = 0; i < size; i++)
                minGamma = std::min(minGamma, (1.0f - epsilon) * sigmoid(surrogateWeights.gammaRaw(n * size + i)));

            float scale = std::sqrt(maxSpectralRadius + minGamma) / std::sqrt(norm + minGamma);
            surrogateWeights.q[n] *= scale;
            constructTransformBlock(n);
        }
    }
}

float SparseRENLayer::spectralRadius(size_t maxIterations, float tol) const {
    // Berechne den Spektralradius als Maximum der Spektralradien der Blöcke
    float maxRadius = 0.0f;
    for(size_t n = 0; n < size; n++)
        maxRadius = std::max(maxRadius, transform[n].spectralRadius(maxIterations, tol));

    return maxRadius;
}

float SparseRENLayer::blockSpectralRadius(size_t block, size_t maxIterations, float tol) const {
    return transform[block].spectralRadius(maxIterations, tol);
}

template <typename Q>
ML::Vector SparseRENLayer::forwardImpl(const ML::Vector& h_t, const ML::Vector& h_0, Q q) const {
    ML::Vector result(size * size);

    for(size_t n = 0; n < size; n++) {
        for(size_t i = 0; i < size; i++) {
            float sum = h_0(n * size + i) + q(bias(n * size + i));
            for(size_t j = 0; j < size; j++)
                sum += q(transform[n](i, j)) * h_t(n * size + j);

            // Transponiert einfügen
            result(i * size + n) = sum;
        }
    }

    return result;
};

template <typename Q>
SparseRENLayer::Gradients SparseRENLayer::backwardImpl(const ML::Vector& h_opt, const ML::Vector& z_opt, const ML::Vector& inputGrad, float tol, Q q) const {
    Gradients grads(size);

    // System I - W_r^T * diag(clippedReLU'(z_opt)) aufstellen
    size_t systemSize = size * size;
    ML::Matrix system(systemSize);
    for(size_t n = 0; n < size; n++) {
        for(size_t i = 0; i < size; i++) {
            for(size_t j = 0; j < size; j++) {
                float val = q(transform[n](j, i)) * (float)clippedReLUDerivative(z_opt(j * size + n), Q::CLIPPED_RELU_MAX);
                size_t idx_i = i * size + n;
                size_t idx_j = n * size + j; // Transponiert indexieren
                system(idx_i, idx_j) = (i == j ? 1.0f : 0.0f) - val;
            }
        }
    }

    constexpr size_t M = 50; // Krylov-Unterraumgröße für GMRES
    size_t restarts = (size_t)std::ceil(systemSize / (double)M); // Maximale Anzahl der Restarts
    ML::Vector v = ML::gmresRestarted(system, inputGrad, M, restarts, tol);

    // Berechne v' = diag(clippedReLU'(z_opt)) * v
    for(size_t i = 0; i < systemSize; i++)
        v(i) *= (float)clippedReLUDerivative(z_opt(i), Q::CLIPPED_RELU_MAX);

    // Gradient für Ausgabe
    grads.inputGrad = v;

    // Gradient für bias und gammaRaw
    for(size_t n = 0; n < size; n++) {
        for(size_t i = 0; i < size; i++) {
            size_t t_idx = i * size + n;
            size_t idx = n * size + i;
            grads.bias(idx) = v(t_idx);
            grads.gammaRaw(idx) = v(t_idx) * h_opt(t_idx) * (1.0f - epsilon) * sigmoidDerivative(surrogateWeights.gammaRaw(idx));
        }
    }

    // Gradient für q
    // g_q = -(v h_opt^T + h_opt v^T) * q
    
    for(size_t n = 0; n < size; n++) {
        // Berechne Hilfsvektoren h_opt^T * q und v^T * q
        ML::Vector hq(size), vq(size);
        for(size_t i = 0; i < size; i++) {
            float hq_i = 0.0f, vq_i = 0.0f;
            for(size_t j = 0; j < size; j++) {
                size_t idx = j * size + n;
                hq_i += h_opt(idx) * surrogateWeights.q[n](j, i);
                vq_i += v(idx) * surrogateWeights.q[n](j, i);
            }
            hq(i) = hq_i;
            vq(i) = vq_i;
        }

        // Addiere äußere Produkte um g_q zu berechnen
        for(size_t i = 0; i < size; i++)
            for(size_t j = 0; j < size; j++) {
                size_t idx = i * size + n;
                grads.q[n](i, j) = -(v(idx) * hq(j) + h_opt(idx) * vq(j));
            }
    }

    return grads;
}

template <typename Q>
inline float activate(float z_i, float h_i, float alpha) {
    float act = clippedReLU(z_i, Q::CLIPPED_RELU_MAX);
    return (1.0f - alpha) * h_i + alpha * act;
}

SparseRENLayer::ForwardResult SparseRENLayer::forward(const ML::Vector& h_0, bool fakeQuant, size_t maxIterations, float tol, float alpha, bool stepSizeBacktracking) const {
    ForwardResult result(h_0);
    result.residual = std::numeric_limits<float>::max();

    float residual;
    for(size_t iter = 0; iter < maxIterations; iter++) {
        residual = 0.0f;

        if(fakeQuant) {
            result.z_opt = forwardImpl(result.h_opt, h_0, ML::FakeQuantizationI8());
            for(size_t i = 0; i < size * size; i++) {
                float h_next_i = activate<ML::FakeQuantizationI8>(result.z_opt(i), result.h_opt(i), alpha);
                float diff = h_next_i - result.h_opt(i);
                residual += diff * diff;
                result.h_opt(i) = h_next_i;
            }
        } else {
            result.z_opt = forwardImpl(result.h_opt, h_0, ML::Identity());
            for(size_t i = 0; i < size * size; i++) {
                float h_next_i = activate<ML::Identity>(result.z_opt(i), result.h_opt(i), alpha);
                float diff = h_next_i - result.h_opt(i);
                residual += diff * diff;
                result.h_opt(i) = h_next_i;
            }
        }

        residual = std::sqrt(residual);

        if(stepSizeBacktracking && residual >= result.residual)
            alpha *= 0.5f;

        result.residual = residual;
        result.iterations = iter + 1;

        if(residual < tol)
            break;
    }

    return result;
}

SparseRENLayer::Gradients SparseRENLayer::backward(const ForwardResult& f, const ML::Vector& inputGrad, bool fakeQuant, float tol) const {
    if(fakeQuant)
        return backwardImpl(f.h_opt, f.z_opt, inputGrad, tol, ML::FakeQuantizationI8());
    else
        return backwardImpl(f.h_opt, f.z_opt, inputGrad, tol, ML::Identity());
}