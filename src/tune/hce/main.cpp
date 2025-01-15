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
#include <optional>
#include <random>

void simulateGames(size_t n, std::istream& pgnFile, std::ostream& outFile, std::optional<HCEParameters> params = std::nullopt);

void generateData() {
    std::ifstream pgnFile(pgnFilePath);
    std::ofstream resultFile(samplesFilePath, std::ios::app);

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
        else if(parameter == "numGenerations")
            numGenerations = std::stoi(value);
        else if(parameter == "batchSize")
            batchSize = std::stoi(value);
        else if(parameter == "k")
            k = std::stod(value);
        else if(parameter == "epsilon")
            epsilon = std::stod(value);
        else if(parameter == "discount")
            discount = std::stod(value);
        else if(parameter == "validationSplit")
            validationSplit = std::stod(value);
        else if(parameter == "noImprovementPatience")
            noImprovementPatience = std::stoi(value);
        else if(parameter == "startOutputAtMove")
            startOutputAtMove = std::stoi(value);
        else if(parameter == "startingMoves")
            startingMoves = std::stod(value);
        else if(parameter == "useNoisyParameters")
            useNoisyParameters = value == "true";
        else if(parameter == "noiseDefaultStdDev")
            noiseDefaultStdDev = std::stod(value);
        else if(parameter == "noiseLinearStdDev")
            noiseLinearStdDev = std::stod(value);
        else if(parameter == "noiseDecay")
            noiseDecay = std::stod(value);
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
    std::cout << "numGenerations: " << numGenerations << std::endl;
    std::cout << "batchSize: " << batchSize << std::endl;
    std::cout << "k: " << k << std::endl;
    std::cout << "epsilon: " << epsilon << std::endl;
    std::cout << "discount: " << discount << std::endl;
    std::cout << "validationSplit: " << validationSplit << std::endl;
    std::cout << "noImprovementPatience: " << noImprovementPatience << std::endl;
    std::cout << "startOutputAtMove: " << startOutputAtMove << std::endl;
    std::cout << "startingMoves: " << startingMoves << std::endl;
    std::cout << "useNoisyParameters: " << (useNoisyParameters ? "true" : "false") << std::endl;
    std::cout << "noiseDefaultStdDev: " << noiseDefaultStdDev << std::endl;
    std::cout << "noiseLinearStdDev: " << noiseLinearStdDev << std::endl;
    std::cout << "noiseDecay: " << noiseDecay << std::endl;
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
        else if(input == "learn")
            learn();
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

void simulateGames(size_t n, std::istream& pgnFile, std::ostream& outFile, std::optional<HCEParameters> params) {
    std::vector<Board> startingPositions;
    startingPositions.reserve(n);

    std::default_random_engine generator;

    std::cout << "Loaded 0 games" << std::flush;

    // Lade alle Positionen aus der PGN-Datei
    size_t i = 0;
    while(pgnFile.good()) {
        startingPositions.push_back(std::get<0>(Board::fromPGN(pgnFile, startingMoves)));
        i++;

        if(i % 10 == 0)
            std::cout << "\rLoaded " << i << " games" << std::flush;
    }

    // Entferne die letzte Position, da sie nicht vollständig geladen wurde
    startingPositions.pop_back();

    std::cout << "\rLoaded " << startingPositions.size() << " games" << std::endl;

    // Wähle n zufällige Positionen aus
    std::shuffle(startingPositions.begin(), startingPositions.end(), generator);
    startingPositions.resize(n);

    Simulation sim(startingPositions, timeControl, increment, 10);

    if(params.has_value()) {
        sim.setWhiteParams(params.value());
        sim.setBlackParams(params.value());
    }

    if(useNoisyParameters) {
        sim.setParameterNoise(noiseDefaultStdDev);
        sim.setLinearParameterNoise(noiseLinearStdDev);
    }
    sim.run();

    std::cout << "Writing results to file..." << std::endl;
    std::cout << "Remaining games: " << startingPositions.size() << std::flush;

    std::vector<GameResult> results = sim.getResults();

    for(size_t i = 0; i < startingPositions.size(); i++) {
        std::vector<std::string> output;
        int outputSize = std::max(0u, startingPositions[i].getAge() - startOutputAtMove);
        output.resize(outputSize);

        int numPlayedMoves = startingPositions[i].getAge();

        for(int j = startingPositions[i].getAge(); j > startOutputAtMove; j--) {
            std::stringstream ss;
            ss << startingPositions[i].toFEN() << ";";
            int gameEndsIn = numPlayedMoves - j + 1;
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

        for(int j = 0; j < outputSize; j++)
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

        int res = std::stoi(result);

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

    double prevLoss = std::numeric_limits<double>::max(), loss = 1.0;
    int i = 1;

    while(loss < prevLoss) {
        prevLoss = loss;
        double newK = 1e-6 * i * i;

        loss = Tune::loss(data, HCE_PARAMS, newK, discount);

        if(loss < prevLoss) {
            std::stringstream ss;
            ss << "\r(" << i << ") Loss: " << loss << " with k = " << newK << " at discount = " << discount;
            std::cout << std::left << std::setw(70) << ss.str() << std::right << std::flush;
        }

        i++;
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

void learn() {
    size_t oldNumGames = numGames;
    size_t oldTimeControl = timeControl;
    size_t oldIncrement = increment;
    size_t oldNumEpochs = numEpochs;

    // Erstelle ein neuen Parametersatz
    HCEParameters currentHCEParams;

    // Durchlaufe alle Generationen
    for(size_t i = 0; i < numGenerations; i++) {
        std::cout << "Generation " << i + 1 << "/" << numGenerations << std::endl;

        // Generiere die Datenpunkte
        std::ifstream pgnFile(pgnFilePath);
        std::ofstream resultFile(samplesFilePath, std::ios::trunc);
        double oldNoiseDefaultStdDev = noiseDefaultStdDev;
        double oldNoiseLinearStdDev = noiseLinearStdDev;
        noiseDefaultStdDev = noiseDefaultStdDev * std::pow(noiseDecay, i);
        noiseLinearStdDev = noiseLinearStdDev * std::pow(noiseDecay, i);
        simulateGames(numGames, pgnFile, resultFile, currentHCEParams);
        noiseDefaultStdDev = oldNoiseDefaultStdDev;
        noiseLinearStdDev = oldNoiseLinearStdDev;
        pgnFile.close();
        resultFile.close();

        // Lade die Datenpunkte
        std::ifstream samplesFile(samplesFilePath);
        std::vector<DataPoint> data = loadData(samplesFile);

        // Führe den Gradientenabstieg durch
        double oldLearningRate = learningRate;
        learningRate = learningRate * std::pow(learningRateDecay, i);
        currentHCEParams = Tune::adaGrad(data, currentHCEParams);
        learningRate = oldLearningRate;

        // Speichere die Parameter
        std::ofstream outFile("data/generation" + std::to_string(i) + ".hce");

        currentHCEParams.saveParameters(outFile);

        numGames += numGamesIncrement;
        timeControl += timeControlIncrement;
        increment += incrementIncrement;
        numEpochs += numEpochsIncrement;
    }

    std::cout << "Finished training" << std::endl;

    // Speichere die finalen Parameter
    std::ofstream outFile("data/epochFinal.hce");
    currentHCEParams.saveParameters(outFile);

    numGames = oldNumGames;
    timeControl = oldTimeControl;
    increment = oldIncrement;
    numEpochs = oldNumEpochs;
}

#endif
#endif