#ifndef NNUE_TRAIN_H
#define NNUE_TRAIN_H

#include "core/utils/nnue/NNUENetwork.h"
#include "tune/Definitions.h"
#include "tune/EloTable.h"
#include "tune/nnue/NNUEMasterWeights.h"

#include <cmath>
#include <fstream>
#include <vector>

namespace Train {
    struct TrainingSession {
        // Erster Moment des HalfKP-Layers
        std::vector<float> mHalfKPBiases = std::vector<float>(NNUE::Network::SINGLE_SUBNET_SIZE, 0);
        std::vector<float> mHalfKPWeights = std::vector<float>(NNUE::Network::INPUT_SIZE * NNUE::Network::SINGLE_SUBNET_SIZE, 0);

        // Erster Moment der Dense-Layer
        std::array<std::vector<float>, NNUE::Network::NUM_LAYERS> mDenseLayerBiases;
        std::array<std::vector<float>, NNUE::Network::NUM_LAYERS> mDenseLayerWeights;

        // Zweiter Moment des HalfKP-Layers
        std::vector<float> vHalfKPBiases = std::vector<float>(NNUE::Network::SINGLE_SUBNET_SIZE, 0);
        std::vector<float> vHalfKPWeights = std::vector<float>(NNUE::Network::INPUT_SIZE * NNUE::Network::SINGLE_SUBNET_SIZE, 0);

        // Zweiter Moment der Dense-Layer
        std::array<std::vector<float>, NNUE::Network::NUM_LAYERS> vDenseLayerBiases;
        std::array<std::vector<float>, NNUE::Network::NUM_LAYERS> vDenseLayerWeights;

        // Letzte Aktualisierung der Parameter (für sparse Adam)
        std::vector<size_t> lastUpdateHalfKPWeights = std::vector<size_t>(NNUE::Network::INPUT_SIZE * NNUE::Network::SINGLE_SUBNET_SIZE, 0);

        NNUE::MasterWeights masterWeights;
        EloTable<NNUE::Network> eloTable;

        size_t generation = 0;
        size_t epoch = 0;
        double averageLoss = 0.0;

        inline TrainingSession() {
            for(size_t i = 0; i < NNUE::Network::NUM_LAYERS; i++) {
                mDenseLayerBiases[i] = std::vector<float>(NNUE::Network::LAYER_SIZES[i + 1], 0);
                mDenseLayerWeights[i] = std::vector<float>(NNUE::Network::LAYER_SIZES[i] * NNUE::Network::LAYER_SIZES[i + 1], 0);
                vDenseLayerBiases[i] = std::vector<float>(NNUE::Network::LAYER_SIZES[i + 1], 0);
                vDenseLayerWeights[i] = std::vector<float>(NNUE::Network::LAYER_SIZES[i] * NNUE::Network::LAYER_SIZES[i + 1], 0);
            }
        }

        inline TrainingSession(const NNUE::Network& network) : TrainingSession() {
            masterWeights = NNUE::MasterWeights(network);
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const TrainingSession& session) {
        os.write(reinterpret_cast<const char*>(&session.generation), sizeof(session.generation));
        os.write(reinterpret_cast<const char*>(&session.epoch), sizeof(session.epoch));
        os.write(reinterpret_cast<const char*>(&session.averageLoss), sizeof(session.averageLoss));
        os.write(reinterpret_cast<const char*>(session.mHalfKPBiases.data()), session.mHalfKPBiases.size() * sizeof(float));
        os.write(reinterpret_cast<const char*>(session.mHalfKPWeights.data()), session.mHalfKPWeights.size() * sizeof(float));
        for(size_t i = 0; i < NNUE::Network::NUM_LAYERS; i++) {
            os.write(reinterpret_cast<const char*>(session.mDenseLayerBiases[i].data()), session.mDenseLayerBiases[i].size() * sizeof(float));
            os.write(reinterpret_cast<const char*>(session.mDenseLayerWeights[i].data()), session.mDenseLayerWeights[i].size() * sizeof(float));
        }

        os.write(reinterpret_cast<const char*>(session.vHalfKPBiases.data()), session.vHalfKPBiases.size() * sizeof(float));
        os.write(reinterpret_cast<const char*>(session.vHalfKPWeights.data()), session.vHalfKPWeights.size() * sizeof(float));
        for(size_t i = 0; i < NNUE::Network::NUM_LAYERS; i++) {
            os.write(reinterpret_cast<const char*>(session.vDenseLayerBiases[i].data()), session.vDenseLayerBiases[i].size() * sizeof(float));
            os.write(reinterpret_cast<const char*>(session.vDenseLayerWeights[i].data()), session.vDenseLayerWeights[i].size() * sizeof(float));
        }

        os.write(reinterpret_cast<const char*>(session.lastUpdateHalfKPWeights.data()), session.lastUpdateHalfKPWeights.size() * sizeof(size_t));

        os.write(reinterpret_cast<const char*>(session.masterWeights.halfKPLayer.bias.data()), session.masterWeights.halfKPLayer.bias.size * sizeof(float));
        os.write(reinterpret_cast<const char*>(session.masterWeights.halfKPLayer.weights.data()), session.masterWeights.halfKPLayer.weights.size * sizeof(float));
        for(size_t i = 0; i < NNUE::Network::NUM_LAYERS; i++) {
            os.write(reinterpret_cast<const char*>(session.masterWeights.denseLayers[i].bias.data()), session.masterWeights.denseLayers[i].bias.size * sizeof(float));
            os.write(reinterpret_cast<const char*>(session.masterWeights.denseLayers[i].weights.data()), session.masterWeights.denseLayers[i].weights.size * sizeof(float));
        }

        // Speichere die Elo-Tabellen-Einträge (Name und Elo, nicht die Netzwerke)
        size_t numEloEntries = session.eloTable.size();
        os.write(reinterpret_cast<const char*>(&numEloEntries), sizeof(numEloEntries));
        
        for(const auto& [name, data] : session.eloTable) {
            // Speichere den Namen (Length + String)
            size_t nameLen = name.size();
            os.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
            os.write(name.c_str(), nameLen);
            
            // Speichere die Elo-Zahl
            double elo = data.elo;
            os.write(reinterpret_cast<const char*>(&elo), sizeof(elo));
        }

        return os;
    }

