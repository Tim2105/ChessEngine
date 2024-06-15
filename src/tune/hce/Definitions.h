#ifndef TRAIN_DEFINITIONS_H
#define TRAIN_DEFINITIONS_H

#include "core/chess/Board.h"
#include "tune/Simulation.h"

#include <string>

struct DataPoint {
    Board board;
    double result;
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
extern size_t startingMovesMean;
extern size_t startingMovesStdDev;
extern uint32_t timeControl;
extern uint32_t increment;
extern int32_t startOutputAtMove;

/**
 * Variablen des Trainings.
 */

extern double validationSplit;
extern double k;
extern double learningRate;
extern size_t numEpochs;
extern size_t noImprovementPatience;
extern size_t batchSize;
extern double epsilon;

/**
 * Variablen des Lernalgorithmus.
 */

extern double evolutionDefaultVariance;
extern double evolutionLinearVariance;
extern size_t numGenerations;

#endif