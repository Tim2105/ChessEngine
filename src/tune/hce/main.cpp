#ifdef TUNE
#ifdef USE_HCE

#include "core/chess/Board.h"
#include "core/utils/magics/Magics.h"
#include "tune/Simulation.h"
#include "tune/hce/Definitions.h"
#include "tune/hce/Tune.h"
#include "uci/Options.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>

void simulateGames(size_t n, std::istream& pgnFile, std::ostream& outFile);

void generateData() {
    std::ifstream pgnFile(pgnFilePath);
    std::ofstream resultFile(samplesFilePath);

    simulateGames(numGames, pgnFile, resultFile);
}

std::vector<DataPoint> loadData(std::istream& resultFile, size_t n = std::numeric_limits<size_t>::max());

void findOptimalK();

void gradientDescent();

void displayFinalEpoch();

void learn();

void setParameter(std::string parameter, std::string value) {
    try {
        if(parameter == "numGames")
            numGames = std::stoi(value);
        else if(parameter == "timeControl")
            timeControl = std::stoi(value);
        else if(parameter == "increment")
            increment = std::stoi(value);
        else if(parameter == "pgnFilePath")
            pgnFilePath = value;
        else if(parameter == "samplesFilePath")
            samplesFilePath = value;
        else if(parameter == "learningRate")
            learningRate = std::stod(value);
        else if(parameter == "learningRateDecay")
            learningRateDecay = std::stod(value);
        else if(parameter == "numEpochs")
            numEpochs = std::stoi(value);
        else if(parameter == "batchSize")
            batchSize = std::stoi(value);
        else if(parameter == "k")
            k = std::stod(value);
        else if(parameter == "validationSplit")
            validationSplit = std::stod(value);
        else if(parameter == "noImprovementPatience")
            noImprovementPatience = std::stoi(value);
        else if(parameter == "startOutputAtMove")
            startOutputAtMove = std::stoi(value);
        else if(parameter == "startingMovesMean")
            startingMovesMean = std::stod(value);
        else if(parameter == "startingMovesStdDev")
            startingMovesStdDev = std::stod(value);
        else if(parameter == "evolutionDefaultVariance")
            evolutionDefaultVariance = std::stod(value);
        else if(parameter == "evolutionLinearVariance")
            evolutionLinearVariance = std::stod(value);
        else if(parameter == "numGenerations")
            numGenerations = std::stoi(value);
        else
            std::cerr << "Unknown parameter: " << parameter << std::endl;
    } catch(std::invalid_argument& e) {
        std::cerr << "Invalid value for parameter " << parameter << ": " << value << std::endl;
    }
}

void displayParameters() {
    std::cout << "numGames: " << numGames << std::endl;
    std::cout << "timeControl: " << timeControl << std::endl;
    std::cout << "increment: " << increment << std::endl;
    std::cout << "pgnFilePath: " << pgnFilePath << std::endl;
    std::cout << "samplesFilePath: " << samplesFilePath << std::endl;
    std::cout << "learningRate: " << learningRate << std::endl;
    std::cout << "learningRateDecay: " << learningRateDecay << std::endl;
    std::cout << "numEpochs: " << numEpochs << std::endl;
    std::cout << "batchSize: " << batchSize << std::endl;
    std::cout << "k: " << k << std::endl;
    std::cout << "validationSplit: " << validationSplit << std::endl;
    std::cout << "noImprovementPatience: " << noImprovementPatience << std::endl;
    std::cout << "startOutputAtMove: " << startOutputAtMove << std::endl;
    std::cout << "startingMovesMean: " << startingMovesMean << std::endl;
    std::cout << "startingMovesStdDev: " << startingMovesStdDev << std::endl;
    std::cout << "evolutionDefaultVariance: " << evolutionDefaultVariance << std::endl;
    std::cout << "evolutionLinearVariance: " << evolutionLinearVariance << std::endl;
    std::cout << "numGenerations: " << numGenerations << std::endl;
}

