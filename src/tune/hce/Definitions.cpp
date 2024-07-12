#include "tune/hce/Definitions.h"

std::string pgnFilePath = "pgn/o-deville.pgn";
std::string samplesFilePath = "data/samples.txt";

size_t numGames = 800;
size_t startingMovesMean = 18;
size_t startingMovesStdDev = 1;
uint32_t timeControl = 2000;
uint32_t increment = 200;
int32_t startOutputAtMove = 20;
bool useNoisyParameters = true;
double noiseDefaultStdDev = 0.8;
double noiseLinearStdDev = 0.09;

double validationSplit = 0.2;
double k = 0.001225;
double learningRate = 1;
size_t numEpochs = 300;
size_t noImprovementPatience = 10;
size_t batchSize = 1024;
double epsilon = 1e-8;
double discount = 0.98;