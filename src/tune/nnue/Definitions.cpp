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
Variable timeControl("timeControl", "Time control in ms at generation 0", 200ull);
Variable increment("increment", "Time control increment in ms at generation 0", 20ull);
Variable timeGrowth("timeGrowth", "Time control growth factor with each generation", 1.0);
Variable openingBookMovesMin("openingBookMovesMin", "Minimum number of half moves to play from opening positions before simulation", 0ull);
Variable openingBookMovesMax("openingBookMovesMax", "Maximum number of half moves to play from opening positions before simulation", 0ull);
Variable randomMovesMin("randomMovesMin", "Minimum number of random half moves to play before simulation", 0ull);
Variable randomMovesMax("randomMovesMax", "Maximum number of random half moves to play before simulation", 0ull);
Variable virtualMateScore("virtualMateScore", "Score in centipawns used for mate in TD(lambda)", 5000u);
Variable simMultiPV("simMultiPV", "Number of PV lines to calculate during simulation", 256u);
Variable temperature("temperature", "Temperature parameter in centipawns for move selection during simulation (0 = greedy, inf = uniform random)", 30.0);
Variable temperatureDecay("temperatureDecay", "Decay factor for temperature with each half move during simulation", 0.99);
Variable startFromScratch("startFromScratch", "Whether to start training from scratch or continue with existing parameters", true);
Variable validationSplit("validationSplit", "Fraction of the training data to use for validation", 0.0);
Variable k("k", "Factor multiplied with the evaluation value inside the tanh function", 0.0025);
Variable learningRate("learningRate", "Learning rate for the optimizer at generation 0", 2e-4);
Variable learningRateDecay("learningRateDecay", "Learning rate decay factor with each generation", 1.0);
Variable numEpochs("numEpochs", "Number of epochs for training at generation 0", 4ull);
Variable numEpochsIncrement("numEpochsIncr", "Additional number of epochs for training with each generation", 0ull);
Variable numGenerations("numGenerations", "Number of generations for training", 100000ull);
Variable noImprovementPatience("noImprovementPatience", "Number of generations without improvement before stopping training", 10ull);
Variable batchSize("batchSize", "Batch size for training", 16384ull);
Variable epsilon("epsilon", "Epsilon value for the AdamW optimizer", 1e-8);
Variable discount("discount", "Discount factor for the learning algorithm", 1.0);
Variable lambda("lambda", "Base lambda parameter for TD(lambda) updates", 0.95);
Variable kappa("kappa", "Weight of the final result in the target calculation", 0.0);
Variable alpha("alpha", "Smoothing factor for the exponential moving average loss", 0.9);
Variable beta1("beta1", "First moment smoothing factor for the AdamW optimizer", 0.9);
Variable beta2("beta2", "Second moment smoothing factor for the AdamW optimizer", 0.999);
Variable weightDecay("weightDecay", "Weight decay factor for the AdamW optimizer", 1e-4);
Variable eloTableSize("eloTableSize", "Maximum number of stored Elo players/checkpoints", 0ull);
Variable eloAddPeriod("eloAddPeriod", "Add a new generation snapshot to Elo table every N generations", 50ull);
Variable eloUpdatePeriod("eloUpdatePeriod", "Update Elo table every N generations", 5ull);
Variable eloNumGamesPerUpdate("eloNumGamesPerUpdate", "Number of games to play when updating the Elo table", 40ull);
Variable eloPlayerChoiceTemperature("eloPlayerChoiceTemperature", "Temperature for softmax selection of players during Elo updates", 100.0);
Variable eloOpeningBookMovesMin("eloOpeningBookMovesMin", "Minimum number of half moves to play from opening positions before simulation for Elo updates", 0ull);
Variable eloOpeningBookMovesMax("eloOpeningBookMovesMax", "Maximum number of half moves to play from opening positions before simulation for Elo updates", 30ull);
Variable eloRandomMovesMin("eloRandomMovesMin", "Minimum number of random half moves to play before simulation for Elo updates", 0ull);
Variable eloRandomMovesMax("eloRandomMovesMax", "Maximum number of random half moves to play before simulation for Elo updates", 0ull);
Variable eloKFactor("eloKFactor", "Elo K-factor for rating updates", 5.0);
Variable eloInitialRandomPlayerElo("eloInitialRandomPlayerElo", "Initial Elo assigned to current model (random-player baseline estimate)", 50.0);