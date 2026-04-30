#ifndef REN_TRAIN_H
#define REN_TRAIN_H

#include "core/utils/nnue/NNUENetwork.h"
#include "tune/Definitions.h"
#include "tune/EloTable.h"
#include "tune/ren/RENMasterWeights.h"

#include <fstream>
#include <vector>

namespace Train {
    struct TrainingSession {
        // Erster Moment des HalfKP-Layers
        std::vector<float> mHalfKPBiases = std::vector<float>(REN::REN_SIZE / 2, 0);
        std::vector<float> mHalfKPWeights = std::vector<float>(NNUE::Network::INPUT_SIZE * (REN::REN_SIZE / 2), 0);

        // Erster Moment der REN- und Output-Layer
        std::vector<float> mRENBiases = std::vector<float>(REN::REN_SIZE, 0);
        std::vector<float> mRENWeights = std::vector<float>(REN::REN_SIZE * REN::REN_SIZE, 0);
        std::vector<float> mOutputBiases = std::vector<float>(1, 0);
        std::vector<float> mOutputWeights = std::vector<float>(REN::REN_SIZE * 1, 0);

        // Zweiter Moment des HalfKP-Layers
        std::vector<float> vHalfKPBiases = std::vector<float>(REN::REN_SIZE / 2, 0);
        std::vector<float> vHalfKPWeights = std::vector<float>(NNUE::Network::INPUT_SIZE * (REN::REN_SIZE / 2), 0);

        // Zweiter Moment der REN- und Output-Layer
        std::vector<float> vRENBiases = std::vector<float>(REN::REN_SIZE, 0);
        std::vector<float> vRENWeights = std::vector<float>(REN::REN_SIZE * REN::REN_SIZE, 0);
        std::vector<float> vOutputBiases = std::vector<float>(1, 0);
        std::vector<float> vOutputWeights = std::vector<float>(REN::REN_SIZE * 1, 0);

        // Letzte Aktualisierung der Parameter (für sparse Adam)
        std::vector<size_t> lastUpdateHalfKPWeights = std::vector<size_t>(NNUE::Network::INPUT_SIZE * (REN::REN_SIZE / 2), 0);

        REN::MasterWeights masterWeights;
        EloTable<NNUE::Network> eloTable;

        size_t generation = 0;
        size_t epoch = 0;
        double averageLoss = 0.0;

        inline TrainingSession() = default;
    };

    inline std::ostream& operator<<(std::ostream& os, const TrainingSession& session) {
        os.write(reinterpret_cast<const char*>(&session.generation), sizeof(session.generation));
        os.write(reinterpret_cast<const char*>(&session.epoch), sizeof(session.epoch));
        os.write(reinterpret_cast<const char*>(&session.averageLoss), sizeof(session.averageLoss));
        os.write(reinterpret_cast<const char*>(session.mHalfKPBiases.data()), session.mHalfKPBiases.size() * sizeof(float));
        os.write(reinterpret_cast<const char*>(session.mHalfKPWeights.data()), session.mHalfKPWeights.size() * sizeof(float));
        os.write(reinterpret_cast<const char*>(session.mRENBiases.data()), session.mRENBiases.size() * sizeof(float));
        os.write(reinterpret_cast<const char*>(session.mRENWeights.data()), session.mRENWeights.size() * sizeof(float));
        os.write(reinterpret_cast<const char*>(session.mOutputBiases.data()), session.mOutputBiases.size() * sizeof(float));
        os.write(reinterpret_cast<const char*>(session.mOutputWeights.data()), session.mOutputWeights.size() * sizeof(float));

        os.write(reinterpret_cast<const char*>(session.vHalfKPBiases.data()), session.vHalfKPBiases.size() * sizeof(float));
        os.write(reinterpret_cast<const char*>(session.vHalfKPWeights.data()), session.vHalfKPWeights.size() * sizeof(float));
        os.write(reinterpret_cast<const char*>(session.vRENBiases.data()), session.vRENBiases.size() * sizeof(float));
        os.write(reinterpret_cast<const char*>(session.vRENWeights.data()), session.vRENWeights.size() * sizeof(float));
        os.write(reinterpret_cast<const char*>(session.vOutputBiases.data()), session.vOutputBiases.size() * sizeof(float));
        os.write(reinterpret_cast<const char*>(session.vOutputWeights.data()), session.vOutputWeights.size() * sizeof(float));

        os.write(reinterpret_cast<const char*>(session.lastUpdateHalfKPWeights.data()), session.lastUpdateHalfKPWeights.size() * sizeof(size_t));

        os.write(reinterpret_cast<const char*>(session.masterWeights.halfKAv2Layer.bias.data()), session.masterWeights.halfKAv2Layer.bias.size * sizeof(float));
        os.write(reinterpret_cast<const char*>(session.masterWeights.halfKAv2Layer.weights.data()), session.masterWeights.halfKAv2Layer.weights.size * sizeof(float));

        os.write(reinterpret_cast<const char*>(session.masterWeights.renLayer.surrogateWeights.q.data()), session.masterWeights.renLayer.surrogateWeights.q.size * sizeof(float));
        os.write(reinterpret_cast<const char*>(session.masterWeights.renLayer.surrogateWeights.gammaRaw.data()), session.masterWeights.renLayer.surrogateWeights.gammaRaw.size * sizeof(float));
        os.write(reinterpret_cast<const char*>(session.masterWeights.renLayer.bias.data()), session.masterWeights.renLayer.bias.size * sizeof(float));

        os.write(reinterpret_cast<const char*>(session.masterWeights.outputLayer.bias.data()), session.masterWeights.outputLayer.bias.size * sizeof(float));
        os.write(reinterpret_cast<const char*>(session.masterWeights.outputLayer.weights.data()), session.masterWeights.outputLayer.weights.size * sizeof(float));

        size_t eloTableSize = session.eloTable.size();
        os.write(reinterpret_cast<const char*>(&eloTableSize), sizeof(eloTableSize));

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
    };

