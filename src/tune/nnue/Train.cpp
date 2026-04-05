#include "tune/nnue/Train.h"
#include "core/engine/evaluation/NNUEEvaluator.h"
#include "core/utils/nnue/NNUEUtils.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <iostream>
#include <thread>

using namespace Train;

TrainingSession trainingSession;

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

NNUE::Network* MasterWeights::toNetwork() const {
    NNUE::Network* network = new NNUE::Network;

    auto& halfKP = network->getHalfKPLayer();

    int16_t* biasPtrHalfKP = (int16_t*)halfKP.getBiasPtr();
    for(size_t i = 0; i < NNUE::Network::SINGLE_SUBNET_SIZE; i++)
        biasPtrHalfKP[i] = (int16_t)(std::round(exactHalfKPBiases[i] * 64.0f));

    for(size_t i = 0; i < NNUE::Network::INPUT_SIZE; i++) {
        int16_t* weightPtr = (int16_t*)halfKP.getWeightPtr(i);
        for(size_t j = 0; j < NNUE::Network::SINGLE_SUBNET_SIZE; j++)
            weightPtr[j] = (int16_t)(std::round(exactHalfKP[i * NNUE::Network::SINGLE_SUBNET_SIZE + j] * 64.0f));
    }

    auto& layer1 = network->getLayer1();
    auto& layer2 = network->getLayer2();
    auto& layer3 = network->getLayer3();

    int32_t* biasPtr = (int32_t*)layer1.getBiasPtr();
    int8_t* weightPtr;
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[1]; i++) {
        biasPtr[i] = (int32_t)(std::round(exactDenseLayerBiases[0][i] * 64.0f * 64.0f));

        weightPtr = (int8_t*)layer1.getWeightPtr(i);
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[0]; j++)
            weightPtr[j] = (int8_t)(std::round(exactDenseLayerWeights[0][i * NNUE::Network::LAYER_SIZES[0] + j] * 64.0f));
    }

    biasPtr = (int32_t*)layer2.getBiasPtr();
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[2]; i++) {
        biasPtr[i] = (int32_t)(std::round(exactDenseLayerBiases[1][i] * 64.0f * 64.0f));

        weightPtr = (int8_t*)layer2.getWeightPtr(i);
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[1]; j++)
            weightPtr[j] = (int8_t)(std::round(exactDenseLayerWeights[1][i * NNUE::Network::LAYER_SIZES[1] + j] * 64.0f));
    }

    biasPtr = (int32_t*)layer3.getBiasPtr();
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[3]; i++) {
        biasPtr[i] = (int32_t)(std::round(exactDenseLayerBiases[2][i] * 64.0f * 64.0f));

        weightPtr = (int8_t*)layer3.getWeightPtr(i);
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[2]; j++)
            weightPtr[j] = (int8_t)(std::round(exactDenseLayerWeights[2][i * NNUE::Network::LAYER_SIZES[2] + j] * 64.0f));
    }

    return network;
}

constexpr float clippedReLU(float x) {
    return std::clamp(x, 0.0f, (float)(std::numeric_limits<int8_t>::max() / 64.0));
}

constexpr bool clippedReLUMask(float x) {
    return (x > 0.0f && x < (float)(std::numeric_limits<int8_t>::max() / 64.0f));
}

constexpr float clippedReLUGrad(float x) {
    return clippedReLUMask(x) ? 1.0f : 0.0f;
}

