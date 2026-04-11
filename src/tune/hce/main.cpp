#include "core/chess/Board.h"
#include "core/utils/magics/Magics.h"
#include "tune/Simulation.h"
#include "tune/Definitions.h"
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
                   double noiseLinearStdDev, double noiseDecay, std::istream& pgnFile, std::ostream& outFile,
                   std::optional<HCEParameters> params = std::nullopt);

void generateData() {
    std::ifstream pgnFile(pgnFilePath.get<std::string>());
    std::ofstream resultFile(samplesFilePath.get<std::string>(), std::ios::app);

    simulateGames(numGames.get<size_t>(), timeControl.get<uint32_t>(), increment.get<uint32_t>(), 
                  useNoisyParameters.get<bool>(), noiseDefaultStdDev.get<double>(), noiseLinearStdDev.get<double>(),
                  noiseDecay.get<double>(), pgnFile, resultFile);

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
                   double noiseLinearStdDev, double noiseDecay, std::istream& pgnFile, std::ostream& outFile,
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
    if(!startingPositions.empty())
        startingPositions.pop_back();

    std::cout << "\rLoaded " << startingPositions.size() << " games" << std::endl;

    // Wähle n zufällige Positionen aus
    std::shuffle(startingPositions.begin(), startingPositions.end(), generator);
    if(startingPositions.size() > n)
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

    if(params.has_value())
        sim.setParams(params.value());

    if(useNoisyParameters) {
        sim.setParameterNoise(noiseDefaultStdDev);
        sim.setLinearParameterNoise(noiseLinearStdDev);
        sim.setNoiseDecay(noiseDecay);
    }
    sim.run();

    std::cout << "Writing results to file..." << std::endl;
    std::cout << "Remaining games: " << startingPositions.size() << std::flush;

    std::vector<Result>& results = sim.getResults();

    for(size_t i = 0; i < startingPositions.size(); i++) {
        int outputSize = (int)startingPositions[i].getAge() - (int)startingMoves[i];
        if(outputSize <= 0)
            continue;

        std::vector<std::string> output;
        output.resize(outputSize);

        int finalResult;
        switch(results[i].result) {
            case WHITE_WIN:
                finalResult = 1;
                break;
            case BLACK_WIN:
                finalResult = -1;
                break;
            default:
                finalResult = 0;
                break;
        }

        for(int j = (int)results[i].leafEvaluations.size() - 1; j >= 0; j--) {
            // Mache den letzten Zug rückgängig, das Spiel ist da schon vorbei
            startingPositions[i].undoMove();

            std::stringstream ss;
            ss << startingPositions[i].toFEN() << ";" << results[i].leafFENs[j] << ";" << results[i].leafEvaluations[j] << ";" << finalResult;
            output[j] = ss.str();
        }

        for(int j = 0; j < outputSize; j++)
            outFile << output[j] << "\n";

        outFile << "NEW_GAME\n";

        if(i % 10 == 0)
            std::cout << "\rRemaining games: " << std::left << std::setw(7) <<
            startingPositions.size() - i << std::right << std::flush;
    }

    std::cout << "\rRemaining games: 0      " << std::endl;
}

void calculateTDTargets(std::vector<DataPoint>& dest, std::vector<DataPoint>& rawData) {
    double terminalValue = 0.0;
    int finalResult = rawData.empty() ? 0 : rawData.back().finalResult;
    
    if(finalResult == 1)
        terminalValue = virtualMateScore.get<double>();
    else if(finalResult == -1)
        terminalValue = -virtualMateScore.get<double>();
    
    double tdTarget = terminalValue;
    double currentLambda = lambda.get<double>();
    double currentDiscount = discount.get<double>();
    int mateScore = virtualMateScore.get<int>();

    for(int j = (int)rawData.size() - 1; j >= 0; j--) {
        int virtualizedEval = rawData[j].leafEvaluation;
        if(virtualizedEval > mateScore)
            virtualizedEval = mateScore;
        else if(virtualizedEval < -mateScore)
            virtualizedEval = -mateScore;

        rawData[j].leafEvaluation = virtualizedEval;

        double nextSearchValue;
        if(j == (int)rawData.size() - 1)
            nextSearchValue = terminalValue;
        else
            nextSearchValue = rawData[j + 1].leafEvaluation;

        tdTarget = currentDiscount * ((1.0 - currentLambda) * nextSearchValue + currentLambda * tdTarget);

        dest.push_back({rawData[j].board, rawData[j].leafBoard, rawData[j].leafEvaluation, finalResult, tdTarget});
    }
}

std::vector<DataPoint> loadData(std::istream& resultFile, size_t n) {
    std::cout << "Loading data..." << std::endl;

    std::vector<DataPoint> data;

    std::cout << "Loaded 0 data points" << std::flush;

    std::vector<DataPoint> currentGame;

    std::string line;
    while(std::getline(resultFile, line) && data.size() < n) {
        if(line == "NEW_GAME") {
            calculateTDTargets(data, currentGame);
            currentGame.clear();

            if(data.size() % 10 == 0)
                std::cout << "\rLoaded " << data.size() << " data points" << std::flush;

            continue;
        }

        if(line.empty())
            continue;

        std::stringstream ss(line);
        std::string segment;
        std::vector<std::string> parts;
        while(std::getline(ss, segment, ';'))
            parts.push_back(segment);

        if(parts.size() < 4)
            continue;

        Board board(parts[0]);
        Board leafBoard(parts[1]);
        int leafEvaluation = std::stoi(parts[2]);
        int finalResult = std::stoi(parts[3]);

        currentGame.push_back({board, leafBoard, leafEvaluation, finalResult, 0.0});
    }

    calculateTDTargets(data, currentGame);

    std::cout << "\rLoaded " << data.size() << " data points" << std::endl;

    return data;
}

