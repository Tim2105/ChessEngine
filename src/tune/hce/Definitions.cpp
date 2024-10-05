#include "tune/hce/Definitions.h"

std::string pgnFilePath = "pgn/o-deville.pgn";
std::string samplesFilePath = "data/samples.txt";

size_t numGames = 300;
size_t startingMoves = 4;
uint32_t timeControl = 1000;
uint32_t increment = 80;
int startOutputAtMove = 4;
bool useNoisyParameters = true;
double noiseDefaultStdDev = 0.5;
double noiseLinearStdDev = 0.05;
double noiseDecay = 1.0;

double validationSplit = 0.2;
double k = 0.001225;
double learningRate = 3;
double learningRateDecay = 0.95;
size_t numEpochs = 150;
size_t numGenerations = 4;
size_t noImprovementPatience = 10;
size_t batchSize = 256;
double epsilon = 1e-8;
double discount = 0.99;