int main() {
    // Initialisere die Magic Bitboards
    Magics::initializeMagics();

    // Setze die UCI-Option Threads auf 1
    UCI::options["Threads"] = 1;

    std::string input;
    while(true) {
        std::cout << "> ";
        std::getline(std::cin, input);

        if(input == "quit" || input == "exit" || input == "q")
            break;

        if(input == "gen")
            generateData();
        else if(input == "findK")
            findOptimalK();
        else if(input == "grad")
            gradientDescent();
        else if(input == "learn")
            learn();
        else if(input == "dpParams")
            displayFinalEpoch();
        else if(input == "dp")
            displayParameters();
        else {
            size_t pos = input.find("=");
            if(pos != std::string::npos) {
                std::string parameter = input.substr(0, pos);
                std::string value = input.substr(pos + 1);

                // Entferne Leerzeichen am Anfang und Ende
                parameter.erase(0, parameter.find_first_not_of(" "));
                parameter.erase(parameter.find_last_not_of(" ") + 1);
                value.erase(0, value.find_first_not_of(" "));
                value.erase(value.find_last_not_of(" ") + 1);

                setParameter(parameter, value);
            }
        }
    }

    return 0;
}

void simulateGames(size_t n, std::istream& pgnFile, std::ostream& outFile) {
    std::cout << "Loading " << n << " games..." << std::endl;

    std::vector<Board> startingPositions;
    startingPositions.reserve(n);

    std::default_random_engine generator;
    std::normal_distribution<double> distribution(startingMovesMean, startingMovesStdDev);

    std::cout << "Loaded 0 games" << std::flush;

    for(size_t i = 0; i < n; i++) {
        size_t startingMoves = (size_t)distribution(generator);
        startingPositions.push_back(std::get<0>(Board::fromPGN(pgnFile, startingMoves)));

        if(i % 10 == 0)
            std::cout << "\rLoaded " << i << " games" << std::flush;
    }

    std::cout << "\rLoaded " << startingPositions.size() << " games" << std::endl;

    Simulation sim(startingPositions, timeControl, increment);
    sim.run();

    std::cout << "Writing results to file..." << std::endl;
    std::cout << "Remaining games: " << startingPositions.size() << std::flush;

    std::vector<GameResult> results = sim.getResults();

    for(size_t i = 0; i < startingPositions.size(); i++) {
        std::vector<std::string> output;
        int32_t outputSize = std::max(0, startingPositions[i].getAge() - startOutputAtMove);
        output.resize(outputSize);

        for(int32_t j = startingPositions[i].getAge(); j > startOutputAtMove; j--) {
            std::stringstream ss;
            ss << startingPositions[i].toFEN() << ";";
            switch(results[i]) {
                case WHITE_WIN:
                    if(startingPositions[i].getSideToMove() == WHITE)
                        ss << "1";
                    else
                        ss << "0";
                    break;
                case BLACK_WIN:
                    if(startingPositions[i].getSideToMove() == BLACK)
                        ss << "1";
                    else
                        ss << "0";
                    break;
                case DRAW:
                    ss << "0.5";
                    break;
            }

            output[j - startOutputAtMove - 1] = ss.str();
            startingPositions[i].undoMove();
        }

        for(int32_t j = 0; j < outputSize; j++)
            outFile << output[j] << "\n";

        if(i % 10 == 0)
            std::cout << "\rRemaining games: " << std::left << std::setw(7) <<
            startingPositions.size() - i << std::right << std::flush;
    }

    std::cout << "\rRemaining games: 0      " << std::endl;
}

std::vector<DataPoint> loadData(std::istream& resultFile, size_t n) {
    std::cout << "Loading data..." << std::endl;

    std::vector<DataPoint> data;

    std::cout << "Loaded 0 data points" << std::flush;

    for(size_t i = 0; i < n && !resultFile.eof(); i++) {
        std::string fen;
        std::getline(resultFile, fen, ';');

        if(fen.empty())
            break;

        Board board(fen);

        std::string result;
        std::getline(resultFile, result);

        double res = std::stod(result);

        data.push_back({board, res});

        if(i % 1000 == 0)
            std::cout << "\rLoaded " << i << " data points" << std::flush;
    }

    std::cout << "\rLoaded " << data.size() << " data points" << std::endl;

    return data;
}

