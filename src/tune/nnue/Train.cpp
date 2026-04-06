#include "tune/nnue/Train.h"
#include "core/engine/evaluation/NNUEEvaluator.h"
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

double Train::loss(std::vector<DataPoint>& data, const NNUE::MasterWeights& masterWeights, double k, double weightDecay) {
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
                double trueValue = tanh(dp.result, k);

                sum.fetch_add(mse(prediction, trueValue));
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

    sum.store(sum.load() / data.size());

    // L2-Regularisierung hinzufügen
    double wd = 0.0;
    size_t nWeights = 0;

    nWeights += masterWeights.exactHalfKPBiases.size();
    for(size_t i = 0; i < masterWeights.exactHalfKPBiases.size(); i++)
        wd += (double)masterWeights.exactHalfKPBiases[i] * masterWeights.exactHalfKPBiases[i];

    nWeights += masterWeights.exactHalfKP.size();
    for(size_t i = 0; i < masterWeights.exactHalfKP.size(); i++)
        wd += (double)masterWeights.exactHalfKP[i] * masterWeights.exactHalfKP[i];

    for(size_t layer = 0; layer < NNUE::Network::NUM_LAYERS; layer++) {
        nWeights += masterWeights.exactDenseLayerBiases[layer].size();
        for(size_t i = 0; i < masterWeights.exactDenseLayerBiases[layer].size(); i++)
            wd += (double)masterWeights.exactDenseLayerBiases[layer][i] * masterWeights.exactDenseLayerBiases[layer][i];

        nWeights += masterWeights.exactDenseLayerWeights[layer].size();
        for(size_t i = 0; i < masterWeights.exactDenseLayerWeights[layer].size(); i++)
            wd += (double)masterWeights.exactDenseLayerWeights[layer][i] * masterWeights.exactDenseLayerWeights[layer][i];
    }

    wd = wd / nWeights;
    return sum.load() + weightDecay * wd;
}

double Train::loss(std::vector<DataPoint>& data, const NNUE::Network& network, double k, double weightDecay) {
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
                double trueValue = tanh(dp.result, k);

                sum.fetch_add(mse(prediction, trueValue));
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

    sum.store(sum.load() / data.size());

    // L2-Regularisierung hinzufügen
    double wd = 0.0;
    size_t nWeights = 0;

    nWeights += NNUE::Network::SINGLE_SUBNET_SIZE;
    for(size_t i = 0; i < NNUE::Network::SINGLE_SUBNET_SIZE; i++)
        wd += (network.getHalfKPLayer().getBias(i) / (64.0 * 64.0)) * (network.getHalfKPLayer().getBias(i) / (64.0 * 64.0));

    nWeights += NNUE::Network::INPUT_SIZE * NNUE::Network::SINGLE_SUBNET_SIZE;
    for(size_t i = 0; i < NNUE::Network::INPUT_SIZE; i++)
        for(size_t j = 0; j < NNUE::Network::SINGLE_SUBNET_SIZE; j++)
            wd += (network.getHalfKPLayer().getWeight(i, j) / 64.0) * (network.getHalfKPLayer().getWeight(i, j) / 64.0);

    nWeights += NNUE::Network::LAYER_SIZES[1];
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[1]; i++)
        wd += (network.getLayer1().getBias(i) / (64.0 * 64.0)) * (network.getLayer1().getBias(i) / (64.0 * 64.0));

    nWeights += NNUE::Network::LAYER_SIZES[1] * NNUE::Network::LAYER_SIZES[0];
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[1]; i++)
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[0]; j++)
            wd += (network.getLayer1().getWeight(j, i) / 64.0) * (network.getLayer1().getWeight(j, i) / 64.0);

    nWeights += NNUE::Network::LAYER_SIZES[2];
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[2]; i++)
        wd += (network.getLayer2().getBias(i) / (64.0 * 64.0)) * (network.getLayer2().getBias(i) / (64.0 * 64.0));

    nWeights += NNUE::Network::LAYER_SIZES[2] * NNUE::Network::LAYER_SIZES[1];
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[2]; i++)
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[1]; j++)
            wd += (network.getLayer2().getWeight(j, i) / 64.0) * (network.getLayer2().getWeight(j, i) / 64.0);

    nWeights += NNUE::Network::LAYER_SIZES[3];
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[3]; i++)
        wd += (network.getLayer3().getBias(i) / (64.0 * 64.0)) * (network.getLayer3().getBias(i) / (64.0 * 64.0));

    nWeights += NNUE::Network::LAYER_SIZES[3] * NNUE::Network::LAYER_SIZES[2];
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[3]; i++)
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[2]; j++)
            wd += (network.getLayer3().getWeight(j, i) / 64.0) * (network.getLayer3().getWeight(j, i) / 64.0);

    wd = wd / nWeights;
    return sum.load() + weightDecay * wd;
}