    inline std::istream& operator>>(std::istream& is, TrainingSession& session) {
        is.read(reinterpret_cast<char*>(&session.generation), sizeof(session.generation));
        is.read(reinterpret_cast<char*>(&session.epoch), sizeof(session.epoch));
        is.read(reinterpret_cast<char*>(&session.averageLoss), sizeof(session.averageLoss));
        is.read(reinterpret_cast<char*>(session.mHalfKPBiases.data()), session.mHalfKPBiases.size() * sizeof(float));
        is.read(reinterpret_cast<char*>(session.mHalfKPWeights.data()), session.mHalfKPWeights.size() * sizeof(float));
        for(size_t i = 0; i < NNUE::Network::NUM_LAYERS; i++) {
            is.read(reinterpret_cast<char*>(session.mDenseLayerBiases[i].data()), session.mDenseLayerBiases[i].size() * sizeof(float));
            is.read(reinterpret_cast<char*>(session.mDenseLayerWeights[i].data()), session.mDenseLayerWeights[i].size() * sizeof(float));
        }

        is.read(reinterpret_cast<char*>(session.vHalfKPBiases.data()), session.vHalfKPBiases.size() * sizeof(float));
        is.read(reinterpret_cast<char*>(session.vHalfKPWeights.data()), session.vHalfKPWeights.size() * sizeof(float));
        for(size_t i = 0; i < NNUE::Network::NUM_LAYERS; i++) {
            is.read(reinterpret_cast<char*>(session.vDenseLayerBiases[i].data()), session.vDenseLayerBiases[i].size() * sizeof(float));
            is.read(reinterpret_cast<char*>(session.vDenseLayerWeights[i].data()), session.vDenseLayerWeights[i].size() * sizeof(float));
        }

        is.read(reinterpret_cast<char*>(session.lastUpdateHalfKPWeights.data()), session.lastUpdateHalfKPWeights.size() * sizeof(size_t));

        is.read(reinterpret_cast<char*>(session.masterWeights.halfKPLayer.bias.data()), session.masterWeights.halfKPLayer.bias.size * sizeof(float));
        is.read(reinterpret_cast<char*>(session.masterWeights.halfKPLayer.weights.data()), session.masterWeights.halfKPLayer.weights.size * sizeof(float));
        for(size_t i = 0; i < NNUE::Network::NUM_LAYERS; i++) {
            is.read(reinterpret_cast<char*>(session.masterWeights.denseLayers[i].bias.data()), session.masterWeights.denseLayers[i].bias.size * sizeof(float));
            is.read(reinterpret_cast<char*>(session.masterWeights.denseLayers[i].weights.data()), session.masterWeights.denseLayers[i].weights.size * sizeof(float));
        }

        // Lade die Elo-Tabellen-Einträge (Namen und Elos)
        // Die Netzwerke werden aus .nnue-Dateien im data/-Ordner rekonstruiert
        size_t numEloEntries = 0;
        is.read(reinterpret_cast<char*>(&numEloEntries), sizeof(numEloEntries));

        for(size_t i = 0; i < numEloEntries; i++) {
            // Lade den Namen (Length + String)
            size_t nameLen = 0;
            is.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));
            
