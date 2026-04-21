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
    return (output * (64.0 * 64.0) * 100.0 / 3328.0);
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

                double errorGrad = 2.0 * (prediction - target) * (1.0 - prediction * prediction) * k * 100.0 * (4096.0 / 3328.0);

                // Berechne die Gradienten für die Master-Parameter und addiere sie zum Thread-Gradienten
                NNUE::Gradients dpGrad = masterWeights.backward(dp.board, activations, errorGrad, true);

                for(size_t j = 0; j < grads.halfKPBiases.size(); j++)
                    grads.halfKPBiases[j] += dpGrad.halfKPBiases[j];

                for(const auto& [feature, value] : dpGrad.halfKPWeights)
                    grads.halfKPWeights[feature] += value;

                for(size_t layer = 0; layer < NNUE::Network::NUM_LAYERS; layer++) {
                    for(size_t j = 0; j < grads.denseLayerBiases[layer].size(); j++)
                        grads.denseLayerBiases[layer][j] += dpGrad.denseLayerBiases[layer][j];

                    for(size_t j = 0; j < grads.denseLayerWeights[layer].size(); j++)
                        grads.denseLayerWeights[layer][j] += dpGrad.denseLayerWeights[layer][j];
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

    for(size_t i = 0; i < totalGrad.halfKPBiases.size(); i++) {
        for(size_t t = 0; t < numThreads; t++)
            totalGrad.halfKPBiases[i] += threadGradientAccum[t].halfKPBiases[i];

        totalGrad.halfKPBiases[i] = totalGrad.halfKPBiases[i] / indices.size();
    }

    for(size_t t = 0; t < numThreads; t++)
        for(const auto& [feature, value] : threadGradientAccum[t].halfKPWeights)
            totalGrad.halfKPWeights[feature] += value;

    for(const auto& [feature, value] : totalGrad.halfKPWeights) {
        totalGrad.halfKPWeights[feature] = totalGrad.halfKPWeights[feature] / indices.size();
    }

    for(size_t layer = 0; layer < NNUE::Network::NUM_LAYERS; layer++) {
        for(size_t i = 0; i < totalGrad.denseLayerBiases[layer].size(); i++) {
            for(size_t t = 0; t < numThreads; t++)
                totalGrad.denseLayerBiases[layer][i] += threadGradientAccum[t].denseLayerBiases[layer][i];

            totalGrad.denseLayerBiases[layer][i] = totalGrad.denseLayerBiases[layer][i] / indices.size();
        }

        for(size_t i = 0; i < totalGrad.denseLayerWeights[layer].size(); i++) {
            for(size_t t = 0; t < numThreads; t++)
                totalGrad.denseLayerWeights[layer][i] += threadGradientAccum[t].denseLayerWeights[layer][i];

            totalGrad.denseLayerWeights[layer][i] = totalGrad.denseLayerWeights[layer][i] / indices.size();
        }
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
        for(size_t i = 0; i < masterWeights.exactHalfKPBiases.size(); i++) {
            trainingSession.mHalfKPBiases[i] = beta1.get<double>() * trainingSession.mHalfKPBiases[i] + (1.0 - beta1.get<double>()) * grad.halfKPBiases[i];
            trainingSession.vHalfKPBiases[i] = beta2.get<double>() * trainingSession.vHalfKPBiases[i] + (1.0 - beta2.get<double>()) * grad.halfKPBiases[i] * grad.halfKPBiases[i];
        }

        for(const auto& [feature, value] : grad.halfKPWeights) {
            // Anzahl der verpassten Momentum-Schritte
            size_t tDelta = trainingSession.epoch - trainingSession.lastUpdateHalfKPWeights[feature];
            trainingSession.lastUpdateHalfKPWeights[feature] = trainingSession.epoch + 1;

            trainingSession.mHalfKPWeights[feature] = std::pow(beta1.get<double>(), tDelta + 1) * trainingSession.mHalfKPWeights[feature] + (1.0 - beta1.get<double>()) * value;
            trainingSession.vHalfKPWeights[feature] = std::pow(beta2.get<double>(), tDelta + 1) * trainingSession.vHalfKPWeights[feature] + (1.0 - beta2.get<double>()) * value * value;
        }

        for(size_t layer = 0; layer < NNUE::Network::NUM_LAYERS; layer++) {
            for(size_t i = 0; i < masterWeights.exactDenseLayerBiases[layer].size(); i++) {
                trainingSession.mDenseLayerBiases[layer][i] = beta1.get<double>() * trainingSession.mDenseLayerBiases[layer][i] + (1.0 - beta1.get<double>()) * grad.denseLayerBiases[layer][i];
                trainingSession.vDenseLayerBiases[layer][i] = beta2.get<double>() * trainingSession.vDenseLayerBiases[layer][i] + (1.0 - beta2.get<double>()) * grad.denseLayerBiases[layer][i] * grad.denseLayerBiases[layer][i];
            }

            for(size_t i = 0; i < masterWeights.exactDenseLayerWeights[layer].size(); i++) {
                trainingSession.mDenseLayerWeights[layer][i] = beta1.get<double>() * trainingSession.mDenseLayerWeights[layer][i] + (1.0 - beta1.get<double>()) * grad.denseLayerWeights[layer][i];
                trainingSession.vDenseLayerWeights[layer][i] = beta2.get<double>() * trainingSession.vDenseLayerWeights[layer][i] + (1.0 - beta2.get<double>()) * grad.denseLayerWeights[layer][i] * grad.denseLayerWeights[layer][i];
            }
        }

        // Aktualisiere die Master-Parameter mit AdamW
        double wd = weightDecay.get<double>();
        for(size_t i = 0; i < masterWeights.exactHalfKPBiases.size(); i++) {
            double mHat = trainingSession.mHalfKPBiases[i] / (1.0 - std::pow(beta1.get<double>(), trainingSession.epoch + 1));
            double vHat = trainingSession.vHalfKPBiases[i] / (1.0 - std::pow(beta2.get<double>(), trainingSession.epoch + 1));
            
            // AdamW: Weight Decay wird direkt bei der Parameter-Aktualisierung angewendet
            masterWeights.exactHalfKPBiases[i] = masterWeights.exactHalfKPBiases[i] * (1.0 - learningRate * wd) 
                - learningRate * mHat / (std::sqrt(vHat) + epsilon.get<double>());

            masterWeights.exactHalfKPBiases[i] = std::clamp(masterWeights.exactHalfKPBiases[i],
                NNUE::MasterWeights::HALF_KP_MIN, NNUE::MasterWeights::HALF_KP_MAX);
        }

        // Aktualisiere nur die Gewichte, für die Gradienten != 0 existieren (sparse Update)
        for(const auto& [feature, value] : grad.halfKPWeights) {
             double mHat = trainingSession.mHalfKPWeights[feature] / (1.0 - std::pow(beta1.get<double>(), trainingSession.epoch + 1));
             double vHat = trainingSession.vHalfKPWeights[feature] / (1.0 - std::pow(beta2.get<double>(), trainingSession.epoch + 1));
             
             // AdamW: Weight Decay wird direkt bei der Parameter-Aktualisierung angewendet
             masterWeights.exactHalfKP[feature] = masterWeights.exactHalfKP[feature] * (1.0 - learningRate * wd)
                 - learningRate * mHat / (std::sqrt(vHat) + epsilon.get<double>());

             masterWeights.exactHalfKP[feature] = std::clamp(masterWeights.exactHalfKP[feature],
                 NNUE::MasterWeights::HALF_KP_MIN, NNUE::MasterWeights::HALF_KP_MAX);
        }

        for(size_t layer = 0; layer < NNUE::Network::NUM_LAYERS; layer++) {
            for(size_t i = 0; i < masterWeights.exactDenseLayerBiases[layer].size(); i++) {
                double mHat = trainingSession.mDenseLayerBiases[layer][i] / (1.0 - std::pow(beta1.get<double>(), trainingSession.epoch + 1));
                double vHat = trainingSession.vDenseLayerBiases[layer][i] / (1.0 - std::pow(beta2.get<double>(), trainingSession.epoch + 1));
                
                // AdamW: Weight Decay wird direkt bei der Parameter-Aktualisierung angewendet
                masterWeights.exactDenseLayerBiases[layer][i] = masterWeights.exactDenseLayerBiases[layer][i] * (1.0 - learningRate * wd)
                    - learningRate * mHat / (std::sqrt(vHat) + epsilon.get<double>());

                masterWeights.exactDenseLayerBiases[layer][i] = std::clamp(masterWeights.exactDenseLayerBiases[layer][i],
                    NNUE::MasterWeights::DENSE_BIAS_MIN, NNUE::MasterWeights::DENSE_BIAS_MAX);
            }

            for(size_t i = 0; i < masterWeights.exactDenseLayerWeights[layer].size(); i++) {
                double mHat = trainingSession.mDenseLayerWeights[layer][i] / (1.0 - std::pow(beta1.get<double>(), trainingSession.epoch + 1));
                double vHat = trainingSession.vDenseLayerWeights[layer][i] / (1.0 - std::pow(beta2.get<double>(), trainingSession.epoch + 1));
                
                // AdamW: Weight Decay wird direkt bei der Parameter-Aktualisierung angewendet
                masterWeights.exactDenseLayerWeights[layer][i] = masterWeights.exactDenseLayerWeights[layer][i] * (1.0 - learningRate * wd)
                    - learningRate * mHat / (std::sqrt(vHat) + epsilon.get<double>());

                masterWeights.exactDenseLayerWeights[layer][i] = std::clamp(masterWeights.exactDenseLayerWeights[layer][i],
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
    for(size_t i = 0; i < masterWeights.exactHalfKP.size(); i++)
        masterWeights.exactHalfKP[i] = halfKPDist(rng);

    // Initialisiere Half-KP-Biases mit 0.1
    for(size_t i = 0; i < masterWeights.exactHalfKPBiases.size(); i++)
        masterWeights.exactHalfKPBiases[i] = 0.1f;

    // Initialisiere Dense-Gewichte mit Kaiming-Initialisierung
    for(size_t layer = 0; layer < NNUE::Network::NUM_LAYERS; layer++) {
        float fanIn = NNUE::Network::LAYER_SIZES[layer];
        float stddev = std::sqrt(2.0f / fanIn);
        std::normal_distribution<float> dist(0.0f, stddev);

        for(size_t i = 0; i < masterWeights.exactDenseLayerWeights[layer].size(); i++)
            masterWeights.exactDenseLayerWeights[layer][i] = dist(rng);
    }

    // Initialisiere alle Dense-Biases mit 0
    for(size_t layer = 0; layer < NNUE::Network::NUM_LAYERS; layer++)
        for(size_t i = 0; i < masterWeights.exactDenseLayerBiases[layer].size(); i++)
            masterWeights.exactDenseLayerBiases[layer][i] = 0;
}