NNUE::Gradients Train::gradient(std::vector<DataPoint>& data, const std::vector<size_t>& indices, const NNUE::MasterWeights& masterWeights, double k, double weightDecay) {
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
                double trueValue = tanh(dp.result, k);

                double errorGrad = 2.0 * (prediction - trueValue) * (1.0 - prediction * prediction) * k;

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

    // Durchschnittsbildung und Hinzufügen des Gewichtungszerfalls
    NNUE::Gradients totalGrad;

    size_t totalParams = masterWeights.exactHalfKPBiases.size() + masterWeights.exactHalfKP.size();
    for(size_t layer = 0; layer < NNUE::Network::NUM_LAYERS; layer++) {
        totalParams += masterWeights.exactDenseLayerBiases[layer].size();
        totalParams += masterWeights.exactDenseLayerWeights[layer].size();
    }

    for(size_t i = 0; i < totalGrad.halfKPBiases.size(); i++) {
        for(size_t t = 0; t < numThreads; t++)
            totalGrad.halfKPBiases[i] += threadGradientAccum[t].halfKPBiases[i];

        totalGrad.halfKPBiases[i] = totalGrad.halfKPBiases[i] / indices.size();
        totalGrad.halfKPBiases[i] += 2.0 * weightDecay * masterWeights.exactHalfKPBiases[i] / (double)totalParams;
    }

    for(size_t t = 0; t < numThreads; t++)
        for(const auto& [feature, value] : threadGradientAccum[t].halfKPWeights)
            totalGrad.halfKPWeights[feature] += value;

    for(const auto& [feature, value] : totalGrad.halfKPWeights) {
        totalGrad.halfKPWeights[feature] = totalGrad.halfKPWeights[feature] / indices.size();
        // "Lazy" L2-Regularisierung: Nur die Gewichte berücksichtigen, die in diesem Batch aktualisiert wurden
        totalGrad.halfKPWeights[feature] += 2.0 * weightDecay * masterWeights.exactHalfKP[feature] / (double)totalParams;
    }

    for(size_t layer = 0; layer < NNUE::Network::NUM_LAYERS; layer++) {
        for(size_t i = 0; i < totalGrad.denseLayerBiases[layer].size(); i++) {
            for(size_t t = 0; t < numThreads; t++)
                totalGrad.denseLayerBiases[layer][i] += threadGradientAccum[t].denseLayerBiases[layer][i];

            totalGrad.denseLayerBiases[layer][i] = totalGrad.denseLayerBiases[layer][i] / indices.size();
            totalGrad.denseLayerBiases[layer][i] += 2.0 * weightDecay * masterWeights.exactDenseLayerBiases[layer][i] / (double)totalParams;
        }

        for(size_t i = 0; i < totalGrad.denseLayerWeights[layer].size(); i++) {
            for(size_t t = 0; t < numThreads; t++)
                totalGrad.denseLayerWeights[layer][i] += threadGradientAccum[t].denseLayerWeights[layer][i];

            totalGrad.denseLayerWeights[layer][i] = totalGrad.denseLayerWeights[layer][i] / indices.size();
            totalGrad.denseLayerWeights[layer][i] += 2.0 * weightDecay * masterWeights.exactDenseLayerWeights[layer][i] / (double)totalParams;
        }
    }

    return totalGrad;
}