float NetworkActivations::forward(const MasterWeights& masterWeights, const Board& board) {
    constexpr size_t SUBNET_SIZE = NNUE::Network::SINGLE_SUBNET_SIZE;

    int color = board.getSideToMove();
    int oppColor = color ^ COLOR_MASK;

    // Berechnung der HalfKP-Features
    Array<int32_t, 63> activeFeatures = NNUE::getHalfKPFeatures(board, color);
    Array<int32_t, 63> oppActiveFeatures = NNUE::getHalfKPFeatures(board, oppColor);

    // Biases
    std::copy(masterWeights.exactHalfKPBiases.begin(), masterWeights.exactHalfKPBiases.end(), halfKPOutputs.begin());
    std::copy(masterWeights.exactHalfKPBiases.begin(), masterWeights.exactHalfKPBiases.end(), halfKPOutputs.begin() + SUBNET_SIZE);

    // Gewichte
    for(int32_t feature : activeFeatures)
        for(size_t i = 0; i < SUBNET_SIZE; i++)
            halfKPOutputs[i] += masterWeights.exactHalfKP[feature * SUBNET_SIZE + i];

    for(int32_t feature : oppActiveFeatures)
        for(size_t i = 0; i < SUBNET_SIZE; i++)
            halfKPOutputs[i + SUBNET_SIZE] += masterWeights.exactHalfKP[feature * SUBNET_SIZE + i];

    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[0]; i++)
        halfKPOutputs[i] = clippedReLU(halfKPOutputs[i]);

    // Berechnung der Dense-Layer-Ausgaben

    // Biases
    std::copy(masterWeights.exactDenseLayerBiases[0].begin(), masterWeights.exactDenseLayerBiases[0].end(), denseLayerOutputs[0].begin());

    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[1]; i++) {
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[0]; j++)
            denseLayerOutputs[0][i] += masterWeights.exactDenseLayerWeights[0][i * NNUE::Network::LAYER_SIZES[0] + j] * halfKPOutputs[j];

        denseLayerOutputs[0][i] = clippedReLU(denseLayerOutputs[0][i]);
    }

    // Biases
    std::copy(masterWeights.exactDenseLayerBiases[1].begin(), masterWeights.exactDenseLayerBiases[1].end(), denseLayerOutputs[1].begin());

    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[2]; i++) {
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[1]; j++)
            denseLayerOutputs[1][i] += masterWeights.exactDenseLayerWeights[1][i * NNUE::Network::LAYER_SIZES[1] + j] * denseLayerOutputs[0][j];

        denseLayerOutputs[1][i] = clippedReLU(denseLayerOutputs[1][i]);
    }

    // Biases
    std::copy(masterWeights.exactDenseLayerBiases[2].begin(), masterWeights.exactDenseLayerBiases[2].end(), denseLayerOutputs[2].begin());

    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[3]; i++) {
        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[2]; j++)
            denseLayerOutputs[2][i] += masterWeights.exactDenseLayerWeights[2][i * NNUE::Network::LAYER_SIZES[2] + j] * denseLayerOutputs[1][j];
    }

    return denseLayerOutputs[2][0];
}

Gradients NetworkActivations::backward(const MasterWeights& masterWeights, const Board& board, float outputGrad) {
    Gradients gradients;
    std::array<float, NNUE::Network::LAYER_SIZES[0]> halfKPGrad;

    // Layer 3
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[3]; i++) {
        gradients.denseLayerBiases[2][i] = outputGrad;

        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[2]; j++)
            gradients.denseLayerWeights[2][i * NNUE::Network::LAYER_SIZES[2] + j] = outputGrad * denseLayerOutputs[1][j];
    }

    // Layer 2
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[2]; i++) {
        if(!clippedReLUMask(denseLayerOutputs[1][i]))
            continue;

        float grad = 0.0f;

        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[3]; j++)
            grad += masterWeights.exactDenseLayerWeights[2][j * NNUE::Network::LAYER_SIZES[2] + i] * gradients.denseLayerBiases[2][j];

        gradients.denseLayerBiases[1][i] = grad;

        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[1]; j++)
            gradients.denseLayerWeights[1][i * NNUE::Network::LAYER_SIZES[1] + j] = grad * denseLayerOutputs[0][j];
    }

    // Layer 1
    for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[1]; i++) {
        if(!clippedReLUMask(denseLayerOutputs[0][i]))
            continue;

        float grad = 0.0f;

        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[2]; j++)
            grad += masterWeights.exactDenseLayerWeights[1][j * NNUE::Network::LAYER_SIZES[1] + i] * gradients.denseLayerBiases[1][j];

        gradients.denseLayerBiases[0][i] = grad;

        for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[0]; j++)
            gradients.denseLayerWeights[0][i * NNUE::Network::LAYER_SIZES[0] + j] = grad * halfKPOutputs[j];
    }

    // Gradient für die Ausgaben der HalfKP-Schicht: dL/d(halfKPOut)
    for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[0]; j++) {
        float grad = 0.0f;

        if(!clippedReLUMask(halfKPOutputs[j]))
            continue;

        for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[1]; i++)
            grad += masterWeights.exactDenseLayerWeights[0][i * NNUE::Network::LAYER_SIZES[0] + j] * gradients.denseLayerBiases[0][i];

        halfKPGrad[j] = grad;
    }

    // HalfKP-Layer
    // Berechnung der HalfKP-Features
    int color = board.getSideToMove();
    int oppColor = color ^ COLOR_MASK;
    Array<int32_t, 63> activeFeatures = NNUE::getHalfKPFeatures(board, color);
    Array<int32_t, 63> oppActiveFeatures = NNUE::getHalfKPFeatures(board, oppColor);

    // Bias ist in beiden Subnetzen derselbe Vektor (256): Gradienten beider Perspektiven aufsummieren.
    for(size_t i = 0; i < NNUE::Network::SINGLE_SUBNET_SIZE; i++)
        gradients.halfKPBiases[i] = halfKPGrad[i] + halfKPGrad[i + NNUE::Network::SINGLE_SUBNET_SIZE];

    for(int32_t feature : activeFeatures) {
        for(size_t i = 0; i < NNUE::Network::SINGLE_SUBNET_SIZE; i++)
            gradients.halfKPWeights[feature * NNUE::Network::SINGLE_SUBNET_SIZE + i] += halfKPGrad[i];
    }

    for(int32_t feature : oppActiveFeatures) {
        for(size_t i = 0; i < NNUE::Network::SINGLE_SUBNET_SIZE; i++)
            gradients.halfKPWeights[feature * NNUE::Network::SINGLE_SUBNET_SIZE + i] += halfKPGrad[i + NNUE::Network::SINGLE_SUBNET_SIZE];
    }

    return gradients;
}

