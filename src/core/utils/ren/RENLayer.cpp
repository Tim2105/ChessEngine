#include "core/utils/ren/RENLayer.h"

#include <algorithm>
#include <chrono>
#include <limits>
#include <iostream>

#include "core/utils/ren/Math.h"

using namespace REN;

constexpr float clippedReLU(float x, float maxVal) {
    return std::clamp(x, 0.0f, maxVal);
}

constexpr bool clippedReLUDerivative(float x, float maxVal) {
    return x > 0.0f && x < maxVal;
}

namespace Quantization {
    struct FakeQuantization {
        constexpr static float ONE = 128.0f;
        constexpr static float MIN_VALUE = std::numeric_limits<int8_t>::min() / ONE;
        constexpr static float MAX_VALUE = std::numeric_limits<int8_t>::max() / ONE;
        constexpr static float CLIPPED_RELU_MAX = (std::numeric_limits<int8_t>::max() / ONE);

        /**
         * @brief Quantisierungsfunktion,
         * die einen Wert auf einen darstellbaren Wert im quantisierten Format rundet.
         * 
         * @param x Der Wert.
         * @param minVal Das Minimum des Intervalls.
         * @param maxVal Das Maximum des Intervalls.
         * @param scale Der Skalierungsfaktor der Quantisierung.
         */
        constexpr float operator()(float x) const {
            return std::clamp(std::round(x * ONE) / ONE, MIN_VALUE, MAX_VALUE);
        }
    };

    struct Identity {
        constexpr static float CLIPPED_RELU_MAX = FakeQuantization::CLIPPED_RELU_MAX;

        /**
         * @brief Identitätsfunktion, die als Quantisierungsfunktion verwendet werden kann,
         * wenn keine Quantisierung durchgeführt werden soll.
         */
        constexpr float operator()(float x) const {
            return x;
        }
    };
}

void RENLayer::constructTransform() {
    
}

template <typename Q>
std::tuple<Vector, float> RENLayer::forwardImpl(const Vector& h_t, const Vector& z_0, float alpha, Q q) const {
    Vector result(size);
    float residualSq = 0.0f;

    for(size_t i = 0; i < size; i++) {
        float sum = z_0(i) + q(bias(i));
        for(size_t j = 0; j < size; j++)
            sum += q(transform(i, j)) * h_t(j);

        float z_i = q((1 - alpha) * h_t(i) + alpha * clippedReLU(sum, Q::CLIPPED_RELU_MAX));
        result(i) = z_i;
        float res = z_i - h_t(i);
        residualSq += res * res;
    }

    return {result, std::sqrt(residualSq)};
};

template <typename Q>
RENLayer::Gradients RENLayer::backwardImpl(const Vector& h_opt, const Vector& z_opt, const Vector& h_0, const Vector& inputGrad, float tol, Q q) const {
    Gradients grads;

    // System I - W_r^T * diag(clippedReLU'(z_opt)) aufstellen
    Matrix system(size);
    for(size_t i = 0; i < size; i++) {
        for(size_t j = 0; j < size; j++) {
            float val = q(transform(j, i)) * (float)clippedReLUDerivative(z_opt(j), Q::CLIPPED_RELU_MAX);
            system(i, j) = (i == j ? 1.0f : 0.0f) - val;
        }
    }

    constexpr size_t M = 50; // Krylov-Unterraumgröße für GMRES
    size_t restarts = (size_t)std::ceil(size / (double)M); // Maximale Anzahl der Restarts
    Vector v = REN::gmresRestarted(system, inputGrad, M, restarts, tol);

    // Berechne v' = diag(clippedReLU'(z_opt)) * v
    for(size_t i = 0; i < size; i++)
        v(i) *= (float)clippedReLUDerivative(z_opt(i), Q::CLIPPED_RELU_MAX);

    // Gradient für bias und Ausgabe
    for(size_t i = 0; i < size; i++) {
        grads.bias(i) = v(i);
        grads.outputGrad(i) = v(i);
    }

    // Gradient für gammaRaw
    for(size_t i = 0; i < size; i++) {
        float tanh_val = std::tanh(surrogateWeights.gammaRaw(i));
        grads.gammaRaw(i) = v(i) * h_opt(i) * (1.0f - epsilon) * (1.0f - tanh_val * tanh_val);
    }

    // Gradient für q
    // g_q = -(v h_opt^T + h_opt v^T) * q
    
    // Berechne Hilfsvektoren h_opt^T * q und v^T * q
    Vector hq(size), vq(size);
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

void RENLayer::testSolver() const {
    size_t testSize = size;
    Matrix a(testSize);
    Vector b(testSize);

    for(size_t i = 0; i < testSize; i++) {
        b(i) = i / (float)testSize + 1.0f;
        for(size_t j = 0; j < testSize; j++) {
            a(i, j) = (i == j) ? 2.0f : 1.0f;
        }
    }

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    Vector x = REN::gmresRestarted(a, b, 50, 10, 1e-5f);
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "GMRES löste das Testproblem in " << duration.count() << " Mikrosekunden." << std::endl;

    // Berechne den mittleren absoluten Fehler der Lösung
    double error = 0.0f;
    for(size_t i = 0; i < testSize; i++) {
        double expected = b(i);
        double actual = 0.0f;
        for(size_t j = 0; j < testSize; j++) {
            actual += a(i, j) * x(j);
        }
        error += std::abs(expected - actual);
    }

    error /= testSize;
    std::cout << "Mittlerer absoluter Fehler der Lösung: " << error << std::endl;
}