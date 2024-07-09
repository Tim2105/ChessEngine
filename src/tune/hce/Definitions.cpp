#include "tune/hce/Definitions.h"

std::string pgnFilePath = "pgn/o-deville.pgn";
std::string samplesFilePath = "data/samples.txt";

size_t numGames = 500;
size_t startingMovesMean = 20;
size_t startingMovesStdDev = 1;
uint32_t timeControl = 1000;
uint32_t increment = 100;
int32_t startOutputAtMove = 20;

double validationSplit = 0.2;
double k = 0.0016;
double learningRate = 1;
size_t numEpochs = 200;
size_t noImprovementPatience = 8;
size_t batchSize = 1024;
double epsilon = 1e-8;
double discount = 0.99;

size_t numGenerations = 2;