NNUE::Network* Train::adam(std::vector<DataPoint>& data, size_t numEpochs, double learningRate) {
    NNUE::MasterWeights& masterWeights = trainingSession.masterWeights;
    NNUE::Network* network = masterWeights.toNetwork();
    NNUE::Network* bestNetwork = network;

    // Teile die Daten in Trainings- und Validierungsdaten auf
    size_t validationSize = data.size() * validationSplit.get<double>();
    std::vector<DataPoint> validationData;
    if(validationSize == 0) {
        validationData = data;
    } else {
        std::shuffle(data.begin(), data.end(), std::default_random_engine(std::rand()));
        validationData = std::vector<DataPoint>(data.end() - validationSize, data.end());
        data.erase(data.end() - validationSize, data.end());
    }

    // Initialisiere den Indexvektor
    std::vector<size_t> indices(std::min(batchSize.get<size_t>(), data.size()));

    // Zufallszahlengenerator
    std::random_device rd;
    std::mt19937 gen(rd());

    size_t patience = 0;

    double bestLoss = std::numeric_limits<double>::infinity();

    size_t targetEpochs = trainingSession.epoch + numEpochs;

    for(; trainingSession.epoch < targetEpochs; trainingSession.epoch++) {
        // Berechne den Fehler
        double masterLoss = Train::loss(validationData, masterWeights, k.get<double>(), weightDecay.get<double>());

        network = masterWeights.toNetwork();
        double networkLoss = Train::loss(validationData, *network, k.get<double>(), weightDecay.get<double>());

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
            bestNetwork = network;
        } else
            patience++;

        // Überprüfe, ob die Geduld erschöpft ist
        if(patience >= noImprovementPatience.get<size_t>()) {
            std::cout << std::endl << "Early stopping at epoch " << trainingSession.epoch << std::endl;
            break;
        }

        // Bestimme zufällige Indizes
        std::uniform_int_distribution<size_t> dist(0, data.size() - 1);
        for(size_t& i : indices)
            i = dist(gen);

        // Berechne die Gradienten
        NNUE::Gradients grad = Train::gradient(data, indices, masterWeights, k.get<double>(), weightDecay.get<double>());

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

        // Aktualisiere die Master-Parameter
        for(size_t i = 0; i < masterWeights.exactHalfKPBiases.size(); i++) {
            double mHat = trainingSession.mHalfKPBiases[i] / (1.0 - std::pow(beta1.get<double>(), trainingSession.epoch + 1));
            double vHat = trainingSession.vHalfKPBiases[i] / (1.0 - std::pow(beta2.get<double>(), trainingSession.epoch + 1));
            masterWeights.exactHalfKPBiases[i] -= learningRate * mHat / (std::sqrt(vHat) + epsilon.get<double>());

            masterWeights.exactHalfKPBiases[i] = std::clamp(masterWeights.exactHalfKPBiases[i],
                NNUE::MasterWeights::HALF_KP_MIN, NNUE::MasterWeights::HALF_KP_MAX);
        }

        // Aktualisiere nur die Gewichte, für die Gradienten != 0 existieren (sparse Update)
        for(const auto& [feature, value] : grad.halfKPWeights) {
             double mHat = trainingSession.mHalfKPWeights[feature] / (1.0 - std::pow(beta1.get<double>(), trainingSession.epoch + 1));
             double vHat = trainingSession.vHalfKPWeights[feature] / (1.0 - std::pow(beta2.get<double>(), trainingSession.epoch + 1));
             masterWeights.exactHalfKP[feature] -= learningRate * mHat / (std::sqrt(vHat) + epsilon.get<double>());

             masterWeights.exactHalfKP[feature] = std::clamp(masterWeights.exactHalfKP[feature],
                 NNUE::MasterWeights::HALF_KP_MIN, NNUE::MasterWeights::HALF_KP_MAX);
        }

        for(size_t layer = 0; layer < NNUE::Network::NUM_LAYERS; layer++) {
            for(size_t i = 0; i < masterWeights.exactDenseLayerBiases[layer].size(); i++) {
                double mHat = trainingSession.mDenseLayerBiases[layer][i] / (1.0 - std::pow(beta1.get<double>(), trainingSession.epoch + 1));
                double vHat = trainingSession.vDenseLayerBiases[layer][i] / (1.0 - std::pow(beta2.get<double>(), trainingSession.epoch + 1));
                masterWeights.exactDenseLayerBiases[layer][i] -= learningRate * mHat / (std::sqrt(vHat) + epsilon.get<double>());

                masterWeights.exactDenseLayerBiases[layer][i] = std::clamp(masterWeights.exactDenseLayerBiases[layer][i],
                    NNUE::MasterWeights::DENSE_BIAS_MIN, NNUE::MasterWeights::DENSE_BIAS_MAX);
            }

            for(size_t i = 0; i < masterWeights.exactDenseLayerWeights[layer].size(); i++) {
                double mHat = trainingSession.mDenseLayerWeights[layer][i] / (1.0 - std::pow(beta1.get<double>(), trainingSession.epoch + 1));
                double vHat = trainingSession.vDenseLayerWeights[layer][i] / (1.0 - std::pow(beta2.get<double>(), trainingSession.epoch + 1));
                masterWeights.exactDenseLayerWeights[layer][i] -= learningRate * mHat / (std::sqrt(vHat) + epsilon.get<double>());

                masterWeights.exactDenseLayerWeights[layer][i] = std::clamp(masterWeights.exactDenseLayerWeights[layer][i],
                    NNUE::MasterWeights::DENSE_WEIGHT_MIN, NNUE::MasterWeights::DENSE_WEIGHT_MAX);
            }
        }
    }

    return network;
}