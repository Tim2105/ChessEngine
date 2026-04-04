#ifndef TRAIN_H
#define TRAIN_H

#include "core/utils/nnue/NNUENetwork.h"
#include "tune/Definitions.h"

#include <cmath>
#include <vector>

namespace Train {
    struct MasterWeights {
        // Master-Parameter des HalfKP-Layers
        std::vector<float> exactHalfKPBiases = std::vector<float>(NNUE::Network::SINGLE_SUBNET_SIZE, 0);
        std::vector<float> exactHalfKP = std::vector<float>(NNUE::Network::INPUT_SIZE * NNUE::Network::SINGLE_SUBNET_SIZE, 0);

        // Master-Parameter der Dense-Layer
        std::array<std::vector<float>, NNUE::Network::NUM_LAYERS> exactDenseLayerBiases;
        std::array<std::vector<float>, NNUE::Network::NUM_LAYERS> exactDenseLayerWeights;

        inline MasterWeights() {
            for(size_t i = 0; i < NNUE::Network::NUM_LAYERS; i++) {
                exactDenseLayerBiases[i] = std::vector<float>(NNUE::Network::LAYER_SIZES[i + 1], 0);
                exactDenseLayerWeights[i] = std::vector<float>(NNUE::Network::LAYER_SIZES[i] * NNUE::Network::LAYER_SIZES[i + 1], 0);
            }
        }

        inline MasterWeights(const NNUE::Network& network) : MasterWeights() {
            const auto& halfKP = network.getHalfKPLayer();

            // Biases, dequantisiert
            for(size_t i = 0; i < NNUE::Network::SINGLE_SUBNET_SIZE; i++)
                exactHalfKPBiases[i] = halfKP.getBias(i) / 64.0f;

            // Gewichte, dequantisiert
            for(size_t i = 0; i < NNUE::Network::INPUT_SIZE; i++) {
                for(size_t j = 0; j < NNUE::Network::SINGLE_SUBNET_SIZE; j++)
                    exactHalfKP[i * NNUE::Network::SINGLE_SUBNET_SIZE + j] = halfKP.getWeight(i, j) / 64.0f;
            }

            const auto& layer1 = network.getLayer1();
            const auto& layer2 = network.getLayer2();
            const auto& layer3 = network.getLayer3();

            for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[1]; i++) {
                exactDenseLayerBiases[0][i] = layer1.getBias(i) / 64.0f;

                for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[0]; j++)
                    exactDenseLayerWeights[0][j * NNUE::Network::LAYER_SIZES[1] + i] = layer1.getWeight(j, i) / 64.0f;
            }

