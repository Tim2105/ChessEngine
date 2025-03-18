#include "tune/hce/Definitions.h"

std::string pgnFilePath = "pgn/o-deville.pgn";
std::string samplesFilePath = "data/samples.txt";

size_t numGames = 100;
size_t numGamesIncrement = 40;
size_t startingMoves = 16;
uint32_t timeControl = 200;
uint32_t timeControlIncrement = 0;
uint32_t increment = 20;
uint32_t incrementIncrement = 0;
int startOutputAtMove = 16;
bool useNoisyParameters = true;
double noiseDefaultStdDev = 1.0;
double noiseLinearStdDev = 0.06;
double noiseDecay = 0.98;

double validationSplit = 0.2;
double k = 0.001225;
double learningRate = 1.0;
double learningRateDecay = 0.99;
size_t numEpochs = 20;
size_t numEpochsIncrement = 8;
size_t numGenerations = 50;
size_t noImprovementPatience = 25;
size_t batchSize = 256;
double epsilon = 1e-8;
double discount = 0.99;
double beta1 = 0.9;
double beta2 = 0.999;
double weightDecay = 0.0;