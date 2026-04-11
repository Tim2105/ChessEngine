#include "tune/Definitions.h"

#include <cmath>
#include <thread>

std::vector<Variable*> tuneVariables;

Variable pgnFilePath("pgnFile", "Path to the PGN file for sampling opening positions", "pgn/o-deville.pgn");
Variable samplesFilePath("samplesFilePath", "Path to the file for storing generated samples", "data/samples.txt");
unsigned int nThreads = std::thread::hardware_concurrency();
Variable numThreads("numThreads", "Number of threads to use for the simulation", std::max(1u, (unsigned int)std::round(nThreads * 7.0 / 8)));
Variable numGames("numGames", "Number of games to simulate at generation 0", 25ull);
Variable numGamesIncrement("numGamesIncr", "Number of games to simulate more with each generation", 0ull);
Variable timeControl("timeControl", "Time control in ms at generation 0", 1000ull);
Variable increment("increment", "Time control increment in ms at generation 0", 100ull);
Variable timeGrowth("timeGrowth", "Time control growth factor with each generation", 1.0);
Variable openingBookMovesMin("openingBookMovesMin", "Minimum number of half moves to play from opening positions before simulation", 0ull);
Variable openingBookMovesMax("openingBookMovesMax", "Maximum number of half moves to play from opening positions before simulation", 30ull);
Variable randomMovesMin("randomMovesMin", "Minimum number of random half moves to play before simulation", 0ull);
Variable randomMovesMax("randomMovesMax", "Maximum number of random half moves to play before simulation", 0ull);
Variable virtualMateScore("virtualMateScore", "Score in centipawns used for mate in TD(lambda)", 5000u);
Variable startFromScratch("startFromScratch", "Whether to start training from scratch or continue with existing parameters", true);
Variable validationSplit("validationSplit", "Fraction of the training data to use for validation", 0.0);
Variable k("k", "Factor multiplied with the evaluation value inside the tanh function", 0.0025);
Variable learningRate("learningRate", "Learning rate for the optimizer at generation 0", 0.5);
Variable learningRateDecay("learningRateDecay", "Learning rate decay factor with each generation", 0.999);
Variable numEpochs("numEpochs", "Number of epochs for training at generation 0", 5ull);
Variable numEpochsIncrement("numEpochsIncr", "Additional number of epochs for training with each generation", 0ull);
Variable numGenerations("numGenerations", "Number of generations for training", 50000ull);
Variable noImprovementPatience("noImprovementPatience", "Number of generations without improvement before stopping training", 10ull);
Variable batchSize("batchSize", "Batch size for training", 2048ull);
Variable epsilon("epsilon", "Epsilon value for the Adam optimizer", 1e-8);
Variable discount("discount", "Discount factor for the learning algorithm", 1.0);
Variable lambda("lambda", "Lambda parameter for TD(lambda) updates", 0.95);
Variable kappa("kappa", "Weight of the final result in the target calculation", 0.0);
Variable alpha("alpha", "Smoothing factor for the exponential moving average loss", 0.9);
Variable beta1("beta1", "First moment smoothing factor for the Adam optimizer", 0.9);
Variable beta2("beta2", "Second moment smoothing factor for the Adam optimizer", 0.999);
Variable weightDecay("weightDecay", "Weight decay factor for the Adam optimizer", 0.0);

Variable useNoisyParameters("useNoisyParameters", "Use noisy parameters for the simulation", false);
Variable noiseDefaultStdDev("noiseDefaultStdDev", "Default standard deviation for noise", 5.0);
Variable noiseLinearStdDev("noiseLinearStdDev", "Additional standard deviation for noise based on the absolute value of the parameter", 0.2);
Variable noiseDecay("noiseDecay", "Noise decay factor with each generation", 0.99);
