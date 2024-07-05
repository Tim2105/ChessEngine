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
        else if(parameter == "numEpochs")
            numEpochs = std::stoi(value);
        else if(parameter == "batchSize")
            batchSize = std::stoi(value);
        else if(parameter == "k")
            k = std::stod(value);
        else if(parameter == "epsilon")
            epsilon = std::stod(value);
        else if(parameter == "gamma")
            gamma = std::stod(value);
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
    std::cout << "numEpochs: " << numEpochs << std::endl;
    std::cout << "batchSize: " << batchSize << std::endl;
    std::cout << "k: " << k << std::endl;
    std::cout << "epsilon: " << epsilon << std::endl;
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
            int32_t gameEndsIn = startingPositions[i].getAge() - j;
            switch(results[i]) {
                case WHITE_WIN:
                    if(startingPositions[i].getSideToMove() == WHITE)
                        ss << gameEndsIn;
                    else
                        ss << -gameEndsIn;
                    break;
                case BLACK_WIN:
                    if(startingPositions[i].getSideToMove() == BLACK)
                        ss << gameEndsIn;
                    else
                        ss << -gameEndsIn;
                    break;
                case DRAW:
                    ss << "0";
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

        int32_t res = std::stoi(result);

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

    std::ifstream samplesFile(samplesFilePath);
    std::vector<DataPoint> data = loadData(samplesFile);
    samplesFile.close();

    HCEParameters bestHCEParams = Tune::adaGrad(data, currentHCEParams);

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

#endif
#endif