void findOptimalK() {
    std::cout << "Finding optimal k..." << std::endl;

    std::ifstream samplesFile(samplesFilePath);
    std::vector<DataPoint> data = loadData(samplesFile);
    samplesFile.close();

    double k = 0;
    double bestLoss = std::numeric_limits<double>::max();

    for(int32_t i = 1; i < 81; i++) {
        double newK = 2e-6 * i * i;
        double newLoss = Tune::loss(data, HCE_PARAMS, newK);

        if(newLoss < bestLoss) {
            bestLoss = newLoss;
            k = newK;
        }

        std::stringstream ss;
        ss << "\r(" << i << "/" << 80 << ") Best loss: " << bestLoss << " with k = " << k;
        std::cout << std::left << std::setw(50) << ss.str() << std::right << std::flush;
    }

    std::cout << std::endl;
}

void gradientDescent() {
    HCEParameters currentHCEParams;

    // Setze alle Parameter auf 0
    for(size_t i = 0; i < currentHCEParams.size(); i++)
        currentHCEParams[i] = 0;

    HCEParameters bestHCEParams = currentHCEParams;

    std::ifstream samplesFile(samplesFilePath);
    std::vector<DataPoint> data = loadData(samplesFile);
    samplesFile.close();

    std::mt19937 generator;

    std::shuffle(data.begin(), data.end(), generator);

    size_t validationSize = std::round(validationSplit * data.size());
    std::vector<DataPoint> validationData(data.end() - validationSize, data.end());
    data.resize(data.size() - validationSize);

    std::vector<size_t> indices(data.size());
    std::iota(indices.begin(), indices.end(), 0);

    size_t batches = data.size() / batchSize;

    double oldLearningRate = learningRate;

    double bestLoss = std::numeric_limits<double>::max();
    size_t bestEpoch = 0;

    std::cout << "Starting Gradient Descent..." << std::endl;

    for(size_t i = 0; i < numEpochs; i++) {
        if(i % 10 == 0 && i != 0) {
            std::ofstream outFile("data/epoch" + std::to_string(i) + ".hce");
            currentHCEParams.saveParameters(outFile);
        }

        double loss = Tune::loss(validationData, currentHCEParams, k);
        if(i == 0)
            std::cout << "Initial Val loss: " << loss << std::flush;

        if(i % 10 == 1)
            std::cout << std::endl << "Epoch: " << std::left << std::setw(4) << i;
        else if(i != 0)
            std::cout << "\rEpoch: " << std::left << std::setw(4) << i;

        if(i != 0)
            std::cout << " Val loss: " << std::setw(10) << std::fixed << std::setprecision(5) << loss << std::right << std::flush;

        if(loss < bestLoss) {
            bestLoss = loss;
            bestEpoch = i;
            bestHCEParams = currentHCEParams;
        } else if(i - bestEpoch >= noImprovementPatience) {
            std::cout << std::endl << "No improvement for " << noImprovementPatience << " epochs. Stopping training." << std::endl;
            break;
        }

        // Mische die Indizes neu, wenn alle Batches durchlaufen wurden
        if(i % batches == 0 && i != 0)
            std::shuffle(indices.begin(), indices.end(), generator);

        size_t currBatch = i % batches;

        std::vector<size_t> batchIndices(indices.begin() + currBatch * batchSize, std::min(indices.begin() + (currBatch + 1) * batchSize, indices.end()));

        std::vector<double> grad = Tune::gradient(data, batchIndices, currentHCEParams, k);

        for(size_t j = 0; j < currentHCEParams.size(); j++)
            currentHCEParams[j] -= std::round(learningRate * grad[j]);

        learningRate *= learningRateDecay;
    }

    learningRate = oldLearningRate;

    std::cout << std::endl;

    double loss = Tune::loss(validationData, bestHCEParams, k);
    std::cout << "Final Val loss: " << loss << std::endl;

    std::ofstream outFile("data/epochFinal.hce");
    bestHCEParams.saveParameters(outFile);

    std::cout << "Finished training" << std::endl;
}

void displayFinalEpoch() {
    std::ifstream epochFile("data/epochFinal.hce");
    HCEParameters finalHCEParams;
    finalHCEParams.loadParameters(epochFile);

    std::cout << "Final parameters:" << std::endl;
    finalHCEParams.displayParameters(std::cout);
}

