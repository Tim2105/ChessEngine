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

void simulateGames(size_t n, uint32_t timeControl, uint32_t increment, bool useNoisyParameters, double noiseDefaultStdDev,
                   double noiseLinearStdDev, std::istream& pgnFile, std::ostream& outFile,
                   std::optional<HCEParameters> params = std::nullopt);

void generateData() {
    std::ifstream pgnFile(pgnFilePath.get<std::string>());
    std::ofstream resultFile(samplesFilePath.get<std::string>(), std::ios::app);

    simulateGames(numGames.get<size_t>(), timeControl.get<uint32_t>(), increment.get<uint32_t>(), 
                  useNoisyParameters.get<bool>(), noiseDefaultStdDev.get<double>(), noiseLinearStdDev.get<double>(),
                  pgnFile, resultFile);

    pgnFile.close();
    resultFile.close();
}

std::vector<DataPoint> loadData(std::istream& resultFile, size_t n = std::numeric_limits<size_t>::max());

void findOptimalK();

void gradientDescent();

void displayFinalEpoch();

void learn();

void setParameter(std::string parameter, std::string value) {
    try {
        for(Variable* var : tuneVariables) {
            if(var->getName() == parameter) {
                var->set(value);
                std::cout << "Set parameter " << parameter << " to " << var->getAsString() << std::endl;
                break;
            }
        }
    } catch(std::invalid_argument& e) {
        std::cerr << "Invalid value for parameter " << parameter << ": " << value << std::endl;
    }
}

