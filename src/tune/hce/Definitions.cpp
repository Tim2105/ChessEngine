#include "tune/hce/Definitions.h"

std::string pgnFilePath = "pgn/o-deville.pgn";
std::string samplesFilePath = "data/samples.txt";

size_t numGames = 100;
size_t numGamesIncrement = 20;
size_t startingMoves = 8;
uint32_t timeControl = 400;
uint32_t timeControlIncrement = 10;
uint32_t increment = 40;
uint32_t incrementIncrement = 1;
int startOutputAtMove = 8;
bool useNoisyParameters = true;
double noiseDefaultStdDev = 5.0;
double noiseLinearStdDev = 0.02;
double noiseDecay = 0.96;

double validationSplit = 0.2;
double k = 0.001225;
double learningRate = 1.0;
double learningRateDecay = 0.975;
size_t numEpochs = 150;
size_t numEpochsIncrement = 20;
size_t numGenerations = 60;
size_t noImprovementPatience = 25;
size_t batchSize = 256;
double epsilon = 1e-8;
double discount = 0.99;