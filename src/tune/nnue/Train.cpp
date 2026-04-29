#include "tune/nnue/Train.h"
#include "core/engine/evaluation/NNUEEvaluator.h"
#include "core/utils/Random.h"
#include "core/utils/nnue/NNUEUtils.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

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

double Train::loss(std::vector<DataPoint>& data, const NNUE::MasterWeights& masterWeights, double k, double kappa) {
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

double Train::loss(std::vector<DataPoint>& data, const NNUE::Network& network, double k, double kappa) {
    std::atomic<double> sum = 0.0;

    size_t currIndex = 0;
    std::mutex mutex;

    auto threadFunc = [&]() {
        Board board;
        NNUEEvaluator evaluator(board, network);

        mutex.lock();
        while(currIndex < data.size()) {
            // Bearbeite Blöcke von 256 Datenpunkten
            size_t start = currIndex;
            size_t end = std::min(currIndex + 256, data.size());
            currIndex = end;
            mutex.unlock();

            for(size_t i = start; i < end; i++) {
                DataPoint& dp = data[i];
                evaluator.setBoard(dp.board);

                double prediction = tanh(evaluator.evaluate(), k);
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

NNUE::Gradients Train::gradient(std::vector<DataPoint>& data, const std::vector<size_t>& indices, const NNUE::MasterWeights& masterWeights, double k, double kappa) {
    size_t currIndex = 0;
    std::mutex mutex;

    size_t numThreads = std::max(std::thread::hardware_concurrency(), 1u);
    std::vector<NNUE::Gradients> threadGradientAccum;
    threadGradientAccum.resize(numThreads);

    auto threadFunc = [&](size_t threadId) {
        NNUE::Gradients& grads = threadGradientAccum[threadId];

        mutex.lock();
        while(currIndex < indices.size()) {
            // Bearbeite Blöcke von 64 Datenpunkten
            size_t start = currIndex;
            size_t end = std::min(currIndex + 64, indices.size());
            currIndex = end;
            mutex.unlock();

            for(size_t i = start; i < end; i++) {
                size_t dataIndex = indices[i];
                DataPoint& dp = data[dataIndex];

                NNUE::NetworkActivations activations = masterWeights.forward(dp.board, true);
                float networkOutput = activations.output();
                double cp = networkOutputToCentipawns(networkOutput);
                double prediction = tanh(cp, k);
                double target = (1.0 - kappa) * tanh(dp.tdTarget, k) + kappa * (double)dp.finalResult;

                double errorGrad = 2.0 * (prediction - target) * (1.0 - prediction * prediction) * k * 100.0 * (16384.0 / 6656.0);

                // Berechne die Gradienten für die Master-Parameter und addiere sie zum Thread-Gradienten
                NNUE::Gradients dpGrad = masterWeights.backward(dp.board, activations, errorGrad, true);

                grads.halfKAGradients.bias += dpGrad.halfKAGradients.bias;

                for(const auto& [feature, value] : dpGrad.halfKAGradients.weights)
                    grads.halfKAGradients.weights[feature] += value;

                for(size_t layer = 0; layer < NNUE::Network::NUM_LAYERS; layer++) {
                    grads.denseLayerGradients[layer].bias += dpGrad.denseLayerGradients[layer].bias;
                    grads.denseLayerGradients[layer].weights += dpGrad.denseLayerGradients[layer].weights;
                }
            }

            mutex.lock();
        }

        mutex.unlock();
    };

    // Starte die Threads
    std::vector<std::thread> threads;
    for(size_t i = 0; i < numThreads; i++)
        threads.push_back(std::thread(threadFunc, i));

    // Warte auf die Threads
    for(std::thread& t : threads)
        t.join();

    // Durchschnittsbildung
    NNUE::Gradients totalGrad;

    for(size_t i = 0; i < numThreads; i++) {
        totalGrad.halfKAGradients.bias += threadGradientAccum[i].halfKAGradients.bias;

        for(const auto& [feature, value] : threadGradientAccum[i].halfKAGradients.weights)
            totalGrad.halfKAGradients.weights[feature] += value;
    }

    totalGrad.halfKAGradients.bias /= indices.size();

    for(const auto& [feature, value] : totalGrad.halfKAGradients.weights)
        totalGrad.halfKAGradients.weights[feature] = totalGrad.halfKAGradients.weights[feature] / indices.size();

    for(size_t layer = 0; layer < NNUE::Network::NUM_LAYERS; layer++) {
        for(size_t i = 0; i < numThreads; i++) {
            totalGrad.denseLayerGradients[layer].bias += threadGradientAccum[i].denseLayerGradients[layer].bias;
            totalGrad.denseLayerGradients[layer].weights += threadGradientAccum[i].denseLayerGradients[layer].weights;
        }

        totalGrad.denseLayerGradients[layer].bias /= indices.size();
        totalGrad.denseLayerGradients[layer].weights /= indices.size();
    }

    return totalGrad;
}

NNUE::Network* Train::adamW(std::vector<DataPoint>& data, size_t numEpochs, double learningRate, double kappa) {
    NNUE::MasterWeights& masterWeights = trainingSession.masterWeights;
    NNUE::Network* bestNetwork = masterWeights.toNetwork();

    // Teile die Daten in Trainings- und Validierungsdaten auf
    size_t validationSize = data.size() * validationSplit.get<double>();
    std::vector<DataPoint> validationData;
    if(validationSize == 0) {
        validationData = data;
    } else {
        std::mt19937& shuffleRng = Random::generator<10>();
        std::shuffle(data.begin(), data.end(), shuffleRng);
        validationData = std::vector<DataPoint>(data.end() - validationSize, data.end());
        data.erase(data.end() - validationSize, data.end());
    }

    // Initialisiere den Indexvektor
    bool useBatch = batchSize.get<size_t>() < data.size();
    std::vector<size_t> indices(std::min(batchSize.get<size_t>(), data.size()));
    if(!useBatch) {
        for(size_t i = 0; i < indices.size(); i++)
            indices[i] = i;
    }

    std::mt19937& rng = Random::generator<11>();

    size_t patience = 0;

    double bestLoss = std::numeric_limits<double>::infinity();

    size_t targetEpochs = trainingSession.epoch + numEpochs;

    for(; trainingSession.epoch < targetEpochs; trainingSession.epoch++) {
        // Berechne den Fehler
        double masterLoss = Train::loss(validationData, masterWeights, k.get<double>(), kappa);

        NNUE::Network* currentNetwork = masterWeights.toNetwork();
        double networkLoss = Train::loss(validationData, *currentNetwork, k.get<double>(), kappa);

        std::cout << "\rEpoch: " << std::left << std::setw(4) << trainingSession.epoch;
        size_t currPrecision = std::cout.precision();
        std::cout << " Master val loss: " << std::setw(10) << std::setprecision(6) << masterLoss << std::right << std::flush;
        std::cout << " Quantized val loss: " << std::setw(10) << std::setprecision(6) << networkLoss << std::right << std::flush;
        std::cout.precision(currPrecision);

        // Überprüfe, ob der Fehler besser ist
        if(networkLoss < bestLoss) {
            bestLoss = networkLoss;
            patience = 0;
            delete bestNetwork;
            bestNetwork = currentNetwork;
        } else {
            delete currentNetwork;
            patience++;
        }

        // Überprüfe, ob die Geduld erschöpft ist
        if(patience >= noImprovementPatience.get<size_t>()) {
            std::cout << std::endl << "Early stopping at epoch " << trainingSession.epoch << std::endl;
            break;
        }

        if(useBatch) {
            // Bestimme zufällige Indizes
            std::uniform_int_distribution<size_t> dist(0, data.size() - 1);
            for(size_t& i : indices)
                i = dist(rng);
        }

        // Berechne die Gradienten
        NNUE::Gradients grad = Train::gradient(data, indices, masterWeights, k.get<double>(), kappa);

        // Aktualisiere die ersten und zweiten Momente
        for(size_t i = 0; i < grad.halfKAGradients.bias.size; i++) {
            trainingSession.mHalfKPBiases[i] = beta1.get<double>() * trainingSession.mHalfKPBiases[i] + (1.0 - beta1.get<double>()) * grad.halfKAGradients.bias(i);
            trainingSession.vHalfKPBiases[i] = beta2.get<double>() * trainingSession.vHalfKPBiases[i] + (1.0 - beta2.get<double>()) * grad.halfKAGradients.bias(i) * grad.halfKAGradients.bias(i);
        }

        double wd = weightDecay.get<double>();
        for(const auto& [feature, value] : grad.halfKAGradients.weights) {
            // Anzahl der Schritte seit dem letzten Update dieses Features (inklusive aktuellem Schritt)
            size_t tDelta = trainingSession.epoch - trainingSession.lastUpdateHalfKPWeights[feature] + 1;
            trainingSession.lastUpdateHalfKPWeights[feature] = trainingSession.epoch + 1;

            trainingSession.mHalfKPWeights[feature] = std::pow(beta1.get<double>(), tDelta) * trainingSession.mHalfKPWeights[feature] + (1.0 - beta1.get<double>()) * value;
            trainingSession.vHalfKPWeights[feature] = std::pow(beta2.get<double>(), tDelta) * trainingSession.vHalfKPWeights[feature] + (1.0 - beta2.get<double>()) * value * value;

            double mHat = trainingSession.mHalfKPWeights[feature] / (1.0 - std::pow(beta1.get<double>(), trainingSession.epoch + 1));
            double vHat = trainingSession.vHalfKPWeights[feature] / (1.0 - std::pow(beta2.get<double>(), trainingSession.epoch + 1));

            // Lazy AdamW: trage den verpassten Decay exakt nach, bevor der Gradienten-Schritt angewendet wird.
            double decayFactor = std::pow(1.0 - learningRate * wd, tDelta);
            masterWeights.halfKPLayer.weights(feature) = masterWeights.halfKPLayer.weights(feature) * decayFactor
                - learningRate * mHat / (std::sqrt(vHat) + epsilon.get<double>());

            masterWeights.halfKPLayer.weights(feature) = std::clamp(masterWeights.halfKPLayer.weights(feature),
                NNUE::MasterWeights::HALF_KP_MIN, NNUE::MasterWeights::HALF_KP_MAX);
        }

        for(size_t layer = 0; layer < NNUE::Network::NUM_LAYERS; layer++) {
            for(size_t i = 0; i < masterWeights.denseLayers[layer].bias.size; i++) {
                trainingSession.mDenseLayerBiases[layer][i] = beta1.get<double>() * trainingSession.mDenseLayerBiases[layer][i] + (1.0 - beta1.get<double>()) * grad.denseLayerGradients[layer].bias(i);
                trainingSession.vDenseLayerBiases[layer][i] = beta2.get<double>() * trainingSession.vDenseLayerBiases[layer][i] + (1.0 - beta2.get<double>()) * grad.denseLayerGradients[layer].bias(i) * grad.denseLayerGradients[layer].bias(i);
            }

            for(size_t i = 0; i < masterWeights.denseLayers[layer].weights.size; i++) {
                trainingSession.mDenseLayerWeights[layer][i] = beta1.get<double>() * trainingSession.mDenseLayerWeights[layer][i] + (1.0 - beta1.get<double>()) * grad.denseLayerGradients[layer].weights(i);
                trainingSession.vDenseLayerWeights[layer][i] = beta2.get<double>() * trainingSession.vDenseLayerWeights[layer][i] + (1.0 - beta2.get<double>()) * grad.denseLayerGradients[layer].weights(i) * grad.denseLayerGradients[layer].weights(i);
            }
        }

        // Aktualisiere die Master-Parameter mit AdamW
        for(size_t i = 0; i < masterWeights.halfKPLayer.bias.size; i++) {
            double mHat = trainingSession.mHalfKPBiases[i] / (1.0 - std::pow(beta1.get<double>(), trainingSession.epoch + 1));
            double vHat = trainingSession.vHalfKPBiases[i] / (1.0 - std::pow(beta2.get<double>(), trainingSession.epoch + 1));
            
            // AdamW: Weight Decay wird direkt bei der Parameter-Aktualisierung angewendet
            masterWeights.halfKPLayer.bias(i) = masterWeights.halfKPLayer.bias(i) * (1.0 - learningRate * wd) 
                - learningRate * mHat / (std::sqrt(vHat) + epsilon.get<double>());

            masterWeights.halfKPLayer.bias(i) = std::clamp(masterWeights.halfKPLayer.bias(i),
                NNUE::MasterWeights::HALF_KP_MIN, NNUE::MasterWeights::HALF_KP_MAX);
        }

        for(size_t layer = 0; layer < NNUE::Network::NUM_LAYERS; layer++) {
            for(size_t i = 0; i < masterWeights.denseLayers[layer].bias.size; i++) {
                double mHat = trainingSession.mDenseLayerBiases[layer][i] / (1.0 - std::pow(beta1.get<double>(), trainingSession.epoch + 1));
                double vHat = trainingSession.vDenseLayerBiases[layer][i] / (1.0 - std::pow(beta2.get<double>(), trainingSession.epoch + 1));
                
                // AdamW: Weight Decay wird direkt bei der Parameter-Aktualisierung angewendet
                masterWeights.denseLayers[layer].bias(i) = masterWeights.denseLayers[layer].bias(i) * (1.0 - learningRate * wd)
                    - learningRate * mHat / (std::sqrt(vHat) + epsilon.get<double>());

                masterWeights.denseLayers[layer].bias(i) = std::clamp(masterWeights.denseLayers[layer].bias(i),
                    NNUE::MasterWeights::DENSE_BIAS_MIN, NNUE::MasterWeights::DENSE_BIAS_MAX);
            }

            for(size_t i = 0; i < masterWeights.denseLayers[layer].weights.size; i++) {
                double mHat = trainingSession.mDenseLayerWeights[layer][i] / (1.0 - std::pow(beta1.get<double>(), trainingSession.epoch + 1));
                double vHat = trainingSession.vDenseLayerWeights[layer][i] / (1.0 - std::pow(beta2.get<double>(), trainingSession.epoch + 1));
                
                // AdamW: Weight Decay wird direkt bei der Parameter-Aktualisierung angewendet
                masterWeights.denseLayers[layer].weights(i) = masterWeights.denseLayers[layer].weights(i) * (1.0 - learningRate * wd)
                    - learningRate * mHat / (std::sqrt(vHat) + epsilon.get<double>());

                masterWeights.denseLayers[layer].weights(i) = std::clamp(masterWeights.denseLayers[layer].weights(i),
                    NNUE::MasterWeights::DENSE_WEIGHT_MIN, NNUE::MasterWeights::DENSE_WEIGHT_MAX);
            }
        }
    }

    // Berechne den finalen Fehler
    double masterLoss = Train::loss(validationData, masterWeights, k.get<double>(), kappa);
    NNUE::Network* currentNetwork = masterWeights.toNetwork();
    double networkLoss = Train::loss(validationData, *currentNetwork, k.get<double>(), kappa);
    std::cout << "\rEpoch: " << std::left << std::setw(4) << trainingSession.epoch;
    size_t currPrecision = std::cout.precision();
    std::cout << " Master val loss: " << std::setw(10) << std::setprecision(6) << masterLoss << std::right << std::flush;
    std::cout << " Quantized val loss: " << std::setw(10) << std::setprecision(6) << networkLoss << std::right << std::endl;

    if(networkLoss < bestLoss) {
        bestLoss = networkLoss;
        delete bestNetwork;
        bestNetwork = currentNetwork;
    } else
        delete currentNetwork;

    trainingSession.generation++;
    trainingSession.averageLoss = trainingSession.averageLoss * alpha.get<double>() + (1.0 - alpha.get<double>()) * bestLoss;
    // Bias-korrigierter Loss
    double biasCorrectedLoss = trainingSession.averageLoss / (1 - std::pow(alpha.get<double>(), trainingSession.generation));

    std::cout << "Average loss: " << std::setprecision(6) << biasCorrectedLoss << std::endl;
    std::cout.precision(currPrecision);

    return bestNetwork;
}

void Train::kaimingInitialization(NNUE::MasterWeights& masterWeights) {
    std::mt19937& rng = Random::generator<12>();

    // Initialisiere Half-KP-Gewichte mit Kaiming-Initialisierung
    std::normal_distribution<float> halfKPDist(0.0f, 2.0f / std::sqrt(NNUE::Network::INPUT_SIZE));
    for(size_t i = 0; i < masterWeights.halfKPLayer.weights.size; i++)
        masterWeights.halfKPLayer.weights(i) = halfKPDist(rng);

    // Initialisiere Half-KP-Biases mit 0.1
    for(size_t i = 0; i < masterWeights.halfKPLayer.bias.size; i++)
        masterWeights.halfKPLayer.bias(i) = 0.1f;

    // Initialisiere Dense-Gewichte mit Kaiming-Initialisierung
    for(size_t layer = 0; layer < NNUE::Network::NUM_LAYERS; layer++) {
        float fanIn = NNUE::Network::LAYER_SIZES[layer];
        float stddev = std::sqrt(2.0f / fanIn);
        std::normal_distribution<float> dist(0.0f, stddev);

        for(size_t i = 0; i < masterWeights.denseLayers[layer].weights.size; i++)
            masterWeights.denseLayers[layer].weights(i) = dist(rng);
    }

    // Initialisiere alle Dense-Biases mit 0
    for(size_t layer = 0; layer < NNUE::Network::NUM_LAYERS; layer++)
        for(size_t i = 0; i < masterWeights.denseLayers[layer].bias.size; i++)
            masterWeights.denseLayers[layer].bias(i) = 0;
}