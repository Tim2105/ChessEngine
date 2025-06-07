#include "core/engine/search/PVSSearchInstance.h"
#include "tune/hce/Definitions.h"
#include "tune/hce/Tune.h"

#include <atomic>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>

namespace Tune {
    TrainingSession trainingSession(HCE_PARAMS);

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

    double loss(std::vector<DataPoint>& data, const std::vector<size_t>& indices, const HCEParameters& hceParams, double k, double discount, double weightDecay) {
        double sum = 0;

        for(size_t i : indices) {
            // Extrahiere den Datenpunkt
            DataPoint& dp = data[i % data.size()];

            // Setze das Schachbrett auf die aktuelle Position
            HandcraftedEvaluator evaluator(dp.board, hceParams);

            // Berechne die Vorhersage
            double prediction = tanh(evaluator.evaluate(), k);

            // Berechne den wahren Wert
            double trueValue;
            if(dp.result == 0)
                trueValue = 0.0;
            else if(dp.result > 0)
                trueValue = std::pow(discount, dp.result);
            else
                trueValue = -std::pow(discount, -dp.result);

            // Berechne den Fehler
            sum += mse(prediction, trueValue);
        }

        sum /= indices.size();

        // Berechne den Gewichtungsabfall
        double wd = 0;
        for(size_t i = 0; i < hceParams.size(); i++)
            if(hceParams.isOptimizable(i))
                wd += hceParams[i] * hceParams[i];

        return sum + weightDecay * wd;
    }

