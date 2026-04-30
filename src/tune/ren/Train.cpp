#include "tune/ren/Train.h"
#include "core/utils/Random.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <thread>

using namespace Train;

TrainingSession Train::trainingSession;

/**
 * @brief Tanh-Funktion mit einem zusätzlichen Parameter.
 * 
 * @param x Der Wert.
 * @param k Der Parameter.
 */
constexpr double tanh(double x, double k) {
    return std::tanh(k * x);
}

/**
 * @brief Berechnet den mittleren quadratischen Fehler.
 * 
 * @param x Der erste Wert.
 * @param y Der zweite Wert.
 */
constexpr double mse(double x, double y) {
    return (x - y) * (x - y);
}

double networkOutputToCentipawns(float output) {
    return (output * (128.0 * 128.0) * 100.0 / 6656.0);
}

double Train::loss(std::vector<DataPoint>& data, const REN::MasterWeights& masterWeights, double k, double kappa) {
    std::atomic<double> sum = 0.0;

    size_t currIndex = 0;
    std::mutex mutex;

    auto threadFunc = [&]() {
        mutex.lock();
        while(currIndex < data.size()) {
            // Bearbeite Blöcke von 256 Datenpunkten
            size_t start = currIndex;
            size_t end = std::min(currIndex + 256, data.size());
            currIndex = end;
            mutex.unlock();

            for(size_t i = start; i < end; i++) {
                DataPoint& dp = data[i];

                float networkOutput = masterWeights.forward(dp.board, true).output();
                double prediction = tanh(networkOutputToCentipawns(networkOutput), k);
                double target = (1.0 - kappa) * tanh(dp.tdTarget, k) + kappa * (double)dp.finalResult;

                sum.fetch_add(mse(prediction, target));
            }

            mutex.lock();
        }

        mutex.unlock();
    };

    // Starte die Threads
    std::vector<std::thread> threads;
    for(size_t i = 0; i < std::max(std::thread::hardware_concurrency(), 1u); i++)
        threads.push_back(std::thread(threadFunc));

    // Warte auf die Threads
    for(std::thread& t : threads)
        t.join();

    return sum.load() / data.size();
}

void Train::initializeWeights(REN::MasterWeights& masterWeights) {
    // Kaiming-Initialisierung für die HalfKAv2_hm- und Dense-Layer
    std::mt19937& rng = Random::generator<12>();
    std::normal_distribution<float> distHalfKA(0.0f, 2.0f / std::sqrt(32.0f));
    std::normal_distribution<float> distREN(0.0f, 0.1f / (float)std::sqrt(REN::REN_SIZE));
    std::normal_distribution<float> distDense(0.0f, std::sqrt(2.0f / REN::REN_SIZE));

    for(size_t i = 0; i < masterWeights.halfKAv2Layer.weights.size; i++)
        masterWeights.halfKAv2Layer.weights(i) = distHalfKA(rng);

    for(size_t i = 0; i < masterWeights.halfKAv2Layer.bias.size; i++)
        masterWeights.halfKAv2Layer.bias(i) = 0.1f;

    for(size_t i = 0; i < masterWeights.outputLayer.weights.size; i++)
        masterWeights.outputLayer.weights(i) = distDense(rng);

    for(size_t i = 0; i < masterWeights.outputLayer.bias.size; i++)
        masterWeights.outputLayer.bias(i) = 0.0f;

    // Initialisiere die REN-Schicht so, dass sie zu Beginn fast eine Identitätsfunktion darstellt
    for(size_t i = 0; i < masterWeights.renLayer.surrogateWeights.q.size; i++)
        masterWeights.renLayer.surrogateWeights.q(i) = distREN(rng);

    for(size_t i = 0; i < masterWeights.renLayer.surrogateWeights.gammaRaw.size; i++) 
        masterWeights.renLayer.surrogateWeights.gammaRaw(i) = 0.0f;

    for(size_t i = 0; i < masterWeights.renLayer.bias.size; i++)
        masterWeights.renLayer.bias(i) = 0.0f;

    masterWeights.renLayer.constructTransform();
}