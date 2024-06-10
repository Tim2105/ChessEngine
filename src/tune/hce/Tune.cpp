#if defined(USE_HCE) && defined(TUNE)

#include "core/engine/search/PVSSearchInstance.h"
#include "tune/hce/Definitions.h"
#include "tune/hce/Tune.h"

#include <atomic>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>

namespace Tune {
    /**
     * @brief Sigmoid-Funktion mit einem zusätzlichen Parameter.
     * 
     * @param x Das Argument der Sigmoid-Funktion.
     * @param k Ein Faktor, der mit dem Argument multipliziert wird.
     */
    constexpr double sigmoid(double x, double k) {
        return 1 / (1 + std::exp(-k * x));
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
            double prediction = sigmoid(searchInstance.pvs(0, 0, MIN_SCORE, MAX_SCORE, 0, PV_NODE), k);

            // Berechne den Fehler
            sum += mse(prediction, dp.result);
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
                    double prediction = sigmoid(searchInstance.pvs(0, 0, MIN_SCORE, MAX_SCORE, 0, PV_NODE), k);

                    // Berechne den Fehler
                    double l = mse(prediction, dp.result);

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

                double l = loss(data, indices, hceParamsCopy, k);
                grad[i] = (l - currLoss) / std::pow(k, 1.5);

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
};

#endif