    double loss(std::vector<DataPoint>& data, const HCEParameters& hceParams, double k, double discount, double weightDecay) {
        std::atomic<double> sum = 0;

        size_t currIndex = 0;
        std::mutex mutex;

        auto threadFunc = [&]() {
            // Sperre den Mutex um die zu bearbeitenden Datenpunkte zu extrahieren
            mutex.lock();
            while(currIndex < data.size()) {
                // Bearbeite immer Blöcke von 1024 Datenpunkten
                size_t start = currIndex;
                size_t end = std::min(currIndex + 1024, data.size());
                currIndex = end;
                mutex.unlock();

                // Iteriere über die Datenpunkte
                for(size_t i = start; i < end; i++) {
                    // Extrahiere den Datenpunkt
                    DataPoint& dp = data[i];

                    // Setze das Schachbrett auf die aktuelle Position
                    HandcraftedEvaluator evaluator(dp.board, hceParams);

                    // Berechne die Vorhersage
                    double prediction = tanh(evaluator.evaluate(), k);

                    // Berechne den wahren Wert
                    double trueValue;
                    if(dp.result == 0)
                        trueValue = 0.0;
                    else if(dp.result > 0)
                        trueValue = std::pow(discount, dp.result);
                    else
                        trueValue = -std::pow(discount, -dp.result);

                    // Berechne den Fehler
                    double l = mse(prediction, trueValue);

                    // Addiere den Fehler zum Gesamtfehler
                    sum.fetch_add(l);
                }

                // Sperre den Mutex um die nächsten Datenpunkte zu extrahieren
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

        // Berechne den Gewichtungsabfall
        double wd = 0;
        for(size_t i = 0; i < hceParams.size(); i++)
            if(hceParams.isOptimizable(i))
                wd += hceParams[i] * hceParams[i];

        return sum + weightDecay * wd;
    }

    std::vector<double> gradient(std::vector<DataPoint>& data, const std::vector<size_t>& indices, const HCEParameters& hceParams, double k, double discount) {
        // Rückgabevektor initialisieren
        std::vector<double> grad(hceParams.size(), 0);

        size_t currIndex = 0;
        std::mutex mutex;

        // Berechne den aktuellen Fehler der zu betrachtenden Datenpunkte
        double currLoss = loss(data, indices, hceParams, k, discount);

        auto threadFunc = [&]() {
            // Sperre den Mutex um den nächsten Parameter zu extrahieren
            mutex.lock();
            while(currIndex < HCEParameters::size()) {
                size_t i = currIndex++;

                // Überspringe nicht optimierbare Parameter
                if(!hceParams.isOptimizable(i))
                    continue;

                mutex.unlock();

                // Bestimme den Gradienten über die Ableitungsdefinition
                // f'(x) = (f(x + h) - f(x)) / h
                HCEParameters hceParamsCopy = hceParams;
                hceParamsCopy[i] += 1;
                hceParamsCopy.unpackPSQT();

                double l = loss(data, indices, hceParamsCopy, k, discount);
                grad[i] = l - currLoss;

                // Sperre den Mutex um den nächsten Parameter zu extrahieren
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

        return grad;
    }

    HCEParameters adamW(std::vector<DataPoint>& data, const HCEParameters& hceParams, size_t numEpochs, double learningRate) {
        HCEParameters currentParams = hceParams;
        HCEParameters bestParams = hceParams;

        // Teile die Daten in Trainings- und Validierungsdaten auf
        size_t validationSize = data.size() * validationSplit.get<double>();
        std::vector<DataPoint> validationData;
        if(validationSize == 0) {
            validationData = data;
        } else {
            std::random_shuffle(data.begin(), data.end());
            validationData = std::vector<DataPoint>(data.end() - validationSize, data.end());
            data.erase(data.end() - validationSize, data.end());
        }
        
        // Initialisiere den Gradienten
        std::vector<double> grad(hceParams.size(), 0);

        // Initialisiere den Indexvektor
        std::vector<size_t> indices(std::min(batchSize.get<size_t>(), data.size()));

        // Bestimme einen Zufallszahlengenerator
        std::random_device rd;
        std::mt19937 gen(rd());

        // Initialisiere den Zähler für die Geduld
        size_t patience = 0;

        // Initialisiere den besten Fehler
        double bestLoss = std::numeric_limits<double>::infinity();

        size_t targetEpochs = trainingSession.epoch + numEpochs;

        // Iteriere über die Epochen
        for(; trainingSession.epoch < targetEpochs; trainingSession.epoch++) {
            // Berechne den Fehler
            double loss = Tune::loss(validationData, currentParams, k.get<double>(), discount.get<double>(), weightDecay.get<double>());

            std::cout << "\rEpoch: " << std::left << std::setw(4) << trainingSession.epoch;
            size_t currPrecision = std::cout.precision();
            std::cout << " Val loss: " << std::setw(10) << std::setprecision(6) << loss << std::right << std::flush;
            std::cout.precision(currPrecision);

            // Überprüfe, ob der Fehler besser ist
            if(loss < bestLoss) {
                bestLoss = loss;
                patience = 0;
                bestParams = currentParams;
            } else {
                patience++;
            }

            // Überprüfe, ob die Geduld erschöpft ist
            if(patience >= noImprovementPatience.get<size_t>()) {
                std::cout << std::endl << "Early stopping at epoch " << trainingSession.epoch << std::endl;
                break;
            }

            // Bestimme zufällige Indizes
            std::uniform_int_distribution<size_t> dist(0, data.size() - 1);
            for(size_t& i : indices)
                i = dist(gen);

            // Berechne den Gradienten
            grad = Tune::gradient(data, indices, currentParams, k.get<double>(), discount.get<double>());

            // Berechne die ersten und zweiten Momente
            for(size_t i = 0; i < currentParams.size(); i++) {
                trainingSession.m[i] = beta1.get<double>() * trainingSession.m[i] + (1.0 - beta1.get<double>()) * grad[i];
                trainingSession.v[i] = beta2.get<double>() * trainingSession.v[i] + (1.0 - beta2.get<double>()) * grad[i] * grad[i];
            }

            // Aktualisiere die Parameter
            for(size_t i = 0; i < currentParams.size(); i++) {
                // Überspringe nicht optimierbare Parameter
                if(!currentParams.isOptimizable(i))
                    continue;

                // Berechne den Bias-korrigierten ersten und zweiten Moment
                double mHat = trainingSession.m[i] / (1 - std::pow(beta1.get<double>(), trainingSession.epoch + 1));
                double vHat = trainingSession.v[i] / (1 - std::pow(beta2.get<double>(), trainingSession.epoch + 1));

                // Berechne die Lernrate
                double lr = learningRate / (std::sqrt(vHat) + epsilon.get<double>());

                // Aktualisiere den Parameter
                trainingSession.exactParams[i] -= lr * mHat;

                // Aktualisiere den Parameter im Parametersatz
                currentParams[i] = std::round(trainingSession.exactParams[i]);
            }

            // Die Positionstabellen müssen neu entpackt werden
            currentParams.unpackPSQT();
        }

        // Berechne den finalen Fehler
        double loss = Tune::loss(validationData, currentParams, k.get<double>(), discount.get<double>(), weightDecay.get<double>());
        if(loss < bestLoss) {
            bestParams = currentParams;
            bestLoss = loss;
        }

        trainingSession.generation++;
        trainingSession.averageLoss = trainingSession.averageLoss * alpha.get<double>() + (1.0 - alpha.get<double>()) * loss;
        // Bias-korrigierter Loss
        double biasCorrectedLoss = trainingSession.averageLoss / (1 - std::pow(alpha.get<double>(), trainingSession.generation));

        std::cout << "\rEpoch: " << std::left << std::setw(4) << trainingSession.epoch;
        size_t currPrecision = std::cout.precision();
        std::cout << " Val loss: " << std::setw(10) << std::setprecision(6) << loss << std::right << std::endl;
        std::cout << "Average loss: " << std::setw(10) << std::setprecision(6) << biasCorrectedLoss << std::right << std::endl;
        std::cout.precision(currPrecision);

        return bestParams;
    }
};