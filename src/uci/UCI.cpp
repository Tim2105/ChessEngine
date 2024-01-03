#include "core/engine/search/PVSEngine.h"
#include "core/engine/evaluation/NNUEEvaluator.h"

#include "uci/PortabilityHelper.h"
#include "uci/UCI.h"

#include "test/Perft.h"

#include <iostream>
#include <sstream>

template<typename T>
class Option {
    private:
        std::string name;
        T value;
        T minValue;
        T maxValue;

    public:
        Option(const std::string& name, const T& value,
                const T& minValue,
                const T& maxValue) {
    
                this->name = name;
                this->value = value;
                this->minValue = minValue;
                this->maxValue = maxValue;
          };

        std::string getName() const {
            return name;
        };

        void setValue(const T& value) {
            this->value = std::clamp(value, minValue, maxValue);
        };

        T getValue() const {
            return value;
        };

        T getMinValue() const {
            return minValue;
        };

        T getMaxValue() const {
            return maxValue;
        };
};

Option<int> hashOption("Hash", (1 << 19) * 2 * TT_ENTRY_SIZE / 1000000, 0, 2048);
Option<bool> ponderOption("Ponder", false, false, true);
Option<int> multiPVOption("MultiPV", 1, 1, 256);

Board board;
NNUEEvaluator evaluator(board);
PVSEngine engine(evaluator);

static int64_t TIME_BETWEEN_NODE_OUTPUT = 3000;
int64_t lastNodeOutput = 0;

bool quitFlag = false;
bool debug = false;

void readAndHandleNextCommand(std::istream& is);

bool hasReceivedInput();
std::string getNextToken(std::istream& is);
std::string getNextLine(std::istream& is);

void handleUCICommand();
void handleDebugCommand(std::string args);
void handleIsReadyCommand();
void handleSetOptionCommand(std::string args);
void handleRegisterCommand();
void handleUCINewGameCommand();
void handlePositionCommand(std::string args);
void handleGoCommand(std::string args);
void handleStopCommand();
void handlePonderHitCommand();

void UCI::listen() {
    board = Board();

    std::cout << ENGINE_NAME << " " << ENGINE_VERSION << std::endl << std::endl;

    std::string command;

    while(!quitFlag)
        readAndHandleNextCommand(std::cin);
}

void readAndHandleNextCommand(std::istream& is) {
    std::string command = getNextToken(is);

    std::cout << std::endl;

    if(command == "uci")
        handleUCICommand();
    else if(command == "isready")
        handleIsReadyCommand();
    else if(command == "ucinewgame")
        handleUCINewGameCommand();
    else if(command == "stop")
        handleStopCommand();
    else if(command == "ponderhit")
        handlePonderHitCommand();
    else if(command == "position")
        handlePositionCommand(getNextLine(std::cin));
    else if(command == "go")
        handleGoCommand(getNextLine(std::cin));
    else if(command == "debug")
        handleDebugCommand(getNextLine(std::cin));
    else if(command == "setoption")
        handleSetOptionCommand(getNextLine(std::cin));
    else if(command == "register")
        handleRegisterCommand();
    else if(command == "quit") {
        quitFlag = true;
        engine.stop();
    }

    std::cout << std::endl;
}

bool hasReceivedInput() {
    return isEnterWaiting();
}

std::string getNextToken(std::istream& is) {
    std::stringstream ss;

    while(is.good() && (is.peek() == ' ' || is.peek() == '\t' || is.peek() == '\n'))
        is.get();

    while(is.good() && !(is.peek() == ' ' || is.peek() == '\t' || is.peek() == '\n'))
        ss << (char)is.get();

    while(is.good() && (is.peek() == ' ' || is.peek() == '\t'))
        is.get();
        
    return ss.str();
}

std::string getNextLine(std::istream& is) {
    std::string line;
    std::getline(is, line);

    return line;
}

