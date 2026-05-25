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

Train::LossSummary Train::loss(const std::vector<DataPoint>& data, const REN::MasterWeights& masterWeights, double k,
    double kappa, double encLossWeight, size_t iterationLimit) {

    std::atomic<double> sum = 0.0;
    std::atomic<uint64_t> avgIterations = 0;
    uint64_t minIterations = std::numeric_limits<uint64_t>::max();
    uint64_t maxIterations = 0;

    size_t currIndex = 0;
    std::mutex mutex;

    auto threadFunc = [&]() {
        mutex.lock();
        while(currIndex < data.size()) {
            // Bearbeite Blöcke von 16 Datenpunkten
            size_t start = currIndex;
            size_t end = std::min(currIndex + 16, data.size());
            currIndex = end;
            mutex.unlock();

            uint64_t localMinIterations = std::numeric_limits<uint64_t>::max();
            uint64_t localMaxIterations = 0;

            for(size_t i = start; i < end; i++) {
                const DataPoint& dp = data[i];

                REN::NetworkActivations activations = masterWeights.forward(dp.board, true, iterationLimit);
                float networkOutput = activations.output();
                double prediction = tanh(networkOutputToCentipawns(networkOutput), k);

                double encPrediction = masterWeights.outputLayer.forward(activations.halfKPActivations.output, true).output(0);
                encPrediction = tanh(networkOutputToCentipawns(encPrediction), k);

                double target = (1.0 - kappa) * tanh(dp.tdTarget, k) + kappa * (double)dp.finalResult;
                if(dp.board.getSideToMove() == BLACK)
                    target = -target;

                sum.fetch_add(mse(prediction, target) + encLossWeight * mse(encPrediction, target));
                avgIterations.fetch_add(activations.renActivations.iterations);
                localMinIterations = std::min(localMinIterations, activations.renActivations.iterations);
                localMaxIterations = std::max(localMaxIterations, activations.renActivations.iterations);
            }

            mutex.lock();

            minIterations = std::min(minIterations, localMinIterations);
            maxIterations = std::max(maxIterations, localMaxIterations);
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

    return {sum.load() / data.size(), avgIterations.load() / data.size(), minIterations, maxIterations};
}

constexpr double lossGrad(double prediction, double target, double k) {
    return 2.0 * (prediction - target) * (1.0 - prediction * prediction) * k * 100.0 * (16384.0 / 6656.0);
}

REN::Gradients Train::gradient(const std::vector<DataPoint>& data, const std::vector<size_t>& indices, const REN::MasterWeights& masterWeights,
    double k, double kappa, double encLossWeight) {

    size_t currIndex = 0;
    std::mutex mutex;

    size_t numThreads = std::max(std::thread::hardware_concurrency(), 1u);
    std::vector<REN::Gradients> threadGradientAccum;
    threadGradientAccum.resize(numThreads);

    auto threadFunc = [&](size_t threadId) {
        REN::Gradients& grads = threadGradientAccum[threadId];

        mutex.lock();
        while(currIndex < indices.size()) {
            // Bearbeite Blöcke von 16 Datenpunkten
            size_t start = currIndex;
            size_t end = std::min(currIndex + 16, indices.size());
            currIndex = end;
            mutex.unlock();

            for(size_t i = start; i < end; i++) {
                size_t dataIndex = indices[i];
                const DataPoint& dp = data[dataIndex];

                REN::NetworkActivations activations = masterWeights.forward(dp.board, true);
                float networkOutput = activations.output();
                double cp = networkOutputToCentipawns(networkOutput);
                double prediction = tanh(cp, k);

                ML::DenseLayer::ForwardResult encActivations = masterWeights.outputLayer.forward(activations.halfKPActivations.output, true);
                double encOutput = encActivations.output(0);
                double encCp = networkOutputToCentipawns(encOutput);
                double encPrediction = tanh(encCp, k);

                double target = (1.0 - kappa) * tanh(dp.tdTarget, k) + kappa * (double)dp.finalResult;
                if(dp.board.getSideToMove() == BLACK)
                    target = -target;

                double errorGrad = lossGrad(prediction, target, k);
                double encErrorGrad = lossGrad(encPrediction, target, k) * encLossWeight;

                // Berechne die Gradienten für die Master-Parameter und addiere sie zum Thread-Gradienten
                REN::Gradients dpGrad = masterWeights.backward(dp.board, activations, encActivations, errorGrad, encErrorGrad, true);

                grads.halfKAGradients.bias += dpGrad.halfKAGradients.bias;

                for(const auto& [feature, value] : dpGrad.halfKAGradients.weights)
                    grads.halfKAGradients.weights[feature] += value;

                grads.renGradients.bias += dpGrad.renGradients.bias;
                for(size_t j = 0; j < dpGrad.renGradients.q.size(); j++)
                    grads.renGradients.q[j] += dpGrad.renGradients.q[j];
                grads.renGradients.gammaRaw += dpGrad.renGradients.gammaRaw;

                grads.outputLayerGradients.bias += dpGrad.outputLayerGradients.bias;
                grads.outputLayerGradients.weights += dpGrad.outputLayerGradients.weights;
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

    // Durchschnittsbildung für die Gradienten des Threads
    REN::Gradients totalGrad;

    for(size_t i = 0; i < numThreads; i++) {
        totalGrad.halfKAGradients.bias += threadGradientAccum[i].halfKAGradients.bias;
        for(const auto& [feature, value] : threadGradientAccum[i].halfKAGradients.weights)
            totalGrad.halfKAGradients.weights[feature] += value;

        totalGrad.renGradients.bias += threadGradientAccum[i].renGradients.bias;
        for(size_t j = 0; j < threadGradientAccum[i].renGradients.q.size(); j++)
            totalGrad.renGradients.q[j] += threadGradientAccum[i].renGradients.q[j];
        totalGrad.renGradients.gammaRaw += threadGradientAccum[i].renGradients.gammaRaw;

        totalGrad.outputLayerGradients.bias += threadGradientAccum[i].outputLayerGradients.bias;
        totalGrad.outputLayerGradients.weights += threadGradientAccum[i].outputLayerGradients.weights;
    }

    totalGrad.halfKAGradients.bias /= indices.size();
    for(const auto& [feature, value] : totalGrad.halfKAGradients.weights)
        totalGrad.halfKAGradients.weights[feature] = totalGrad.halfKAGradients.weights[feature] / indices.size();

    totalGrad.renGradients.bias /= indices.size();
    for(size_t j = 0; j < totalGrad.renGradients.q.size(); j++)
        totalGrad.renGradients.q[j] /= indices.size();
    totalGrad.renGradients.gammaRaw /= indices.size();

    totalGrad.outputLayerGradients.bias /= indices.size();
    totalGrad.outputLayerGradients.weights /= indices.size();

    return totalGrad;
}

void Train::adamW(std::vector<DataPoint>& data, size_t numEpochs, double learningRate, double kappa, double encLossWeight) {
    REN::MasterWeights& masterWeights = trainingSession.masterWeights;

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
    std::vector<size_t> indices(data.size());
    for(size_t i = 0; i < indices.size(); i++)
        indices[i] = i;

    std::mt19937& rng = Random::generator<11>();

    size_t patience = 0;

    double bestLoss = std::numeric_limits<double>::infinity();

    size_t targetEpochs = trainingSession.epoch + numEpochs;

    for(; trainingSession.epoch < targetEpochs; trainingSession.epoch++) {
        // Berechne den Fehler
        auto [masterLossExact, avgIterations, minIterations, maxIterations] = Train::loss(validationData, masterWeights, k.get<double>(), kappa, encLossWeight);

        // Überprüfe, ob der Fehler besser ist
        if(masterLossExact < bestLoss) {
            bestLoss = masterLossExact;
            patience = 0;
        } else
            patience++;

        // Überprüfe, ob die Geduld erschöpft ist
        if(patience >= noImprovementPatience.get<size_t>()) {
            std::cout << std::endl << "Early stopping at epoch " << trainingSession.epoch << std::endl;
            break;
        }

        // Shuffle die Trainingsdaten für die nächste Epoche
        std::shuffle(indices.begin(), indices.end(), rng);

        // Zerlege die Trainingsdaten in Batches
        size_t numBatches = indices.size() / batchSize.get<size_t>();
        if(indices.size() % batchSize.get<size_t>() != 0)
            numBatches++;

        std::vector<std::vector<size_t>> batches(numBatches);
        for(size_t i = 0; i < indices.size(); i++)
            batches[i / batchSize.get<size_t>()].push_back(indices[i]);

        size_t batchesProcessed = 0;

        float loss0It = Train::loss(validationData, masterWeights, k.get<double>(), kappa, 0.0, 0).loss;
        float loss2It = Train::loss(validationData, masterWeights, k.get<double>(), kappa, 0.0, 2).loss;
        float lossOpt = Train::loss(validationData, masterWeights, k.get<double>(), kappa, 0.0).loss;
        float spectralRadius = masterWeights.renLayer.spectralRadius();

        std::stringstream ssLoss;
        ssLoss << std::setprecision(6) << loss0It << "/" << loss2It << "/" << lossOpt;

        std::stringstream ssIter;
        ssIter << std::setprecision(2) << avgIterations << "/" << minIterations << "/" << maxIterations;

        std::stringstream ssOutput;

        ssOutput << "\rEpoch: " << std::left << std::setw(6) << trainingSession.epoch <<
            " Val loss: " << std::setw(9) << std::setprecision(6) << masterLossExact <<
            " Loss (0/2/inf it): " << std::setw(27) << ssLoss.str() <<
            " Iter (avg/min/max): " << std::setw(12) << ssIter.str() <<
            " Spec rad: " << std::setw(8) << std::setprecision(4) << spectralRadius;

        std::cout << ssOutput.str() << std::endl;

        // Berechne die Gradienten für alle Batches und aktualisiere die Master-Parameter mit AdamW
        for(const std::vector<size_t>& batchIndices : batches) {
            REN::Gradients grad = Train::gradient(data, batchIndices, masterWeights, k.get<double>(), kappa, encLossWeight);

            batchesProcessed++;
            double batchProgress = (double)batchesProcessed / numBatches * 100.0;
            std::cout << "\rBatch: "  << std::setw(3) << (int)batchProgress << "%" << std::flush;

            // Aktualisiere die Master-Parameter mit AdamW

            double wd = weightDecay.get<double>();
            double b1 = beta1.get<double>();
            double b2 = beta2.get<double>();
            double eps = epsilon.get<double>();

            // HalfKAv2_hm-Layer
            for(const auto& [feature, value] : grad.halfKAGradients.weights) {
                // Anzahl der Schritte seit dem letzten Update dieses Features (inklusive aktuellem Schritt)
                size_t tDelta = trainingSession.epoch - trainingSession.lastUpdateHalfKPWeights[feature] + 1;
                trainingSession.lastUpdateHalfKPWeights[feature] = trainingSession.epoch + 1;

                trainingSession.mHalfKPWeights[feature] = std::pow(b1, tDelta) * trainingSession.mHalfKPWeights[feature] + (1.0 - b1) * value;
                trainingSession.vHalfKPWeights[feature] = std::pow(b2, tDelta) * trainingSession.vHalfKPWeights[feature] + (1.0 - b2) * value * value;

                double mHat = trainingSession.mHalfKPWeights[feature] / (1.0 - std::pow(b1, trainingSession.epoch + 1));
                double vHat = trainingSession.vHalfKPWeights[feature] / (1.0 - std::pow(b2, trainingSession.epoch + 1));

                // Lazy AdamW: trage den verpassten Decay exakt nach, bevor der Gradienten-Schritt angewendet wird.
                double decayFactor = std::pow(1.0 - learningRate * wd, tDelta);

                masterWeights.halfKAv2Layer.weights(feature) = masterWeights.halfKAv2Layer.weights(feature) * decayFactor
                    - learningRate * mHat / (std::sqrt(vHat) + eps);
            }

            for(size_t i = 0; i < grad.halfKAGradients.bias.size; i++) {
                trainingSession.mHalfKPBiases[i] = b1 * trainingSession.mHalfKPBiases[i] + (1.0 - b1) * grad.halfKAGradients.bias(i);
                trainingSession.vHalfKPBiases[i] = b2 * trainingSession.vHalfKPBiases[i] + (1.0 - b2) * grad.halfKAGradients.bias(i) * grad.halfKAGradients.bias(i);

                double mHat = trainingSession.mHalfKPBiases[i] / (1.0 - std::pow(b1, trainingSession.epoch + 1));
                double vHat = trainingSession.vHalfKPBiases[i] / (1.0 - std::pow(b2, trainingSession.epoch + 1));

                masterWeights.halfKAv2Layer.bias(i) = masterWeights.halfKAv2Layer.bias(i) * (1.0 - learningRate * wd) 
                    - learningRate * mHat / (std::sqrt(vHat) + eps);
            }

            // REN-Layer

            for(size_t k = 0; k < grad.renGradients.q.size(); k++) {
                for(size_t i = 0; i < grad.renGradients.q[k].size; i++) {
                    trainingSession.mRENQWeights[i] = b1 * trainingSession.mRENQWeights[i] + (1.0 - b1) * grad.renGradients.q[k](i);
                    trainingSession.vRENQWeights[i] = b2 * trainingSession.vRENQWeights[i] + (1.0 - b2) * grad.renGradients.q[k](i) * grad.renGradients.q[k](i);

                    double mHat = trainingSession.mRENQWeights[i] / (1.0 - std::pow(b1, trainingSession.epoch + 1));
                    double vHat = trainingSession.vRENQWeights[i] / (1.0 - std::pow(b2, trainingSession.epoch + 1));

                    masterWeights.renLayer.surrogateWeights.q[k](i) = masterWeights.renLayer.surrogateWeights.q[k](i) * (1.0 - learningRate * wd) 
                        - learningRate * mHat / (std::sqrt(vHat) + eps);
                }
            }

            for(size_t i = 0; i < grad.renGradients.gammaRaw.size; i++) {
                trainingSession.mRENGammaRawWeights[i] = b1 * trainingSession.mRENGammaRawWeights[i] + (1.0 - b1) * grad.renGradients.gammaRaw(i);
                trainingSession.vRENGammaRawWeights[i] = b2 * trainingSession.vRENGammaRawWeights[i] + (1.0 - b2) * grad.renGradients.gammaRaw(i) * grad.renGradients.gammaRaw(i);

                double mHat = trainingSession.mRENGammaRawWeights[i] / (1.0 - std::pow(b1, trainingSession.epoch + 1));
                double vHat = trainingSession.vRENGammaRawWeights[i] / (1.0 - std::pow(b2, trainingSession.epoch + 1));

                masterWeights.renLayer.surrogateWeights.gammaRaw(i) = masterWeights.renLayer.surrogateWeights.gammaRaw(i) * (1.0 - learningRate * wd) 
                    - learningRate * mHat / (std::sqrt(vHat) + eps);
            }

            for(size_t i = 0; i < grad.renGradients.bias.size; i++) {
                trainingSession.mRENBiases[i] = b1 * trainingSession.mRENBiases[i] + (1.0 - b1) * grad.renGradients.bias(i);
                trainingSession.vRENBiases[i] = b2 * trainingSession.vRENBiases[i] + (1.0 - b2) * grad.renGradients.bias(i) * grad.renGradients.bias(i);

                double mHat = trainingSession.mRENBiases[i] / (1.0 - std::pow(b1, trainingSession.epoch + 1));
                double vHat = trainingSession.vRENBiases[i] / (1.0 - std::pow(b2, trainingSession.epoch + 1));

                masterWeights.renLayer.bias(i) = masterWeights.renLayer.bias(i) * (1.0 - learningRate * wd) 
                    - learningRate * mHat / (std::sqrt(vHat) + eps);
            }

            masterWeights.renLayer.constructTransform();
            masterWeights.renLayer.normalize(maxSpectralRadius.get<double>());

            // Output-Layer

            for(size_t i = 0; i < grad.outputLayerGradients.weights.size; i++) {
                trainingSession.mOutputWeights[i] = b1 * trainingSession.mOutputWeights[i] + (1.0 - b1) * grad.outputLayerGradients.weights(i);
                trainingSession.vOutputWeights[i] = b2 * trainingSession.vOutputWeights[i] + (1.0 - b2) * grad.outputLayerGradients.weights(i) * grad.outputLayerGradients.weights(i);

                double mHat = trainingSession.mOutputWeights[i] / (1.0 - std::pow(b1, trainingSession.epoch + 1));
                double vHat = trainingSession.vOutputWeights[i] / (1.0 - std::pow(b2, trainingSession.epoch + 1));

                masterWeights.outputLayer.weights(i) = masterWeights.outputLayer.weights(i) * (1.0 - learningRate * wd) 
                    - learningRate * mHat / (std::sqrt(vHat) + eps);
            }

            for(size_t i = 0; i < grad.outputLayerGradients.bias.size; i++) {
                trainingSession.mOutputBiases[i] = b1 * trainingSession.mOutputBiases[i] + (1.0 - b1) * grad.outputLayerGradients.bias(i);
                trainingSession.vOutputBiases[i] = b2 * trainingSession.vOutputBiases[i] + (1.0 - b2) * grad.outputLayerGradients.bias(i) * grad.outputLayerGradients.bias(i);

                double mHat = trainingSession.mOutputBiases[i] / (1.0 - std::pow(b1, trainingSession.epoch + 1));
                double vHat = trainingSession.vOutputBiases[i] / (1.0 - std::pow(b2, trainingSession.epoch + 1));

                masterWeights.outputLayer.bias(i) = masterWeights.outputLayer.bias(i) * (1.0 - learningRate * wd) 
                    - learningRate * mHat / (std::sqrt(vHat) + eps);
            }
        }
    }

    // Berechne den finalen Fehler
    auto [masterLossExact, avgIterations, minIterations, maxIterations] = Train::loss(validationData, masterWeights, k.get<double>(), kappa, encLossWeight);
    float loss0It = Train::loss(validationData, masterWeights, k.get<double>(), kappa, 0.0, 0).loss;
    float loss2It = Train::loss(validationData, masterWeights, k.get<double>(), kappa, 0.0, 2).loss;
    float lossOpt = Train::loss(validationData, masterWeights, k.get<double>(), kappa, 0.0).loss;
    float spectralRadius = masterWeights.renLayer.spectralRadius();
    
    std::cout << "\rEpoch: " << std::left << std::setw(6) << trainingSession.epoch;

    std::stringstream ssLoss;
    ssLoss << std::setprecision(6) << loss0It << "/" << loss2It << "/" << lossOpt;
    std::stringstream ssIter;
    ssIter << std::setprecision(2) << avgIterations << "/" << minIterations << "/" << maxIterations;

    size_t currPrecision = std::cout.precision();
    std::cout << " Val loss: " << std::setw(9) << std::setprecision(6) << masterLossExact <<
        " Loss (0/2/inf it): " << std::setw(27) << ssLoss.str() <<
        " Iter (avg/min/max): " << std::setw(12) << ssIter.str() <<
        " Spectral radius: " << std::setw(8) << std::setprecision(4) << spectralRadius <<
        " Batch: 100%" << std::endl;

    std::cout.precision(currPrecision);

    if(masterLossExact < bestLoss)
        bestLoss = masterLossExact;

    trainingSession.generation++;
    trainingSession.averageLoss = trainingSession.averageLoss * alpha.get<double>() + (1.0 - alpha.get<double>()) * bestLoss;
    // Bias-korrigierter Loss
    double biasCorrectedLoss = trainingSession.averageLoss / (1 - std::pow(alpha.get<double>(), trainingSession.generation));

    std::cout << "Average loss: " << std::setprecision(6) << biasCorrectedLoss << std::endl;
    std::cout.precision(currPrecision);
}

void Train::initializeWeights(REN::MasterWeights& masterWeights) {
    std::mt19937& rng = Random::generator<12>();
    float fanIn = 64.0f, fanOut = REN::REN_SIZE;
    std::normal_distribution<float> distHalfKA(0.0f, std::sqrt(2.0f / (fanIn + fanOut)));
    fanIn = REN::REN_SIZE;
    std::normal_distribution<float> distRENQ(0.0f, std::sqrt(2.0f / (fanIn + fanOut)));
    fanOut = 1.0f;
    std::normal_distribution<float> distDense(0.0f, std::sqrt(2.0f / (fanIn + fanOut)));

    for(size_t i = 0; i < masterWeights.halfKAv2Layer.weights.size; i++)
        masterWeights.halfKAv2Layer.weights(i) = distHalfKA(rng);

    for(size_t i = 0; i < masterWeights.halfKAv2Layer.bias.size; i++)
        masterWeights.halfKAv2Layer.bias(i) = 0.2f;

    for(size_t i = 0; i < masterWeights.outputLayer.weights.size; i++)
        masterWeights.outputLayer.weights(i) = distDense(rng);

    for(size_t i = 0; i < masterWeights.outputLayer.bias.size; i++)
        masterWeights.outputLayer.bias(i) = 0.0f;

    // Initialisiere die REN-Schicht so, dass sie zu Beginn fast eine Identitätsfunktion darstellt
    for(size_t k = 0; k < masterWeights.renLayer.surrogateWeights.q.size(); k++)
        for(size_t i = 0; i < masterWeights.renLayer.surrogateWeights.q[k].size; i++)
            masterWeights.renLayer.surrogateWeights.q[k](i) = distRENQ(rng);

    for(size_t i = 0; i < masterWeights.renLayer.surrogateWeights.gammaRaw.size; i++)
        masterWeights.renLayer.surrogateWeights.gammaRaw(i) = -3.0f;

    for(size_t i = 0; i < masterWeights.renLayer.bias.size; i++)
        masterWeights.renLayer.bias(i) = 0.0f;

    masterWeights.renLayer.constructTransform();
    masterWeights.renLayer.normalize(maxSpectralRadius.get<double>());
}