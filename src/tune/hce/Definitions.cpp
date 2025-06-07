#include "tune/hce/Definitions.h"

#include <cmath>
#include <thread>

std::vector<Variable*> tuneVariables;

Variable pgnFilePath("pgnFile", "Path to the PGN file for sampling opening positions", "pgn/o-deville.pgn");
Variable samplesFilePath("samplesFilePath", "Path to the file for storing generated samples", "data/samples.txt");
unsigned int nThreads = std::thread::hardware_concurrency();
Variable numThreads("numThreads", "Number of threads to use for the simulation", std::max(1u, (unsigned int)std::round(nThreads * 5.0 / 8)));
Variable numGames("numGames", "Number of games to simulate at generation 0", 10ull);
Variable numGamesIncrement("numGamesIncr", "Number of games to simulate more with each generation", 0ull);
Variable timeControl("timeControl", "Time control in ms at generation 0", 5000ull);
Variable increment("increment", "Time control increment in ms at generation 0", 500ull);
Variable timeGrowth("timeGrowth", "Time control growth factor with each generation", 1.0);
Variable openingBookMovesMin("openingBookMovesMin", "Minimum number of half moves to play from opening positions before simulation", 10ull);
Variable openingBookMovesMax("openingBookMovesMax", "Maximum number of half moves to play from opening positions before simulation", 40ull);
Variable randomMovesMin("randomMovesMin", "Minimum number of random half moves to play before simulation", 0ull);
Variable randomMovesMax("randomMovesMax", "Maximum number of random half moves to play before simulation", 2ull);
Variable useNoisyParameters("useNoisyParameters", "Use noisy parameters for the simulation", true);
Variable noiseDefaultStdDev("noiseDefaultStdDev", "Default standard deviation for noise", 0.6);
Variable noiseLinearStdDev("noiseLinearStdDev", "Additional standard deviation for noise based on the absolute value of the parameter", 0.03);
Variable noiseDecay("noiseDecay", "Noise decay factor with each generation", 0.998);
Variable validationSplit("validationSplit", "Fraction of the training data to use for validation", 0.0);
Variable k("k", "Factor multiplied with the evaluation value inside the tanh function", 0.002401);
Variable learningRate("learningRate", "Learning rate for the optimizer at generation 0", 0.2);
Variable learningRateDecay("learningRateDecay", "Learning rate decay factor with each generation", 0.998);
Variable numEpochs("numEpochs", "Number of epochs for training at generation 0", 2ull);
Variable numEpochsIncrement("numEpochsIncr", "Additional number of epochs for training with each generation", 0ull);
Variable numGenerations("numGenerations", "Number of generations for training", 50000ull);
Variable noImprovementPatience("noImprovementPatience", "Number of generations without improvement before stopping training", 20ull);
Variable batchSize("batchSize", "Batch size for training", 2048ull);
Variable epsilon("epsilon", "Epsilon value for the Adam optimizer", 1e-8);
Variable discount("discount", "Discount factor for the learning algorithm", 1.0);
Variable alpha("alpha", "Smoothing factor for the exponential moving average loss", 0.9);
Variable beta1("beta1", "First moment smoothing factor for the Adam optimizer", 0.9);
Variable beta2("beta2", "Second moment smoothing factor for the Adam optimizer", 0.999);
Variable weightDecay("weightDecay", "Weight decay factor for the Adam optimizer", 0.0);