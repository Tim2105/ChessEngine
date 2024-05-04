#include "tune/hce/Definitions.h"

std::string pgnFilePath = "pgn/o-deville.pgn";
std::string samplesFilePath = "data/samples.txt";

size_t numGames = 1000;
size_t startingMovesMean = 18;
size_t startingMovesStdDev = 4;
uint32_t timeControl = 2000;
uint32_t increment = 60;
int32_t startOutputAtMove = 8;

double validationSplit = 0.1;
double k = 0.006728;
double learningRate = 10;
size_t numEpochs = 500;
size_t noImprovementPatience = 20;
size_t batchSize = 2048;