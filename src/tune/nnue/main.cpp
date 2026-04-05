#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <random>

#include "tune/Definitions.h"
#include "tune/Simulation.h"
#include "tune/nnue/Train.h"
#include "uci/Options.h"

void simulateGames(size_t n, uint32_t timeControl, uint32_t increment, std::istream& pgnFile,
                   std::ostream& outFile, std::optional<NNUE::Network*> network = std::nullopt);

void generateData() {
    std::ifstream pgnFile(pgnFilePath.get<std::string>());
    std::ofstream resultFile(samplesFilePath.get<std::string>(), std::ios::app);

    simulateGames(numGames.get<size_t>(), timeControl.get<uint32_t>(), increment.get<uint32_t>(), 
                  pgnFile, resultFile);

    pgnFile.close();
    resultFile.close();
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
        // else if(input == "learn")
        //     learn();
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

void simulateGames(size_t n, uint32_t timeControl, uint32_t increment, std::istream& pgnFile,
                   std::ostream& outFile, std::optional<NNUE::Network*> network) {
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

    if(network.has_value()) {
        sim.setWhiteParams(*network.value());
        sim.setBlackParams(*network.value());
    }

    sim.run();

    std::cout << "Writing results to file..." << std::endl;
    std::cout << "Remaining games: " << startingPositions.size() << std::flush;

    std::vector<Result>& results = sim.getResults();

    for(size_t i = 0; i < startingPositions.size(); i++) {
        std::vector<std::string> output;
        int outputSize = (int)(startingPositions[i].getAge() - startingMoves[i]);
        output.resize(outputSize);

        for(int j = (int)results[i].evaluations.size() - 1; j >= 0; j--) {
            // Mache den letzten Zug rückgängig, das Spiel ist da schon vorbei
            startingPositions[i].undoMove();
            std::stringstream ss;
            ss << startingPositions[i].toFEN() << ";" << results[i].evaluations[j];
            output[j] = ss.str();
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


void gradientDescent() {
    std::ifstream samplesFile(samplesFilePath.get<std::string>());
    std::vector<DataPoint> data = loadData(samplesFile);
    samplesFile.close();

    Train::MasterWeights masterWeights(NNUE::DEFAULT_NETWORK);

    double masterLoss = Train::loss(data, masterWeights, k.get<double>(), weightDecay.get<double>());
    double quantizedLoss = Train::loss(data, NNUE::DEFAULT_NETWORK, k.get<double>(), weightDecay.get<double>());

    std::cout << "Initial master loss: " << masterLoss << std::endl;
    std::cout << "Initial quantized loss: " << quantizedLoss << std::endl;
}