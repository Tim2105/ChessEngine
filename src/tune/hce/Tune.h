#ifndef TUNE_H
#define TUNE_H

#include "core/utils/hce/HCEParameters.h"

#include <cmath>
#include <vector>

namespace Tune {
    struct TrainingSession {
        // Initialisiere den ersten Moment
        std::vector<double> m = std::vector<double>(HCEParameters::size(), 0);

        // Initialisiere den zweiten Moment
        std::vector<double> v = std::vector<double>(HCEParameters::size(), 0);

        // Konvertiere die Parameter in einen double-Vektor
        std::vector<double> exactParams = std::vector<double>(HCEParameters::size(), 0);

        size_t generation = 0;
        size_t epoch = 0;
        double averageLoss = 0.0;

        inline TrainingSession(const HCEParameters& params) {
            for(size_t i = 0; i < params.size(); i++)
                exactParams[i] = params[i];
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const TrainingSession& session) {
        os.write(reinterpret_cast<const char*>(&session.generation), sizeof(session.generation));
        os.write(reinterpret_cast<const char*>(&session.epoch), sizeof(session.epoch));
        os.write(reinterpret_cast<const char*>(&session.averageLoss), sizeof(session.averageLoss));
        os.write(reinterpret_cast<const char*>(session.m.data()), session.m.size() * sizeof(double));
        os.write(reinterpret_cast<const char*>(session.v.data()), session.v.size() * sizeof(double));
        os.write(reinterpret_cast<const char*>(session.exactParams.data()), session.exactParams.size() * sizeof(double));

        return os;
    }

    inline std::istream& operator>>(std::istream& is, TrainingSession& session) {
        is.read(reinterpret_cast<char*>(&session.generation), sizeof(session.generation));
        is.read(reinterpret_cast<char*>(&session.epoch), sizeof(session.epoch));
        is.read(reinterpret_cast<char*>(&session.averageLoss), sizeof(session.averageLoss));
        is.read(reinterpret_cast<char*>(session.m.data()), session.m.size() * sizeof(double));
        is.read(reinterpret_cast<char*>(session.v.data()), session.v.size() * sizeof(double));
        is.read(reinterpret_cast<char*>(session.exactParams.data()), session.exactParams.size() * sizeof(double));

        return is;
    }

    extern TrainingSession trainingSession;

    /**
     * @brief Bestimmt den MSE eines Parametersatzes auf einem Datensatz.
     * Zusätzlich werden die Indizes der Datenpunkte übergeben,
     * die für die Berechnung des Fehlers verwendet werden sollen.
     * 
     * @param data Der Datensatz.
     * @param indices Die Indizes der Datenpunkte, die für die Berechnung des Fehlers verwendet werden sollen.
     * @param hceParams Der Parametersatz.
     * @param k Ein Faktor, der mit dem Argument der Sigmoid-Funktion multipliziert wird.
     * @param discount Der Discount-Faktor.
     * @param weightDecay Der Gewichtungsabfall.
     * @return double Der mittlere quadratische Fehler.
     */
    double loss(std::vector<DataPoint>& data, const std::vector<size_t>& indices, const HCEParameters& hceParams, double k, double discount, double weightDecay = 0.0);

    /**
     * @brief Bestimmt den MSE eines Parametersatzes auf einem Datensatz.
     * Diese Funktion betrachtet den gesamten Datensatz. Die Berechnung
     * wird auf mehrere Threads aufgeteilt.
     * 
     * @param data Der Datensatz.
     * @param hceParams Der Parametersatz.
     * @param k Ein Faktor, der mit dem Argument der Sigmoid-Funktion multipliziert wird.
     * @param discount Der Discount-Faktor.
     * @param weightDecay Der Gewichtungsabfall.
     * @return double Der mittlere quadratische Fehler.
     */
    double loss(std::vector<DataPoint>& data, const HCEParameters& hceParams, double k, double discount, double weightDecay = 0.0);

    /**
     * @brief Berechnet den Gradienten des MSE eines Parametersatzes auf einem Datensatz.
     * Zusätzlich werden die Indizes der Datenpunkte übergeben,
     * die für die Berechnung des Gradienten verwendet werden sollen.
     * 
     * @param data Der Datensatz.
     * @param indices Die Indizes der Datenpunkte, die für die Berechnung des Gradienten verwendet werden sollen.
     * @param hceParams Der Parametersatz.
     * @param k Ein Faktor, der mit dem Argument der Sigmoid-Funktion multipliziert wird.
     * @param discount Der Discount-Faktor.
     * @return std::vector<double> Der Gradient.
     */
    std::vector<double> gradient(std::vector<DataPoint>& data, const std::vector<size_t>& indices, const HCEParameters& hceParams, double k, double discount);

    /**
     * @brief Verbessert die Parameter eines HCE-Modells über den AdamW-Algorithmus.
     * 
     * @param data Der Datensatz.
     * @param hceParams Die Parameter des HCE-Modells.
     * @param numEpochs Die Anzahl der Epochen.
     * @param learningRate Die Lernrate.
     * @return HCEParameters Die verbesserten Parameter.
     */
    HCEParameters adamW(std::vector<DataPoint>& data, const HCEParameters& hceParams, size_t numEpochs, double learningRate);
};

#endif