    inline std::istream& operator>>(std::istream& is, TrainingSession& session) {
        is.read(reinterpret_cast<char*>(&session.generation), sizeof(session.generation));
        is.read(reinterpret_cast<char*>(&session.epoch), sizeof(session.epoch));
        is.read(reinterpret_cast<char*>(&session.averageLoss), sizeof(session.averageLoss));
        is.read(reinterpret_cast<char*>(session.mHalfKPBiases.data()), session.mHalfKPBiases.size() * sizeof(float));
        is.read(reinterpret_cast<char*>(session.mHalfKPWeights.data()), session.mHalfKPWeights.size() * sizeof(float));
        is.read(reinterpret_cast<char*>(session.mRENBiases.data()), session.mRENBiases.size() * sizeof(float));
        is.read(reinterpret_cast<char*>(session.mRENWeights.data()), session.mRENWeights.size() * sizeof(float));
        is.read(reinterpret_cast<char*>(session.mOutputBiases.data()), session.mOutputBiases.size() * sizeof(float));
        is.read(reinterpret_cast<char*>(session.mOutputWeights.data()), session.mOutputWeights.size() * sizeof(float));

        is.read(reinterpret_cast<char*>(session.vHalfKPBiases.data()), session.vHalfKPBiases.size() * sizeof(float));
        is.read(reinterpret_cast<char*>(session.vHalfKPWeights.data()), session.vHalfKPWeights.size() * sizeof(float));
        is.read(reinterpret_cast<char*>(session.vRENBiases.data()), session.vRENBiases.size() * sizeof(float));
        is.read(reinterpret_cast<char*>(session.vRENWeights.data()), session.vRENWeights.size() * sizeof(float));
        is.read(reinterpret_cast<char*>(session.vOutputBiases.data()), session.vOutputBiases.size() * sizeof(float));
        is.read(reinterpret_cast<char*>(session.vOutputWeights.data()), session.vOutputWeights.size() * sizeof(float));

        is.read(reinterpret_cast<char*>(session.lastUpdateHalfKPWeights.data()), session.lastUpdateHalfKPWeights.size() * sizeof(size_t));

        is.read(reinterpret_cast<char*>(session.masterWeights.halfKAv2Layer.bias.data()), session.masterWeights.halfKAv2Layer.bias.size * sizeof(float));
        is.read(reinterpret_cast<char*>(session.masterWeights.halfKAv2Layer.weights.data()), session.masterWeights.halfKAv2Layer.weights.size * sizeof(float));

        is.read(reinterpret_cast<char*>(session.masterWeights.renLayer.surrogateWeights.q.data()), session.masterWeights.renLayer.surrogateWeights.q.size * sizeof(float));
        is.read(reinterpret_cast<char*>(session.masterWeights.renLayer.surrogateWeights.gammaRaw.data()), session.masterWeights.renLayer.surrogateWeights.gammaRaw.size * sizeof(float));
        is.read(reinterpret_cast<char*>(session.masterWeights.renLayer.bias.data()), session.masterWeights.renLayer.bias.size * sizeof(float));

        is.read(reinterpret_cast<char*>(session.masterWeights.outputLayer.bias.data()), session.masterWeights.outputLayer.bias.size * sizeof(float));
        is.read(reinterpret_cast<char*>(session.masterWeights.outputLayer.weights.data()), session.masterWeights.outputLayer.weights.size * sizeof(float));

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
            
            // Lade das Netzwerk aus der entsprechenden .ren-Datei
            std::string networkFilePath = "data/" + name + ".ren";
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
    };

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
    double loss(std::vector<DataPoint>& data, const REN::MasterWeights& masterWeights, double k, double kappa);

    /**
     * @brief Initialisiert die Master-Parameter des REN.
     * Die HalfKAv2_hm- und Dense-Layer werden mit der Kaiming-Initialisierung initialisiert.
     * Die REN-Schicht wird so initalisiert, dass sie zu Beginn annähernd eine
     * Identitätsfunktion darstellt.
     */
    void initializeWeights(REN::MasterWeights& masterWeights);
};

#endif