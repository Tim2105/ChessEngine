#include "tune/hce/Definitions.h"

std::string pgnFilePath = "pgn/o-deville.pgn";
std::string samplesFilePath = "data/samples.txt";

size_t numGames = 25;
size_t startingMovesMean = 12;
size_t startingMovesStdDev = 4;
uint32_t timeControl = 1000;
uint32_t increment = 30;
int32_t startOutputAtMove = 12;

double validationSplit = 0.1;
double k = 0.005618;
double learningRate = 1;
size_t numEpochs = 500;
size_t noImprovementPatience = 10;
size_t batchSize = 512;
double epsilon = 1e-8;
double gamma = 0.95;

double evolutionDefaultVariance = 2;
double evolutionLinearVariance = 0.02;
size_t numGenerations = 100;