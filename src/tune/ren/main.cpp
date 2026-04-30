#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <random>

#include "core/utils/Random.h"
#include "tune/Definitions.h"
#include "tune/Simulation.h"
#include "tune/ren/RENMasterWeights.h"
#include "tune/ren/Train.h"
#include "uci/Options.h"

void simulateGames(size_t n, uint32_t timeControl, uint32_t increment, const NNUE::Network& network);
void updateEloTable(size_t n, uint32_t timeControl, uint32_t increment, EloTable<NNUE::Network>& eloTable);

void generateData() {
    simulateGames(numGames.get<size_t>(), timeControl.get<uint32_t>(), increment.get<uint32_t>(), NNUE::DEFAULT_NETWORK);
}

std::vector<DataPoint> loadData(std::istream& resultFile, size_t n = std::numeric_limits<size_t>::max());

void findOptimalK();

void gradientDescent();

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
        // else if(input == "findK")
        //     findOptimalK();
        else if(input == "grad")
            gradientDescent();
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

void simulateGames(size_t n, uint32_t timeControl, uint32_t increment, const NNUE::Network& network) {
    std::ifstream pgnFile(pgnFilePath.get<std::string>());

    std::vector<Board> startingPositions;
    std::vector<unsigned int> startingMoves;
    startingPositions.reserve(n);
    startingMoves.reserve(n);

    std::mt19937& generator = Random::generator<5>();
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

    pgnFile.close();

    size_t numPVs = simMultiPV.get<size_t>();
    numPVs = std::max(numPVs, (size_t)1);
    UCI::options["MultiPV"] = numPVs;

    Simulation sim(startingPositions, timeControl, increment, numThreads.get<size_t>());

    sim.setParams(network);
    sim.setTemperature(temperature.get<double>());
    sim.setTemperatureDecay(temperatureDecay.get<double>());

    sim.run();

    std::ofstream outFile(samplesFilePath.get<std::string>(), std::ios::trunc);

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
            ss << startingPositions[i].toFEN() << ";" << results[i].leafFENs[j] << ";" <<
                results[i].leafEvaluations[j] << ";" << finalResult << ";" << results[i].logProbs[j];
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

    outFile.close();
}

void updateEloTable(size_t n, uint32_t timeControl, uint32_t increment, EloTable<NNUE::Network>& eloTable) {
    std::ifstream pgnFile(pgnFilePath.get<std::string>());

    std::vector<Board> startingPositions;
    startingPositions.reserve(n);

    std::mt19937& generator = Random::generator<6>();
    std::uniform_int_distribution openingBookMoves(eloOpeningBookMovesMin.get<size_t>(), eloOpeningBookMovesMax.get<size_t>());
    std::uniform_int_distribution randomMoves(eloRandomMovesMin.get<size_t>(), eloRandomMovesMax.get<size_t>());

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
    }

    pgnFile.close();

    UCI::options["MultiPV"] = 1;

    Simulation sim(startingPositions, timeControl, increment, numThreads.get<size_t>());
    sim.run(eloTable, eloPlayerChoiceTemperature.get<double>());
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

    for(int j = (int)rawData.size() - 1; j >= 0; j--) {
        double nextSearchValue;
        if(j == (int)rawData.size() - 1)
            nextSearchValue = terminalValue;
        else
            nextSearchValue = rawData[j + 1].leafEvaluation;

        tdTarget = currentDiscount * ((1.0 - currentLambda) * nextSearchValue + currentLambda * tdTarget);

        dest.push_back({rawData[j].board, rawData[j].leafBoard, rawData[j].leafEvaluation, finalResult, rawData[j].logProb, tdTarget});
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

        if(parts.size() < 5)
            continue;

        Board board(parts[0]);
        Board leafBoard(parts[1]);
        int leafEvaluation = std::stoi(parts[2]);
        int finalResult = std::stoi(parts[3]);
        double logProb = std::stod(parts[4]);

        currentGame.push_back({board, leafBoard, leafEvaluation, finalResult, logProb, 0.0});
    }

    calculateTDTargets(data, currentGame);

    std::cout << "\rLoaded " << data.size() << " data points" << std::endl;

    return data;
}

void gradientDescent() {
    std::ifstream samplesFile(samplesFilePath.get<std::string>());
    std::vector<DataPoint> data = loadData(samplesFile);
    samplesFile.close();

    // Test-Inferenz von einem zufälligen Datenpunkt
    if(!data.empty()) {
        const size_t randomIndex = Random::generator<7>()() % data.size();
        const DataPoint& dp = data[randomIndex];
        std::cout << "Testing inference on a random data point " << randomIndex << "..." << std::endl;
        std::cout << "Board: " << dp.board.toFEN() << std::endl;
        std::cout << "Leaf Board: " << dp.leafBoard.toFEN() << std::endl;
        std::cout << "Leaf Evaluation: " << dp.leafEvaluation << std::endl;
        std::cout << "Final Result: " << dp.finalResult << std::endl;
        std::cout << "Log Probability: " << dp.logProb << std::endl;
        std::cout << "TD Target: " << dp.tdTarget << std::endl;

        REN::MasterWeights masterWeights;
        Train::initializeWeights(masterWeights);

        double loss = Train::loss(data, masterWeights, k.get<double>(), kappa.get<double>());
        std::cout << "Initial loss: " << loss << std::endl;

        REN::NetworkActivations activations = masterWeights.forward(dp.board, true);
        std::cout << "Network output: " << activations.output() << std::endl;
        REN::Gradients gradients = masterWeights.backward(dp.board, activations, dp.tdTarget - activations.output(), true);
        std::cout << "Calculated gradients." << std::endl;
    }
}