void displayParameters() {
    std::cout << "Current parameters:" << std::endl;
    for(Variable* var : tuneVariables)
        std::cout << var->getName() << "(" << var->getTypeString() << "): " << var->getAsString() << " - " << var->getDescription() << std::endl;
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
        else if(input == "printResult")
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

void simulateGames(size_t n, uint32_t timeControl, uint32_t increment, bool useNoisyParameters, double noiseDefaultStdDev,
                   double noiseLinearStdDev, std::istream& pgnFile, std::ostream& outFile,
                   std::optional<HCEParameters> params) {
    std::vector<Board> startingPositions;
    std::vector<unsigned int> startingMoves;
    startingPositions.reserve(n);
    startingMoves.reserve(n);

    std::mt19937 generator(std::time(0));
    std::uniform_int_distribution openingBookMoves(openingBookMovesMin.get<size_t>(), openingBookMovesMax.get<size_t>());
    std::uniform_int_distribution randomMoves(randomMovesMin.get<size_t>(), randomMovesMax.get<size_t>());

    std::cout << "Loaded 0 games" << std::flush;

    // Lade alle Positionen aus der PGN-Datei
    size_t i = 0;
    while(pgnFile.good()) {
        startingPositions.push_back(std::get<0>(Board::fromPGN(pgnFile, openingBookMoves(generator))));
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

    for(size_t i = 0; i < startingPositions.size(); i++) {
        size_t numRandomMoves = randomMoves(generator);
        for(size_t j = 0; j < numRandomMoves; j++) {
            Array<Move, 256> moves = startingPositions[i].generateLegalMoves();
            if(moves.size() == 0)
                break;

            std::uniform_int_distribution<size_t> moveDist(0, moves.size() - 1);
            startingPositions[i].makeMove(moves[moveDist(generator)]);
        }

        startingMoves.push_back(startingPositions[i].getAge());
    }

    Simulation sim(startingPositions, timeControl, increment, numThreads.get<size_t>());

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

    std::vector<GameResult>& results = sim.getResults();

    for(size_t i = 0; i < startingPositions.size(); i++) {
        std::vector<std::string> output;
        int outputSize = (int)(startingPositions[i].getAge() - startingMoves[i]);
        output.resize(outputSize);

        int numPlayedMoves = startingPositions[i].getAge();

        for(int j = (int)startingPositions[i].getAge(); j > (int)startingMoves[i]; j--) {
            std::stringstream ss;
            ss << startingPositions[i].toFEN() << ";";
            int gameEndsIn = numPlayedMoves - j;
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

            output[j - startingMoves[i] - 1] = ss.str();
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

    std::ifstream samplesFile(samplesFilePath.get<std::string>());
    std::vector<DataPoint> data = loadData(samplesFile);
    samplesFile.close();

    double prevLoss = std::numeric_limits<double>::max(), loss = 1.0;
    int i = 1;

    while(loss < prevLoss) {
        prevLoss = loss;
        double newK = 1e-8 * i * i;

        loss = Tune::loss(data, HCE_PARAMS, newK, discount.get<double>());

        if(loss < prevLoss) {
            std::stringstream ss;
            ss << "\r(" << i << ") Loss: " << loss << " with k = " << newK << " at discount = " << discount.get<double>();
            std::cout << std::left << std::setw(70) << ss.str() << std::right << std::flush;
        }

        i++;
    }

    std::cout << std::endl;
}

void gradientDescent() {
    HCEParameters currentHCEParams;

    Tune::trainingSession = Tune::TrainingSession(currentHCEParams);

    std::ifstream samplesFile(samplesFilePath.get<std::string>());
    std::vector<DataPoint> data = loadData(samplesFile);
    samplesFile.close();

    HCEParameters bestHCEParams = Tune::adamW(data, currentHCEParams, numEpochs.get<size_t>(), learningRate.get<double>());

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
    // Erstelle ein neuen Parametersatz
    HCEParameters params;

    // Lade die Trainingssession, falls sie existiert
    std::ifstream trainingSessionFile("data/trainingSession.tsession");
    if(trainingSessionFile.good())
        trainingSessionFile >> Tune::trainingSession;
    else
        Tune::trainingSession = Tune::TrainingSession(params);

    // Durchlaufe alle Generationen
    for(size_t i = Tune::trainingSession.generation; i < numGenerations.get<size_t>(); i++) {
        std::cout << "----------Generation " << i + 1 << "/" << numGenerations.get<size_t>() << "----------" << std::endl;

        size_t currentNumGames = numGames.get<size_t>() + numGamesIncrement.get<size_t>() * i;
        double growthFactor = std::pow(timeGrowth.get<double>(), i);
        size_t currentTimeControl = timeControl.get<size_t>() * growthFactor;
        size_t currentIncrement = increment.get<size_t>() * growthFactor;  
        size_t currentNumEpochs = numEpochs.get<size_t>() + numEpochsIncrement.get<size_t>() * i;
        double currentNoiseDefaultStdDev = noiseDefaultStdDev.get<double>() * std::pow(noiseDecay.get<double>(), i);
        double currentNoiseLinearStdDev = noiseLinearStdDev.get<double>() * std::pow(noiseDecay.get<double>(), i);
        double currentLearningRate = learningRate.get<double>() * std::pow(learningRateDecay.get<double>(), i);
                
        // Generiere die Datenpunkte
        std::ifstream pgnFile(pgnFilePath.get<std::string>());
        std::ofstream resultFile(samplesFilePath.get<std::string>(), std::ios::trunc);
        simulateGames(currentNumGames, currentTimeControl, currentIncrement, useNoisyParameters.get<bool>(),
                        currentNoiseDefaultStdDev, currentNoiseLinearStdDev, pgnFile, resultFile, params);
        pgnFile.close();
        resultFile.close();

        // Lade die Datenpunkte
        std::ifstream samplesFile(samplesFilePath.get<std::string>());
        std::vector<DataPoint> data = loadData(samplesFile);

        // Führe den Gradientenabstieg durch
        std::cout << "Optimizing parameters:" << std::endl;
        params = Tune::adamW(data, params, currentNumEpochs, currentLearningRate);

        // Speichere die Parameter
        std::ofstream outFile("data/generation" + std::to_string(i) + ".hce");
        params.saveParameters(outFile);
        outFile.close();

        // Speichere die Trainingssession
        std::ofstream trainingSessionFile("data/trainingSession.tsession");
        trainingSessionFile << Tune::trainingSession;
        trainingSessionFile.close();
    }

    std::cout << "Finished training" << std::endl;
}