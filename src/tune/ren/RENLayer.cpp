#include "tune/ren/RENLayer.h"

#include <algorithm>
#include <chrono>
#include <limits>
#include <iostream>

#include "tune/ml/Math.h"
#include "tune/ml/Quantization.h"

using namespace REN;

void RENLayer::constructTransform() {
    // W_r = -Q Q^T + diag((1 - epsilon) * tanh(gamma_raw))
    for(size_t i = 0; i < size; i++) {
        for(size_t j = 0; j < size; j++) {
            float sum = 0.0f;
            for(size_t k = 0; k < size; k++)
                sum += surrogateWeights.q(i, k) * surrogateWeights.q(j, k);

            float val = -sum;
            if(i == j)
                val += (1.0f - epsilon) * std::tanh(surrogateWeights.gammaRaw(i));

            transform(i, j) = val;
            transform(j, i) = val; // Symmetrie ausnutzen
        }
    }
}

template <typename Q>
ML::Vector RENLayer::forwardImpl(const ML::Vector& h_t, const ML::Vector& z_0, Q q) const {
    ML::Vector result(size);

    for(size_t i = 0; i < size; i++) {
        float sum = z_0(i) + q(bias(i));
        for(size_t j = 0; j < size; j++)
            sum += q(transform(i, j)) * h_t(j);

        result(i) = sum;
    }

    return result;
};

template <typename Q>
RENLayer::Gradients RENLayer::backwardImpl(const ML::Vector& h_opt, const ML::Vector& z_opt, const ML::Vector& inputGrad, float tol, Q q) const {
    Gradients grads(size);

    // System I - W_r^T * diag(clippedReLU'(z_opt)) aufstellen
    ML::Matrix system(size);
    for(size_t i = 0; i < size; i++) {
        for(size_t j = 0; j < size; j++) {
            float val = q(transform(j, i)) * (float)clippedReLUDerivative(z_opt(j), Q::CLIPPED_RELU_MAX);
            system(i, j) = (i == j ? 1.0f : 0.0f) - val;
        }
    }

    constexpr size_t M = 50; // Krylov-Unterraumgröße für GMRES
    size_t restarts = (size_t)std::ceil(size / (double)M); // Maximale Anzahl der Restarts
    ML::Vector v = ML::gmresRestarted(system, inputGrad, M, restarts, tol);

    // Berechne v' = diag(clippedReLU'(z_opt)) * v
    for(size_t i = 0; i < size; i++)
        v(i) *= (float)clippedReLUDerivative(z_opt(i), Q::CLIPPED_RELU_MAX);

    // Gradient für bias und Ausgabe
    grads.bias = v;
    grads.inputGrad = v;

    // Gradient für gammaRaw
    for(size_t i = 0; i < size; i++) {
        float tanh_val = std::tanh(surrogateWeights.gammaRaw(i));
        grads.gammaRaw(i) = v(i) * h_opt(i) * (1.0f - epsilon) * (1.0f - tanh_val * tanh_val);
    }

    // Gradient für q
    // g_q = -(v h_opt^T + h_opt v^T) * q
    
    // Berechne Hilfsvektoren h_opt^T * q und v^T * q
    ML::Vector hq(size), vq(size);
    for(size_t i = 0; i < size; i++) {
        float hq_i = 0.0f, vq_i = 0.0f;
        for(size_t j = 0; j < size; j++) {
            hq_i += h_opt(j) * surrogateWeights.q(j, i);
            vq_i += v(j) * surrogateWeights.q(j, i);
        }
        hq(i) = hq_i;
        vq(i) = vq_i;
    }

    // Addiere äußere Produkte um g_q zu berechnen
    for(size_t i = 0; i < size; i++)
        for(size_t j = 0; j < size; j++)
            grads.q(i, j) = -(v(i) * hq(j) + h_opt(i) * vq(j));

    return grads;
}

template <typename Q>
inline float activate(float z_i, float h_i, float alpha) {
    float act = clippedReLU(z_i, Q::CLIPPED_RELU_MAX);
    return (1.0f - alpha) * h_i + alpha * act;
}

RENLayer::ForwardResult RENLayer::forward(const ML::Vector& h_0, bool fakeQuant, float alpha, size_t maxIterations, float tol) const {
    ForwardResult result(h_0);

    float residual = std::numeric_limits<float>::max();
    for(size_t iter = 0; iter < maxIterations; iter++) {
        residual = 0.0f;

        if(fakeQuant) {
            result.z_opt = forwardImpl(result.h_opt, h_0, ML::FakeQuantizationI8());
            for(size_t i = 0; i < size; i++) {
                float h_next_i = activate<ML::FakeQuantizationI8>(result.z_opt(i), result.h_opt(i), alpha);
                float diff = h_next_i - result.h_opt(i);
                residual += diff * diff;
                result.h_opt(i) = h_next_i;
            }
        } else {
            result.z_opt = forwardImpl(result.h_opt, h_0, ML::Identity());
            for(size_t i = 0; i < size; i++) {
                float h_next_i = activate<ML::Identity>(result.z_opt(i), result.h_opt(i), alpha);
                float diff = h_next_i - result.h_opt(i);
                residual += diff * diff;
                result.h_opt(i) = h_next_i;
            }
        }

        residual = std::sqrt(residual);
        result.residual = residual;
        result.iterations = iter + 1;

        if(residual < tol)
            break;
    }

    return result;
}

RENLayer::Gradients RENLayer::backward(const ForwardResult& f, const ML::Vector& inputGrad, bool fakeQuant, float tol) const {
    if(fakeQuant)
        return backwardImpl(f.h_opt, f.z_opt, inputGrad, tol, ML::FakeQuantizationI8());
    else
        return backwardImpl(f.h_opt, f.z_opt, inputGrad, tol, ML::Identity());
}