double networkOutputToCentipawns(float output) {
    return (output * (64.0 * 64.0) * 100.0 / 3328.0);
}

double Train::loss(std::vector<DataPoint>& data, const MasterWeights& masterWeights, double k, double weightDecay) {
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

                float networkOutput = NetworkActivations().forward(masterWeights, dp.board);
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

Gradients Train::gradient(std::vector<DataPoint>& data, const std::vector<size_t>& indices, const MasterWeights& masterWeights, double k, double weightDecay) {
    size_t currIndex = 0;
    std::mutex mutex;

    size_t numThreads = std::max(std::thread::hardware_concurrency(), 1u);
    std::vector<Gradients> threadGradientAccum;
    threadGradientAccum.resize(numThreads);

    auto threadFunc = [&](size_t threadId) {
        Gradients& grad = threadGradientAccum[threadId];
        NetworkActivations activations;

        mutex.lock();
        while(currIndex < indices.size()) {
            // Bearbeite Blöcke von 32 Datenpunkten
            size_t start = currIndex;
            size_t end = std::min(currIndex + 32, indices.size());
            currIndex = end;
            mutex.unlock();

            for(size_t i = start; i < end; i++) {
                size_t dataIndex = indices[i];
                DataPoint& dp = data[dataIndex];

                float networkOutput = activations.forward(masterWeights, dp.board);
                double prediction = tanh(networkOutputToCentipawns(networkOutput), k);
                double trueValue = tanh(dp.result, k);

                double errorGrad = 2.0 * (prediction - trueValue) * (1.0 - prediction * prediction) * k;

                // Berechne die Gradienten für die Master-Parameter und addiere sie zum Thread-Gradienten
                Gradients dpGrad = activations.backward(masterWeights, dp.board, errorGrad);

                for(size_t j = 0; j < grad.halfKPBiases.size(); j++)
                    grad.halfKPBiases[j] += dpGrad.halfKPBiases[j];

                for(size_t j = 0; j < grad.halfKPWeights.size(); j++)
                    grad.halfKPWeights[j] += dpGrad.halfKPWeights[j];

                for(size_t layer = 0; layer < NNUE::Network::NUM_LAYERS; layer++) {
                    for(size_t j = 0; j < grad.denseLayerBiases[layer].size(); j++)
                        grad.denseLayerBiases[layer][j] += dpGrad.denseLayerBiases[layer][j];

                    for(size_t j = 0; j < grad.denseLayerWeights[layer].size(); j++)
                        grad.denseLayerWeights[layer][j] += dpGrad.denseLayerWeights[layer][j];
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
    Gradients totalGrad;

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

    for(size_t i = 0; i < totalGrad.halfKPWeights.size(); i++) {
        for(size_t t = 0; t < numThreads; t++)
            totalGrad.halfKPWeights[i] += threadGradientAccum[t].halfKPWeights[i];

        totalGrad.halfKPWeights[i] = totalGrad.halfKPWeights[i] / indices.size();
        totalGrad.halfKPWeights[i] += 2.0 * weightDecay * masterWeights.exactHalfKP[i] / (double)totalParams;
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