void findOptimalK() {
    std::cout << "Finding optimal k..." << std::endl;

    std::ifstream samplesFile(samplesFilePath.get<std::string>());
    std::vector<DataPoint> data = loadData(samplesFile);
    samplesFile.close();

    double bestLoss = std::numeric_limits<double>::max(), loss = 1.0, bestK = 0.0;
    int i = 1;

    double centerK = 0.0025 * std::max(kappa.get<double>(), 0.01);
    double startK = centerK * 0.1;
    double endK = centerK * 2.0;
    double stepK = centerK * 0.0125;

    for(double newK = startK; newK <= endK; newK += stepK) {
        loss = Tune::loss(data, HCE_PARAMS, newK, kappa.get<double>(), weightDecay.get<double>());

        if(loss < bestLoss) {
            bestLoss = loss;
            bestK = newK;
        }

        std::stringstream ss;
        ss << "\r(" << i << ") Loss: " << loss << " with k = " << newK << " (best loss = " << bestLoss <<
              ", best k = " << bestK << ")" << " at discount = " << discount.get<double>();
        std::cout << std::left << std::setw(120) << ss.str() << std::right << std::flush;

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

    HCEParameters bestHCEParams = Tune::adam(data, currentHCEParams, numEpochs.get<size_t>(), learningRate.get<double>());

    std::ofstream outFile("data/epochFinal.hce");
    bestHCEParams.saveParameters(outFile);

    std::cout << "Finished training" << std::endl;
}

void displayFinalEpoch() {
    std::ifstream epochFile("data/epochFinal.hce");
    if(!epochFile.good()) {
        std::cout << "No epochFinal.hce file found!" << std::endl;
        return;
    }

    HCEParameters finalHCEParams;
    finalHCEParams.loadParameters(epochFile);

    std::cout << "Final parameters:" << std::endl;
    finalHCEParams.displayParameters(std::cout);
}

void learn() {
    // Erstelle ein neuen Parametersatz, enthält die aktuellen Parameter
    HCEParameters params;

    // Lade die Trainingssession, falls sie existiert
    std::ifstream trainingSessionFile("data/trainingSession.tsession");
    if(trainingSessionFile.good()) {
        trainingSessionFile >> Tune::trainingSession;

        // Lade die Parameter aus der Trainingssession
        for(size_t i = 0; i < params.size(); i++)
            params[i] = std::round(Tune::trainingSession.exactParams[i]);

        params.unpackPSQT();
        trainingSessionFile.close();
    } else {
        // Starte von neu, wenn gewünscht
        if(startFromScratch.get<bool>()) {
            std::cout << "Starting training from scratch." << std::endl;
            // Setze alle Parameter auf 0
            for(size_t i = 0; i < params.size(); i++)
                params[i] = 0;

            params.unpackPSQT();
        }

        Tune::trainingSession = Tune::TrainingSession(params);
    }

    // Durchlaufe alle Generationen
    for(size_t i = Tune::trainingSession.generation; i < numGenerations.get<size_t>(); i++) {
        std::cout << "----------Generation " << i + 1 << "/" << numGenerations.get<size_t>() << "----------" << std::endl;

        size_t currentNumGames = numGames.get<size_t>() + numGamesIncrement.get<size_t>() * i;
        double growthFactor = std::pow(timeGrowth.get<double>(), i);
        size_t currentTimeControl = timeControl.get<size_t>() * growthFactor;
        size_t currentIncrement = increment.get<size_t>() * growthFactor;  
        size_t currentNumEpochs = numEpochs.get<size_t>() + numEpochsIncrement.get<size_t>() * i;
        double currentLearningRate = learningRate.get<double>() * std::pow(learningRateDecay.get<double>(), i);
                
        // Generiere die Datenpunkte
        std::ifstream pgnFile(pgnFilePath.get<std::string>());
        std::ofstream resultFile(samplesFilePath.get<std::string>(), std::ios::trunc);
        simulateGames(currentNumGames, currentTimeControl, currentIncrement, useNoisyParameters.get<bool>(),
                      noiseDefaultStdDev.get<double>(), noiseLinearStdDev.get<double>(), noiseDecay.get<double>(), pgnFile, resultFile, params);
        pgnFile.close();
        resultFile.close();

        // Lade die Datenpunkte
        std::ifstream samplesFile(samplesFilePath.get<std::string>());
        std::vector<DataPoint> data = loadData(samplesFile);

        // Führe den Gradientenabstieg durch
        std::cout << "Optimizing parameters:" << std::endl;
        params = Tune::adam(data, params, currentNumEpochs, currentLearningRate);

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