void learn() {
    std::cout << "Starting learning..." << std::endl;

    HCEParameters currentHCEParams;
    HCEParameters updatedHCEParams;

    std::random_device rd;
    std::mt19937 generator(rd());

    std::ifstream pgnFile(pgnFilePath);
    std::normal_distribution<double> startingMovesDistribution(startingMovesMean, startingMovesStdDev);

    for(size_t gen = 1; gen <= numGenerations; gen++) {
        std::cout << "Generation " << gen << std::endl;

        // Lade die aktuellen Parameter
        updatedHCEParams = currentHCEParams;

        // Ändere die Paramater zufällig ab
        std::cout << "Mutating parameters..." << std::endl;
        for(size_t i = 0; i < updatedHCEParams.size(); i++)
            if(updatedHCEParams.isOptimizable(i))
                updatedHCEParams[i] += std::normal_distribution<double>(0, evolutionDefaultVariance + evolutionLinearVariance * std::abs(updatedHCEParams[i]))(generator);
            
        // Spiele die Spiele gegen die aktuelle Version
        std::vector<Board> startingPositions;
        startingPositions.reserve(numGames);

        std::cout << "Loaded 0 games" << std::flush;

        for(size_t i = 0; i < numGames; i++) {
            size_t startingMoves = (size_t)startingMovesDistribution(generator);
            startingPositions.push_back(std::get<0>(Board::fromPGN(pgnFile, startingMoves)));
        }

        std::cout << "\rLoaded " << startingPositions.size() << " games" << std::endl;

        Simulation sim(startingPositions, timeControl, increment);
        sim.setWhiteParams(currentHCEParams);
        sim.setBlackParams(updatedHCEParams);

        sim.run();

        std::vector<GameResult>& results = sim.getResults();

        // Wandle die Ergebnisse in Datenpunkte um
        std::cout << "Converting results to data points... 0" << std::flush;
        std::vector<DataPoint> data;

        for(size_t i = 0; i < startingPositions.size(); i++) {
            Board& board = startingPositions[i];

            for(int32_t j = board.getAge(); j > startOutputAtMove; j--) {
                Board dataX(board);
                double dataY = 0;

                switch(results[i]) {
                    case WHITE_WIN:
                        if(board.getSideToMove() == WHITE)
                            dataY = 1;
                        else
                            dataY = 0;
                        break;
                    case BLACK_WIN:
                        if(board.getSideToMove() == BLACK)
                            dataY = 1;
                        else
                            dataY = 0;
                        break;
                    case DRAW:
                        dataY = 0.5;
                        break;
                }

                data.push_back({dataX, dataY});

                board.undoMove();
            }

            if(i % 10 == 0)
                std::cout << "\rConverting results to data points... " << i << std::flush;
        }

        std::cout << "\rConverting results to data points... " << startingPositions.size() << std::endl;

        // Aktualisiere die Parameter mit Gradientenabstieg
        std::cout << "Performing gradient descent... " << std::flush;

        std::shuffle(data.begin(), data.end(), generator);

        std::vector<size_t> indices(data.size());
        std::iota(indices.begin(), indices.end(), 0);

        std::vector<double> grad = Tune::gradient(data, indices, currentHCEParams, k);

        HCEParameters oldParameters = currentHCEParams;

        // Finde die größten 3 Änderungen
        std::vector<std::tuple<size_t, int16_t>> diff(currentHCEParams.size());

        double alpha = std::pow(learningRateDecay, gen - 1);

        for(size_t i = 0; i < currentHCEParams.size(); i++) {
            int16_t delta = -(int16_t)std::round(learningRate * grad[i] * alpha);
            delta = std::sqrt(std::abs(delta)) * (delta < 0 ? -1 : 1);
            currentHCEParams[i] += delta;
            diff[i] = {i, delta};
        }

        std::sort(diff.begin(), diff.end(), [](const std::tuple<size_t, int16_t>& a, const std::tuple<size_t, int16_t>& b) {
            return std::abs(std::get<1>(a)) > std::abs(std::get<1>(b));
        });

        std::cout << "Largest changes: " << std::get<0>(diff[0]) << " (" << std::get<1>(diff[0]) << "), "
                  << std::get<0>(diff[1]) << " (" << std::get<1>(diff[1]) << "), "
                  << std::get<0>(diff[2]) << " (" << std::get<1>(diff[2]) << ")" << std::endl;

        std::ofstream outFile("data/generation" + std::to_string(gen) + ".hce");
        currentHCEParams.saveParameters(outFile);
    }

    std::ofstream outFile("data/epochFinal.hce");
    currentHCEParams.saveParameters(outFile);

    pgnFile.close();

    std::cout << "Finished learning" << std::endl;
}

#endif
#endif