            std::string name(nameLen, '\0');
            is.read(&name[0], nameLen);
            
            // Lade die Elo-Zahl
            double elo = 0.0;
            is.read(reinterpret_cast<char*>(&elo), sizeof(elo));
            
            // Lade das Netzwerk aus der entsprechenden .nnue-Datei
            std::string networkFilePath = "data/" + name + ".nnue";
            std::ifstream networkFile(networkFilePath, std::ios::binary);
            
            if(networkFile.good()) {
                NNUE::Network* network = new NNUE::Network();
                networkFile >> *network;
                networkFile.close();
                
                // Füge den Spieler mit geladenen Daten zur EloTable hinzu
                session.eloTable.addPlayer(name, elo, *network);

                delete network;
            } else {
                std::cout << "Warning: Could not load network for player '" << name << "' from file '" <<
                    networkFilePath << "'. Skipping this entry." << std::endl;
            }
        }

        return is;
    }

    extern TrainingSession trainingSession;

    /**
     * @brief Bestimmt den MSE eines unquantisierten Parametersatzes auf einem Datensatz.
     * Diese Funktion betrachtet den gesamten Datensatz. Die Berechnung
     * wird auf mehrere Threads aufgeteilt.
     * 
     * @param data Der Datensatz.
     * @param masterWeights Die unquantisierten Parameter des Netzwerks.
     * @param k Der Faktor, der mit dem Bewertungswert innerhalb der tanh-Funktion multipliziert wird.
     * @param kappa Bestimmt, wie stark das finale Ergebnis in das TD-Ziel einfließen soll.
     * @return double Der mittlere quadratische Fehler.
     */
    double loss(std::vector<DataPoint>& data, const NNUE::MasterWeights& masterWeights, double k, double kappa);

    /**
     * @brief Bestimmt den MSE eines quantisierten Parametersatzes auf einem Datensatz.
     * Diese Funktion betrachtet den gesamten Datensatz. Die Berechnung
     * wird auf mehrere Threads aufgeteilt.
     * 
     * @param data Der Datensatz.
     * @param network Das Netzwerk (quantisierte Parameter).
     * @param k Der Faktor, der mit dem Bewertungswert innerhalb der tanh-Funktion multipliziert wird.
     * @param kappa Bestimmt, wie stark das finale Ergebnis in das TD-Ziel einfließen soll.
     * @return double Der mittlere quadratische Fehler.
     */
    double loss(std::vector<DataPoint>& data, const NNUE::Network& network, double k, double kappa);

    /**
     * @brief Berechnet den Gradienten des MSE eines unquantisierten Parametersatzes auf einem Datensatz.
     * Zusätzlich werden die Indizes der Datenpunkte übergeben,
     * die für die Berechnung des Gradienten verwendet werden sollen.
     * 
     * @param data Der Datensatz.
     * @param indices Die Indizes der Datenpunkte, die für die Berechnung des Gradienten verwendet werden sollen.
     * @param masterWeights Die unquantisierten Parameter des Netzwerks.
     * @param k Der Faktor, der mit dem Bewertungswert innerhalb der tanh-Funktion multipliziert wird.
     * @param kappa Bestimmt, wie stark das finale Ergebnis in das TD-Ziel einfließen soll.
     * @return std::vector<float> Der Gradient.
     */
    NNUE::Gradients gradient(std::vector<DataPoint>& data, const std::vector<size_t>& indices, const NNUE::MasterWeights& masterWeights, double k, double kappa);

    /**
     * @brief Verbessert die Parameter eines HCE-Modells über den AdamW-Algorithmus.
     * 
     * @param data Der Datensatz.
     * @param numEpochs Die Anzahl der Epochen.
     * @param learningRate Die Lernrate.
     * @param kappa Bestimmt, wie stark das finale Ergebnis in das Ziel einfließen soll.
     * @return HCEParameters Die verbesserten Parameter.
     */
    NNUE::Network* adamW(std::vector<DataPoint>& data, size_t numEpochs, double learningRate, double kappa);

    /**
     * @brief Initialisiert die Master-Parameter mit der Kaiming-Initialisierung.
     * 
     * @param masterWeights Die Master-Parameter, die initialisiert werden sollen.
     */
    void kaimingInitialization(NNUE::MasterWeights& masterWeights);
};

#endif