void handleUCICommand() {
    if(debug)
        std::cout << "info string Received uci" << std::endl;

    std::cout << "id name " << UCI::ENGINE_NAME << " " << UCI::ENGINE_VERSION << std::endl;
    std::cout << "id author " << UCI::ENGINE_AUTHOR << std::endl;

    // Gebe alle Optionen aus
    std::cout << "option name " << hashOption.getName() << " type spin default " <<
        hashOption.getValue() << " min " << hashOption.getMinValue() << " max " <<
        hashOption.getMaxValue() << std::endl;

    std::cout << "option name " << ponderOption.getName() << " type check default " <<
        (ponderOption.getValue() ? "true" : "false") << std::endl;

    std::cout << "option name " << multiPVOption.getName() << " type spin default " <<
        multiPVOption.getValue() << " min " << multiPVOption.getMinValue() << " max " <<
        multiPVOption.getMaxValue() << std::endl;

    std::cout << "uciok" << std::endl;
}

void handleDebugCommand(std::string args) {
    if(args == "on")
        debug = true;
    else
        debug = false;
}

void handleIsReadyCommand() {
    if(debug)
        std::cout << "info string Received isready" << std::endl;

    std::cout << "readyok" << std::endl;
}

void handleSetOptionCommand(std::string args) {
    if(debug)
        std::cout << "info string Received setoption " << args << std::endl;

    std::stringstream ss(args);
    std::string token;

    std::string name;
    std::string value;

    while(ss.good()) {
        token = getNextToken(ss);

        if(token == "name")
            name = getNextToken(ss);
        else if(token == "value")
            value = getNextToken(ss);
    }

    if(debug)
        std::cout << "info string name: " << name << ", value: " << value << std::endl;

    if(name == hashOption.getName()) {
        int valueInt = std::stoi(value);
        hashOption.setValue(valueInt);
    } else if(name == ponderOption.getName()) {
        bool valueBool = value == "true";
        ponderOption.setValue(valueBool);
    } else if(name == multiPVOption.getName()) {
        int valueInt = std::stoi(value);
        multiPVOption.setValue(valueInt);
    }
}

void handleRegisterCommand() {
    if(debug)
        std::cout << "info string Received register" << std::endl;
}

void handleUCINewGameCommand() {
    if(debug)
        std::cout << "info string Received ucinewgame" << std::endl;

    engine.clearHashTable();
}

void handlePositionCommand(std::string args) {
    if(debug)
        std::cout << "info string Received position " << args << std::endl;

    std::stringstream ss(args + "\n"); // Füge ein Zeilenende hinzu, damit getNextToken() "funktioniert"
    std::string moves;

    std::string token;

    Board temp = board;
    
    // Read FEN
    token = getNextToken(ss);

    if(token == "fen") {
        std::string fen;

        while(ss.good() && !((token = getNextToken(ss)) == "moves"))
            fen += " " + token;

        if(debug)
            std::cout << "info string fen: " << fen << std::endl;

        try { temp = Board(fen); }
        catch(std::invalid_argument& e) {
            if(debug)
                std::cout << "info string " << e.what() << std::endl;

            return;
        }
    } else if(token == "startpos") {
        if(debug)
            std::cout << "info string startpos" << std::endl;

        temp = Board();

        token = getNextToken(ss);
    }

    // Read moves
    if(token == "moves")
        moves = getNextLine(ss);

    if(!moves.empty()) {
        std::stringstream ss(moves + " "); // Füge ein Leerzeichen hinzu, damit getNextToken() "funktioniert"
        std::string moveStr;

        while(ss.good()) {
            moveStr = getNextToken(ss);

            Move move;
            for(Move m : temp.generateLegalMoves()) {
                if(m.toString() == moveStr) {
                    move = m;
                    break;
                }
            }

            if(!temp.isMoveLegal(move)) {
                if(debug)
                    std::cout << "info string Invalid move: " << moveStr << std::endl;

                return;
            }

            temp.makeMove(move);
        }
    }

    board = temp;
}

