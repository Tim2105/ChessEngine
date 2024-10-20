#include "tune/hce/Definitions.h"

std::string pgnFilePath = "pgn/o-deville.pgn";
std::string samplesFilePath = "data/samples.txt";

size_t numGames = 1000;
size_t startingMoves = 8;
uint32_t timeControl = 1000;
uint32_t increment = 80;
int startOutputAtMove = 8;
bool useNoisyParameters = true;
double noiseDefaultStdDev = 0.3;
double noiseLinearStdDev = 0.008;
double noiseDecay = 1.0;

double validationSplit = 0.2;
double k = 0.001225;
double learningRate = 0.5;
double learningRateDecay = 0.975;
size_t numEpochs = 600;
size_t numGenerations = 8;
size_t noImprovementPatience = 20;
size_t batchSize = 256;
double epsilon = 1e-8;
double discount = 0.99;