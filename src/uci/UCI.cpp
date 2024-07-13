#include "core/engine/search/PVSEngine.h"

#include "uci/Options.h"
#include "uci/PortabilityHelper.h"
#include "uci/UCI.h"

#include "test/Perft.h"

#include <iostream>
#include <sstream>
#include <thread>

Board board;
PVSEngine engine(board);

void changeHashSize(std::string value) {
    engine.setHashTableCapacity(std::stoull(value) * (1 << 20) / TT_ENTRY_SIZE);
}

UCI::Options UCI::options = {
    UCI::Option("Hash", std::to_string(TT_DEFAULT_CAPACITY * TT_ENTRY_SIZE / (1 << 20)), "1", "16384", changeHashSize),
    #if defined(DISABLE_THREADS)
        UCI::Option("Threads", "1", "1", "1"),
    #else
        UCI::Option("Threads", "1", "1", "128"),
    #endif
    UCI::Option("MultiPV", "1", "1", "256"),
    UCI::Option("Ponder", "false")
};

#if not defined(DISABLE_THREADS)
    std::thread searchThread;
#endif

bool quitFlag = false;
bool debug = false;

void readAndHandleNextCommand(std::istream& is);

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
    std::cout << ENGINE_NAME << " " << ENGINE_VERSION << std::endl << std::endl;

    std::string command;

    while(!quitFlag)
        readAndHandleNextCommand(std::cin);

    engine.stop();

    #if not defined(DISABLE_THREADS)
        if(searchThread.joinable())
            searchThread.join();
    #endif
}

void readAndHandleNextCommand(std::istream& is) {
    std::string command = getNextToken(is);

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
    else if(command == "quit")
        quitFlag = true;
}

std::string getNextToken(std::istream& is) {
    std::stringstream ss;

    is >> std::ws;

    while(is.good() && !(std::isspace(is.peek()) || is.peek() == EOF))
        ss << (char)is.get();

    while(is.good() && std::isspace(is.peek()) && is.peek() != '\n') // Skip whitespace until the next token
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
    for(UCI::Option& option : UCI::options) {
        std::cout << "option name " << option.getName();

        if(option.getType() == UCI::OptionType::Check)
            std::cout << " type check default " << option.getValue<std::string>();
        else if(option.getType() == UCI::OptionType::Spin)
            std::cout << " type spin default " << option.getValue<std::string>() <<
                " min " << option.getMinValue() << " max " << option.getMaxValue();

        std::cout << std::endl;
    }

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
    if(engine.isSearching())
        return;

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

    try {
        UCI::Option& option = UCI::options[name];

        if(option.getType() == UCI::OptionType::Check)
            option = value == "true";
        else if(option.getType() == UCI::OptionType::Spin)
            option = std::stoll(value);

        if(debug)
            std::cout << "info string Set option " << name << " to " << option.getValue<std::string>() << std::endl;
    } catch(std::exception& e) {
        if(debug)
            std::cout << "info string Error setting option " << name << " to " << value << std::endl;
    }
}

void handleRegisterCommand() {
    if(engine.isSearching())
        return;

    if(debug)
        std::cout << "info string Received register" << std::endl;
}

void handleUCINewGameCommand() {
    if(engine.isSearching())
        return;

    if(debug)
        std::cout << "info string Received ucinewgame" << std::endl;

    engine.clearHashTable();
}

void handlePositionCommand(std::string args) {
    if(engine.isSearching())
        return;

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
    if(engine.isSearching())
        return;

    if(debug)
        std::cout << "info string Received go " << args << std::endl;

    std::stringstream ss(args + "\n"); // Füge ein Zeilenende hinzu, damit getNextToken() "funktioniert"
    std::string token;

    UCI::SearchParams params;

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
            params.depth = std::stoul(getNextToken(ss));
        else if(token == "nodes")
            params.nodes = std::stoull(getNextToken(ss));
        else if(token == "searchmoves") {
            Array<Move, 256> legalMoves;
            board.generateLegalMoves(legalMoves);

            size_t ssPos = ss.tellg();
            token = getNextToken(ss);

            params.searchmoves.clear();

            while(ss.good()) {
                Move move;
                for(Move m : legalMoves) {
                    if(m.toString() == token) {
                        move = m;
                        break;
                    }
                }

                if(move.exists())
                    params.searchmoves.push_back(move);
                else
                    break;

                ssPos = ss.tellg();
                token = getNextToken(ss);
            }

            ss.seekg(ssPos);
        } else if(token == "movetime") {
            params.movetime = std::stoul(getNextToken(ss));
            params.useMovetime = true;
            params.useWBTime = false;
            params.infinite = false;
        } else if(token == "wtime") {
            params.wtime = std::stoul(getNextToken(ss));
            params.useWBTime = true;
            params.useMovetime = false;
            params.infinite = false;
        } else if(token == "btime") {
            params.btime = std::stoul(getNextToken(ss));
            params.useWBTime = true;
            params.useMovetime = false;
            params.infinite = false;
        } else if(token == "winc") {
            params.winc = std::stoul(getNextToken(ss));
            params.useWBTime = true;
            params.useMovetime = false;
            params.infinite = false;
        } else if(token == "binc") {
            params.binc = std::stoul(getNextToken(ss));
            params.useWBTime = true;
            params.useMovetime = false;
            params.infinite = false;
        } else if(token == "movestogo") {
            params.movestogo = std::stoul(getNextToken(ss));
            params.useWBTime = true;
            params.useMovetime = false;
            params.infinite = false;
        } else if(token == "mate") {
            params.mate = std::stoul(getNextToken(ss));
            params.useMovetime = false;
            params.useWBTime = false;
            params.infinite = false;
        } else if(token == "infinite") {
            params.infinite = true;
            params.ponder = false;
            params.useMovetime = false;
            params.useWBTime = false;
        } else if(token == "ponder") {
            params.ponder = true;
            params.infinite = false;
        }
    }

    if(debug) {
        std::cout << "info string depth: " << params.depth << std::endl;
        std::cout << "info string nodes: " << params.nodes << std::endl;
        std::cout << "info string movetime: " << params.movetime << std::endl;
        std::cout << "info string wtime: " << params.wtime << std::endl;
        std::cout << "info string btime: " << params.btime << std::endl;
        std::cout << "info string winc: " << params.winc << std::endl;
        std::cout << "info string binc: " << params.binc << std::endl;
        std::cout << "info string movestogo: " << params.movestogo << std::endl;
        std::cout << "info string mate: " << params.mate << std::endl;
        std::cout << "info string infinite: " << params.infinite << std::endl;
        std::cout << "info string ponder: " << params.ponder << std::endl;
    }

    #if defined(DISABLE_THREADS)
        engine.search(params);
    #else
        if(searchThread.joinable())
            searchThread.join();

        searchThread = std::thread(&PVSEngine::search, &engine, params);
    #endif
}

void handleStopCommand() {
    engine.stop();
}

void handlePonderHitCommand() {
    if(debug)
        std::cout << "info string Received ponderhit" << std::endl;
}