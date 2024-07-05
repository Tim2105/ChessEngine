#if defined(USE_HCE) && defined(TUNE)

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

    double loss(std::vector<DataPoint>& data, const std::vector<size_t>& indices, const HCEParameters& hceParams, double k) {
        double sum = 0;

        // Initialisiere Objekte, die dem Konstruktor der Suchinstanz übergeben werden müssen
        TranspositionTable tt(1);
        std::atomic_bool stop = false;
        std::atomic<std::chrono::system_clock::time_point> startTime = std::chrono::system_clock::now();
        std::atomic<std::chrono::system_clock::time_point> endTime = startTime.load() + std::chrono::seconds(1);
        std::atomic_uint64_t nodes = 0;

        for(size_t i : indices) {
            // Extrahiere den Datenpunkt
            DataPoint& dp = data[i % data.size()];

            // Erstelle eine Suchinstanz
            PVSSearchInstance searchInstance(dp.board, hceParams, tt, stop, startTime, endTime, nodes, nullptr);

            // Berechne die Vorhersage
            double prediction = tanh(searchInstance.pvs(0, 0, MIN_SCORE, MAX_SCORE, PV_NODE), k);

            // Berechne den wahren Wert
            double trueValue;
            if(dp.result == 0)
                trueValue = 0.0;
            else if(dp.result > 0)
                trueValue = std::pow(gamma, dp.result);
            else
                trueValue = -std::pow(gamma, -dp.result);

            // Berechne den Fehler
            sum += mse(prediction, trueValue);
        }

        return sum / indices.size();
    }

    double loss(std::vector<DataPoint>& data, const HCEParameters& hceParams, double k) {
        std::atomic<double> sum = 0;

        size_t currIndex = 0;
        std::mutex mutex;

        auto threadFunc = [&]() {
            // Initialisiere Objekte, die dem Konstruktor der Suchinstanz übergeben werden müssen
            TranspositionTable tt(1);
            std::atomic_bool stop = false;
            std::atomic<std::chrono::system_clock::time_point> startTime = std::chrono::system_clock::now();
            std::atomic<std::chrono::system_clock::time_point> endTime = startTime.load() + std::chrono::seconds(1);
            std::atomic_uint64_t nodes = 0;

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

                    // Erstelle eine Suchinstanz
                    PVSSearchInstance searchInstance(dp.board, hceParams, tt, stop, startTime, endTime, nodes, nullptr);

                    // Berechne die Vorhersage
                    double prediction = tanh(searchInstance.pvs(0, 0, MIN_SCORE, MAX_SCORE, PV_NODE), k);

                    // Berechne den wahren Wert
                    double trueValue;
                    if(dp.result == 0)
                        trueValue = 0.0;
                    else if(dp.result > 0)
                        trueValue = std::pow(gamma, dp.result);
                    else
                        trueValue = -std::pow(gamma, -dp.result);

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

        return sum / data.size();
    }

    std::vector<double> gradient(std::vector<DataPoint>& data, const std::vector<size_t>& indices, const HCEParameters& hceParams, double k) {
        // Rückgabevektor initialisieren
        std::vector<double> grad(hceParams.size(), 0);

        size_t currIndex = 0;
        std::mutex mutex;

        // Berechne den aktuellen Fehler der zu betrachtenden Datenpunkte
        double currLoss = loss(data, indices, hceParams, k);

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

                double l = loss(data, indices, hceParamsCopy, k);
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

    HCEParameters adaGrad(std::vector<DataPoint>& data, const HCEParameters& hceParams) {
        HCEParameters currentParams = hceParams;
        HCEParameters bestParams = hceParams;

        // Teile die Daten in Trainings- und Validierungsdaten auf
        std::random_shuffle(data.begin(), data.end());
        size_t validationSize = data.size() * validationSplit;
        std::vector<DataPoint> validationData(data.end() - validationSize, data.end());
        data.erase(data.end() - validationSize, data.end());
        
        // Initialisiere den Gradienten
        std::vector<double> grad(hceParams.size(), 0);

        // Initialisiere den Gradientenquadratsummenvektor
        std::vector<double> gradSqSum(hceParams.size(), 0);

        // Konvertiere die Parameter in einen double-Vektor
        std::vector<double> parameters(hceParams.size());
        for(size_t i = 0; i < hceParams.size(); i++)
            parameters[i] = hceParams[i];

        // Initialisiere den Indexvektor
        std::vector<size_t> indices(batchSize);

        // Bestimme einen Zufallszahlengenerator
        std::random_device rd;
        std::mt19937 gen(rd());

        // Initialisiere den Zähler für die Geduld
        size_t patience = 0;

        // Initialisiere den besten Fehler
        double bestLoss = std::numeric_limits<double>::infinity();

        std::cout << "Starting Gradient Descent..." << std::endl;

        // Iteriere über die Epochen
        for(size_t epoch = 0; epoch < numEpochs; epoch++) {
            // Berechne den Fehler
            double loss = Tune::loss(validationData, currentParams, k);

            if(epoch == 0)
                std::cout << "Initial val loss: " << loss << std::flush;

            if(epoch % 10 == 1)
                std::cout << std::endl << "Epoch: " << std::left << std::setw(4) << epoch;
            else if(epoch != 0)
                std::cout << "\rEpoch: " << std::left << std::setw(4) << epoch;

            if(epoch != 0)
                std::cout << " Val loss: " << std::setw(10) << std::fixed << std::setprecision(6) << loss << std::right << std::flush;

            // Überprüfe, ob der Fehler besser ist
            if(loss < bestLoss) {
                bestLoss = loss;
                patience = 0;
                bestParams = currentParams;
            } else {
                patience++;
            }

            // Überprüfe, ob die Geduld erschöpft ist
            if(patience >= noImprovementPatience) {
                std::cout << std::endl << "Early stopping at epoch " << epoch << std::endl;
                break;
            }

            // Bestimme zufällige Indizes
            std::uniform_int_distribution<size_t> dist(0, data.size() - 1);
            for(size_t& i : indices)
                i = dist(gen);

            // Berechne den Gradienten
            grad = Tune::gradient(data, indices, currentParams, k);

            // Berechne die Gradientenquadratsumme
            for(size_t i = 0; i < currentParams.size(); i++)
                gradSqSum[i] += grad[i] * grad[i];

            // Aktualisiere die Parameter
            for(size_t i = 0; i < currentParams.size(); i++) {
                // Überspringe nicht optimierbare Parameter
                if(!currentParams.isOptimizable(i))
                    continue;

                // Berechne die Lernrate
                double lr = learningRate / (std::sqrt(gradSqSum[i]) + epsilon);

                // Aktualisiere den Parameter
                parameters[i] -= lr * grad[i];

                // Aktualisiere den Parameter im Parametersatz
                currentParams[i] = std::round(parameters[i]);
            }

            // Die Positionstabellen müssen neu entpackt werden
            currentParams.unpackPSQT();
        }

        double loss = Tune::loss(validationData, bestParams, k);
        std::cout << std::endl << "Final val loss: " << loss << std::endl;

        return bestParams;
    }
};

#endif