void handleGoCommand(std::string args) {
    if(debug)
        std::cout << "info string Received go " << args << std::endl;

    std::stringstream ss(args + "\n"); // Füge ein Zeilenende hinzu, damit getNextToken() "funktioniert"
    std::string token;

    uint16_t depth = std::numeric_limits<uint16_t>::max();
    uint64_t nodes = std::numeric_limits<uint64_t>::max();
    int movetime = -1;
    int wtime = -1;
    int btime = -1;
    int winc = -1;
    int binc = -1;
    int movestogo = -1;
    int mate = -1;

    bool infinite = false;
    bool ponder = false;

    while(ss.good()) {
        token = getNextToken(ss);

        if(token == "perft") {
            int depth = 4;

            token = getNextToken(ss);
            if(!token.empty())
                depth = std::stoi(token);

            printPerftResults(board, depth);
            return;
        }

        if(token == "depth")
            depth = std::stoi(getNextToken(ss));
        else if(token == "nodes")
            nodes = std::stoull(getNextToken(ss));
        else if(token == "movetime")
            movetime = std::stoi(getNextToken(ss));
        else if(token == "wtime")
            wtime = std::stoi(getNextToken(ss));
        else if(token == "btime")
            btime = std::stoi(getNextToken(ss));
        else if(token == "winc")
            winc = std::stoi(getNextToken(ss));
        else if(token == "binc")
            binc = std::stoi(getNextToken(ss));
        else if(token == "movestogo")
            movestogo = std::stoi(getNextToken(ss));
        else if(token == "mate")
            mate = std::stoi(getNextToken(ss));
        else if(token == "infinite")
            infinite = true;
        else if(token == "ponder")
            ponder = true;
    }

    if(debug) {
        std::cout << "info string depth: " << depth << std::endl;
        std::cout << "info string nodes: " << nodes << std::endl;
        std::cout << "info string movetime: " << movetime << std::endl;
        std::cout << "info string wtime: " << wtime << std::endl;
        std::cout << "info string btime: " << btime << std::endl;
        std::cout << "info string winc: " << winc << std::endl;
        std::cout << "info string binc: " << binc << std::endl;
        std::cout << "info string movestogo: " << movestogo << std::endl;
        std::cout << "info string mate: " << mate << std::endl;
        std::cout << "info string infinite: " << infinite << std::endl;
        std::cout << "info string ponder: " << ponder << std::endl;
    }

    int time = movetime;
    bool timeControl = false;

    if(time == -1) {
        time = board.getSideToMove() == WHITE ? wtime : btime;
        timeControl = true;
    }

    auto callback = [depth, nodes, mate, time, timeControl]() {
        uint64_t nodesSearched = engine.getNodesSearched();
        int16_t maxDepthReached = engine.getMaxDepthReached();
        int64_t timeElapsed = engine.getElapsedTime();

        if(lastNodeOutput + TIME_BETWEEN_NODE_OUTPUT < timeElapsed) {
            lastNodeOutput = timeElapsed;

            std::cout << "info nodes " << nodesSearched <<
            " nps " << (uint64_t)(nodesSearched / (timeElapsed / 1000.0))
            << std::endl;
        }

        if(nodesSearched >= nodes) {
            engine.stop();
            return;
        } else if(maxDepthReached >= depth) {
            engine.stop();
            return;
        }

        int16_t score = engine.getBestMoveScore();
        if(isMateScore(score)) {
            int16_t mateIn = isMateIn(score);
            if(mateIn <= mate) {
                engine.stop();
                return;
            }
        }

        // Überprüfe, ob neue Befehle vorliegen
        if(hasReceivedInput()) {
            std::string command = getSkippedInput();

            if(command == "stop") {
                handleStopCommand();
            } else if(command == "isready") {
                handleIsReadyCommand();
            } else if(command == "ponderhit") {
                if(debug)
                    std::cout << "info string new time " << time <<
                        ", timeControl " << timeControl << std::endl;
                
                engine.setTime(time, timeControl);

                handlePonderHitCommand();
            } else if(command == "quit") {
                quitFlag = true;
                engine.stop();
            }
        }
    };

    engine.setCheckupCallback(callback);

    lastNodeOutput = 0;

    if(infinite || ponder) {
        time = 0;
        timeControl = false;
    }

    engine.search(time, timeControl);
}

void handleStopCommand() {
    engine.stop();
}

void handlePonderHitCommand() {
    if(debug)
        std::cout << "info string Received ponderhit" << std::endl;
}