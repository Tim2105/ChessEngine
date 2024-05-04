#ifndef TUNE_H
#define TUNE_H

#include "core/utils/hce/HCEParameters.h"
#include "tune/hce/Definitions.h"

#include <cmath>
#include <vector>

namespace Tune {
    /**
     * @brief Bestimmt den MSE eines Parametersatzes auf einem Datensatz.
     * Zusätzlich werden die Indizes der Datenpunkte übergeben,
     * die für die Berechnung des Fehlers verwendet werden sollen.
     * 
     * @param data Der Datensatz.
     * @param indices Die Indizes der Datenpunkte, die für die Berechnung des Fehlers verwendet werden sollen.
     * @param hceParams Der Parametersatz.
     * @param k Ein Faktor, der mit dem Argument der Sigmoid-Funktion multipliziert wird.
     * @return double Der mittlere quadratische Fehler.
     */
    double loss(std::vector<DataPoint>& data, const std::vector<size_t>& indices, const HCEParameters& hceParams, double k);

    /**
     * @brief Bestimmt den MSE eines Parametersatzes auf einem Datensatz.
     * Diese Funktion betrachtet den gesamten Datensatz. Die Berechnung
     * wird auf mehrere Threads aufgeteilt.
     * 
     * @param data Der Datensatz.
     * @param hceParams Der Parametersatz.
     * @param k Ein Faktor, der mit dem Argument der Sigmoid-Funktion multipliziert wird.
     * @return double Der mittlere quadratische Fehler.
     */
    double loss(std::vector<DataPoint>& data, const HCEParameters& hceParams, double k);

    /**
     * @brief Berechnet den Gradienten des MSE eines Parametersatzes auf einem Datensatz.
     * Zusätzlich werden die Indizes der Datenpunkte übergeben,
     * die für die Berechnung des Gradienten verwendet werden sollen.
     * 
     * @param data Der Datensatz.
     * @param indices Die Indizes der Datenpunkte, die für die Berechnung des Gradienten verwendet werden sollen.
     * @param hceParams Der Parametersatz.
     * @param k Ein Faktor, der mit dem Argument der Sigmoid-Funktion multipliziert wird.
     * @return std::vector<double> Der Gradient.
     */
    std::vector<double> gradient(std::vector<DataPoint>& data, const std::vector<size_t>& indices, const HCEParameters& hceParams, double k);

    /**
     * @brief Setzt die Cooldown-Liste für die einzelnen Parameter zurück.
     * Die Cooldown-Liste wird verwendet, um Parameter, die keinen/geringen Einfluss
     * auf den Fehler haben, seltener zu betrachten.
     */
    void resetGradientCooldown();
};

#endif