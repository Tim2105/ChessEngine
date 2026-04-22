#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <random>

#include "core/utils/Random.h"
#include "tune/Definitions.h"
#include "tune/Simulation.h"
#include "tune/nnue/Train.h"
#include "uci/Options.h"

void simulateGames(size_t n, uint32_t timeControl, uint32_t increment, const NNUE::Network& network);
void updateEloTable(size_t n, uint32_t timeControl, uint32_t increment, EloTable<NNUE::Network>& eloTable);

void generateData() {
    simulateGames(numGames.get<size_t>(), timeControl.get<uint32_t>(), increment.get<uint32_t>(), NNUE::DEFAULT_NETWORK);
}

std::vector<DataPoint> loadData(std::istream& resultFile, size_t n = std::numeric_limits<size_t>::max());

void findOptimalK();

void gradientDescent();

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
        // else if(input == "findK")
        //     findOptimalK();
        else if(input == "grad")
            gradientDescent();
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

    if(startFromScratch.get<bool>())
        Train::kaimingInitialization(Train::trainingSession.masterWeights);
    else
        Train::trainingSession = Train::TrainingSession(NNUE::DEFAULT_NETWORK);

    NNUE::Network* network = Train::adamW(data, numEpochs.get<size_t>(), learningRate.get<double>(), kappa.get<double>());

    std::ofstream outFile("data/optimized.nnue", std::ios::binary);
    outFile << *network;

    delete network;
}

void learn() {
    Train::trainingSession.eloTable = EloTable<NNUE::Network>(eloTableSize.get<size_t>(), eloKFactor.get<double>());

    std::ifstream trainingSessionFile("data/trainingSessionNNUE.tsession");
    if(trainingSessionFile.good()) {
        trainingSessionFile >> Train::trainingSession;
        trainingSessionFile.close();

        std::cout << "Loaded training session from file." << std::endl;
    } else {
        std::cout << "Starting new training session." << std::endl;
        if(startFromScratch.get<bool>())
            Train::kaimingInitialization(Train::trainingSession.masterWeights);
        else
            Train::trainingSession = Train::TrainingSession(NNUE::DEFAULT_NETWORK);

        std::ofstream newTrainingSessionFile("data/trainingSessionNNUE.tsession");
        newTrainingSessionFile << Train::trainingSession;
        newTrainingSessionFile.close();
    }

    NNUE::Network* network = Train::trainingSession.masterWeights.toNetwork();

    // Füge den initialen "current" Spieler zur Elo-Tabelle hinzu
    if(!Train::trainingSession.eloTable.hasPlayer("current")) {
        Train::trainingSession.eloTable.addPlayer("current", eloInitialRandomPlayerElo.get<double>(), *network);
        
        // Speichere das initiale Netzwerk als current.nnue
        std::ofstream currentNetworkFile("data/current.nnue", std::ios::binary);
        currentNetworkFile << *network;
        currentNetworkFile.close();
    }

    // Füge den Initialspieler zur Elo-Tabelle hinzu
    if(Train::trainingSession.generation == 0) {
        std::string playerName = "generation0";
        double elo = Train::trainingSession.eloTable.getElo("current");
        
        if(!Train::trainingSession.eloTable.hasPlayer(playerName))
            Train::trainingSession.eloTable.addPlayer(playerName, elo, *network);

        // Speichere die Parameter als generation0.nnue
        std::ofstream outFile("data/" + playerName + ".nnue", std::ios::binary);
        outFile << *network;
        outFile.close();
    }

    // Durchlaufe Generationen
    for(size_t i = Train::trainingSession.generation; i < numGenerations.get<size_t>(); i++) {
        std::cout << "----------Generation " << i + 1 << "/" << numGenerations.get<size_t>() << "----------" << std::endl;

        size_t currentNumGames = numGames.get<size_t>() + numGamesIncrement.get<size_t>() * i;
        double growthFactor = std::pow(timeGrowth.get<double>(), i);
        size_t currentTimeControl = timeControl.get<size_t>() * growthFactor;
        size_t currentIncrement = increment.get<size_t>() * growthFactor;  
        size_t currentNumEpochs = numEpochs.get<size_t>() + numEpochsIncrement.get<size_t>() * i;
        double currentLearningRate = learningRate.get<double>() * std::pow(learningRateDecay.get<double>(), i);
                
        // Generiere die Datenpunkte
        simulateGames(currentNumGames, currentTimeControl, currentIncrement, *network);

        // Lade die Datenpunkte
        std::ifstream samplesFile(samplesFilePath.get<std::string>());
        std::vector<DataPoint> data = loadData(samplesFile);

        // Führe den Gradientenabstieg durch
        std::cout << "Optimizing parameters:" << std::endl;
        delete network;
        network = Train::adamW(data, currentNumEpochs, currentLearningRate, kappa.get<double>());

        // Speichere die Parameter als current.nnue
        std::ofstream currentNetworkFile("data/current.nnue", std::ios::binary);
        currentNetworkFile << *network;
        currentNetworkFile.close();

        // Aktualisiere den "current" Spieler in der Elo-Tabelle
        Train::trainingSession.eloTable.setCurrentData(*network);

        // Alle eloUpdatePeriod Generationen: Aktualisiere die Elo-Werte
        if((i + 1) % eloUpdatePeriod.get<size_t>() == 0) {
            std::ifstream pgnFile(pgnFilePath.get<std::string>());
            updateEloTable(eloNumGamesPerUpdate.get<size_t>(), currentTimeControl, currentIncrement, Train::trainingSession.eloTable);
            pgnFile.close();

            std::cout << "\nElo Table (Generation " << i + 1 << "):\n";
            Train::trainingSession.eloTable.write(std::cout);
            std::cout << "\n";

            std::ofstream eloTableFile("data/eloTable.txt");
            Train::trainingSession.eloTable.write(eloTableFile);
            eloTableFile.close();
        }

        // Alle eloAddPeriod Generationen: Füge einen neuen Spieler hinzu
        if((i + 1) % eloAddPeriod.get<size_t>() == 0) {
            std::string playerName = "generation" + std::to_string(i + 1);
            double elo = Train::trainingSession.eloTable.getElo("current");
            
            if(!Train::trainingSession.eloTable.hasPlayer(playerName))
                Train::trainingSession.eloTable.addPlayer(playerName, elo, *network);

            // Speichere die Parameter als generationX.nnue
            std::ofstream outFile("data/" + playerName + ".nnue", std::ios::binary);
            outFile << *network;
            outFile.close();
        }

        // Speichere die Trainingssession
        std::ofstream trainingSessionFile("data/trainingSessionNNUE.tsession");
        trainingSessionFile << Train::trainingSession;
        trainingSessionFile.close();
    }

    delete network;
    std::cout << "Finished training" << std::endl;
}