            for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[2]; i++) {
                exactDenseLayerBiases[1][i] = layer2.getBias(i) / 64.0f;

                for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[1]; j++)
                    exactDenseLayerWeights[1][j * NNUE::Network::LAYER_SIZES[2] + i] = layer2.getWeight(j, i) / 64.0f;
            }

            for(size_t i = 0; i < NNUE::Network::LAYER_SIZES[3]; i++) {
                exactDenseLayerBiases[2][i] = layer3.getBias(i) / 64.0f;

                for(size_t j = 0; j < NNUE::Network::LAYER_SIZES[2]; j++)
                    exactDenseLayerWeights[2][j * NNUE::Network::LAYER_SIZES[3] + i] = layer3.getWeight(j, i) / 64.0f;
            }
        }

        /**
         * @brief Konvertiert die Master-Parameter zurück in ein NNUE-Netzwerk. Dabei werden die Parameter quantisiert.
         * Das zurückgegebene Objekt muss vom Aufrufer freigegeben werden.
         */
        NNUE::Network* toNetwork() const;
    };

    struct Gradients {
        std::vector<float> halfKPBiases = std::vector<float>(NNUE::Network::SINGLE_SUBNET_SIZE, 0);
        std::vector<float> halfKPWeights = std::vector<float>(NNUE::Network::INPUT_SIZE * NNUE::Network::SINGLE_SUBNET_SIZE, 0);
        std::array<std::vector<float>, NNUE::Network::NUM_LAYERS> denseLayerBiases;
        std::array<std::vector<float>, NNUE::Network::NUM_LAYERS> denseLayerWeights;

        inline Gradients() {
            for(size_t i = 0; i < NNUE::Network::NUM_LAYERS; i++) {
                denseLayerBiases[i] = std::vector<float>(NNUE::Network::LAYER_SIZES[i + 1], 0);
                denseLayerWeights[i] = std::vector<float>(NNUE::Network::LAYER_SIZES[i] * NNUE::Network::LAYER_SIZES[i + 1], 0);
            }
        }
    };

    struct TrainingSession {
        // Erster Moment des HalfKP-Layers
        std::vector<float> mHalfKPBiases = std::vector<float>(NNUE::Network::SINGLE_SUBNET_SIZE, 0);
        std::vector<float> mHalfKPWeights = std::vector<float>(NNUE::Network::INPUT_SIZE * NNUE::Network::SINGLE_SUBNET_SIZE, 0);

        // Erster Moment der Dense-Layer
        std::array<std::vector<float>, NNUE::Network::NUM_LAYERS> mDenseLayerBiases;
        std::array<std::vector<float>, NNUE::Network::NUM_LAYERS> mDenseLayerWeights;

        // Zweiter Moment des HalfKP-Layers
        std::vector<float> vHalfKPBiases = std::vector<float>(NNUE::Network::SINGLE_SUBNET_SIZE, 0);
        std::vector<float> vHalfKP = std::vector<float>(NNUE::Network::INPUT_SIZE * NNUE::Network::SINGLE_SUBNET_SIZE, 0);

        // Zweiter Moment der Dense-Layer
        std::array<std::vector<float>, NNUE::Network::NUM_LAYERS> vDenseLayerBiases;
        std::array<std::vector<float>, NNUE::Network::NUM_LAYERS> vDenseLayerWeights;

        MasterWeights masterWeights;

        size_t generation = 0;
        size_t epoch = 0;
        double averageLoss = 0.0;

        inline TrainingSession(const NNUE::Network& network) : masterWeights(network) {
            for(size_t i = 0; i < NNUE::Network::NUM_LAYERS; i++) {
                mDenseLayerBiases[i] = std::vector<float>(NNUE::Network::LAYER_SIZES[i + 1], 0);
                mDenseLayerWeights[i] = std::vector<float>(NNUE::Network::LAYER_SIZES[i] * NNUE::Network::LAYER_SIZES[i + 1], 0);
                vDenseLayerBiases[i] = std::vector<float>(NNUE::Network::LAYER_SIZES[i + 1], 0);
                vDenseLayerWeights[i] = std::vector<float>(NNUE::Network::LAYER_SIZES[i] * NNUE::Network::LAYER_SIZES[i + 1], 0);
            }
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
        os.write(reinterpret_cast<const char*>(session.vHalfKP.data()), session.vHalfKP.size() * sizeof(float));
        for(size_t i = 0; i < NNUE::Network::NUM_LAYERS; i++) {
            os.write(reinterpret_cast<const char*>(session.vDenseLayerBiases[i].data()), session.vDenseLayerBiases[i].size() * sizeof(float));
            os.write(reinterpret_cast<const char*>(session.vDenseLayerWeights[i].data()), session.vDenseLayerWeights[i].size() * sizeof(float));
        }

        os.write(reinterpret_cast<const char*>(session.masterWeights.exactHalfKPBiases.data()), session.masterWeights.exactHalfKPBiases.size() * sizeof(float));
        os.write(reinterpret_cast<const char*>(session.masterWeights.exactHalfKP.data()), session.masterWeights.exactHalfKP.size() * sizeof(float));
        for(size_t i = 0; i < NNUE::Network::NUM_LAYERS; i++) {
            os.write(reinterpret_cast<const char*>(session.masterWeights.exactDenseLayerBiases[i].data()), session.masterWeights.exactDenseLayerBiases[i].size() * sizeof(float));
            os.write(reinterpret_cast<const char*>(session.masterWeights.exactDenseLayerWeights[i].data()), session.masterWeights.exactDenseLayerWeights[i].size() * sizeof(float));
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
        is.read(reinterpret_cast<char*>(session.vHalfKP.data()), session.vHalfKP.size() * sizeof(float));
        for(size_t i = 0; i < NNUE::Network::NUM_LAYERS; i++) {
            is.read(reinterpret_cast<char*>(session.vDenseLayerBiases[i].data()), session.vDenseLayerBiases[i].size() * sizeof(float));
            is.read(reinterpret_cast<char*>(session.vDenseLayerWeights[i].data()), session.vDenseLayerWeights[i].size() * sizeof(float));
        }

        is.read(reinterpret_cast<char*>(session.masterWeights.exactHalfKPBiases.data()), session.masterWeights.exactHalfKPBiases.size() * sizeof(float));
        is.read(reinterpret_cast<char*>(session.masterWeights.exactHalfKP.data()), session.masterWeights.exactHalfKP.size() * sizeof(float));
        for(size_t i = 0; i < NNUE::Network::NUM_LAYERS; i++) {
            is.read(reinterpret_cast<char*>(session.masterWeights.exactDenseLayerBiases[i].data()), session.masterWeights.exactDenseLayerBiases[i].size() * sizeof(float));
            is.read(reinterpret_cast<char*>(session.masterWeights.exactDenseLayerWeights[i].data()), session.masterWeights.exactDenseLayerWeights[i].size() * sizeof(float));
        }

        return is;
    }

    extern TrainingSession trainingSession;

    /**
     * @brief Bestimmt den MSE eines Parametersatzes auf einem Datensatz.
     * Diese Funktion betrachtet den gesamten Datensatz. Die Berechnung
     * wird auf mehrere Threads aufgeteilt.
     * 
     * @param data Der Datensatz.
     * @param network Das Netzwerk.
     * @param k Ein Faktor, der mit dem Argument der Sigmoid-Funktion multipliziert wird.
     * @param weightDecay Der Gewichtungsabfall.
     * @return double Der mittlere quadratische Fehler.
     */
    double loss(std::vector<DataPoint>& data, const NNUE::Network& network, double k, double weightDecay = 0.0);

    /**
     * @brief Berechnet den Gradienten des MSE eines Parametersatzes auf einem Datensatz.
     * Zusätzlich werden die Indizes der Datenpunkte übergeben,
     * die für die Berechnung des Gradienten verwendet werden sollen.
     * 
     * @param data Der Datensatz.
     * @param indices Die Indizes der Datenpunkte, die für die Berechnung des Gradienten verwendet werden sollen.
     * @param network Das Netzwerk.
     * @param k Ein Faktor, der mit dem Argument der Sigmoid-Funktion multipliziert wird.
     * @param weightDecay Der Gewichtungsabfall.
     * @return std::vector<float> Der Gradient.
     */
    Gradients gradient(std::vector<DataPoint>& data, const std::vector<size_t>& indices, const NNUE::Network& network, double k, double weightDecay = 0.0);

    /**
     * @brief Verbessert die Parameter eines HCE-Modells über den AdamW-Algorithmus.
     * 
     * @param data Der Datensatz.
     * @param network Das Netzwerk.
     * @param numEpochs Die Anzahl der Epochen.
     * @param learningRate Die Lernrate.
     * @return HCEParameters Die verbesserten Parameter.
     */
    NNUE::Network adamW(std::vector<DataPoint>& data, const NNUE::Network& network, size_t numEpochs, double learningRate);
};

#endif