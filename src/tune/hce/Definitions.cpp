#include "tune/hce/Definitions.h"

std::string pgnFilePath = "pgn/o-deville.pgn";
std::string samplesFilePath = "data/samples.txt";

size_t numGames = 1000;
size_t startingMovesMean = 14;
size_t startingMovesStdDev = 1;
uint32_t timeControl = 1000;
uint32_t increment = 80;
int startOutputAtMove = 16;
bool useNoisyParameters = false;
double noiseDefaultStdDev = 1.5;
double noiseLinearStdDev = 0.1;

double validationSplit = 0.2;
double k = 0.001225;
double learningRate = 1;
size_t numEpochs = 300;
size_t noImprovementPatience = 10;
size_t batchSize = 1024;
double epsilon = 1e-8;
double discount = 0.99;