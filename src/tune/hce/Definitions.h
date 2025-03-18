#ifndef TRAIN_DEFINITIONS_H
#define TRAIN_DEFINITIONS_H

#include "core/chess/Board.h"
#include "tune/Simulation.h"

#include <string>

struct DataPoint {
    Board board;
    int result;
};

/**
 * Dateipfade.
 */

extern std::string pgnFilePath;
extern std::string samplesFilePath;

/**
 * Variablen der Trainingsdatengenerierung.
 */

extern size_t numGames;
extern size_t numGamesIncrement;
extern size_t startingMoves;
extern uint32_t timeControl;
extern uint32_t timeControlIncrement;
extern uint32_t increment;
extern uint32_t incrementIncrement;
extern int startOutputAtMove;
extern bool useNoisyParameters;
extern double noiseDefaultStdDev;
extern double noiseLinearStdDev;
extern double noiseDecay;

/**
 * Variablen des Trainings.
 */

extern double validationSplit;
extern double k;
extern double learningRate;
extern double learningRateDecay;
extern size_t numEpochs;
extern size_t numEpochsIncrement;
extern size_t numGenerations;
extern size_t noImprovementPatience;
extern size_t batchSize;
extern double epsilon;
extern double discount;
extern double beta1;
extern double beta2;
extern double weightDecay;

#endif