#ifdef __EMSCRIPTEN__
#include "emscripten/WebAPI.h"
#include "core/utils/MoveNotations.h"

#include <string>

Board board = Board();
StaticEvaluator evaluator(board);
SingleThreadedEngine st(evaluator);

char* fen = nullptr;
char* legalMoves = nullptr;
char* variationAnalysis = nullptr;
char* errorMsg = nullptr;

bool isAnalysis = false;
bool searchRunning = false;

void setBoard(const char* fen) {
    try {board = Board(fen);}
    catch(std::exception& e) {
        board = Board();
        
        if(errorMsg != nullptr) {
            delete[] errorMsg;
        }

        errorMsg = new char[strlen(e.what()) + 1];
        strcpy(errorMsg, e.what());
    }
}

char* getFen() {
    if(fen != nullptr) {
        delete[] fen;
    }

    std::string fenStr = board.fenString();

    fen = new char[fenStr.length() + 1];
    strcpy(fen, fenStr.c_str());

    return fen;
}

void initGame() {
    st.setBoard(board);
    st.setNumVariations(1);

    isAnalysis = false;
}

void initAnalysis(int32_t lines) {
    if(lines < 1)
        lines = 1;

    st.setBoard(board);
    st.setNumVariations(lines);
    
    isAnalysis = true;
}

void search(int32_t time) {
    bool* searchRunningPtr = &searchRunning;

    auto callbackFunc = [searchRunningPtr]() {
        *searchRunningPtr = false;
    };

    searchRunning = true;

    st.search(time, callbackFunc, !isAnalysis, true);
}

void stop() {
    st.stop();
}

bool isSearchRunning() {
    return searchRunning;
}

bool makeMove(const char* moveStr) {
    std::string moveAsStr = std::string(moveStr);

    Move move;

    for(Move m : board.generateLegalMoves()) {
        if(moveAsStr == m.toString() || moveAsStr == toStandardAlgebraicNotation(m, board)) {
            move = m;
            break;
        }
    }

    if(move.exists()) {
        board.makeMove(move);
        return true;
    }

    return false;
}

void undoMove() {
    board.undoMove();
}

char* getLegalMoves() {
    if(legalMoves != nullptr) {
        delete[] legalMoves;
    }

    std::string moves = "{";

    for(Move move : board.generateLegalMoves()) {
        moves += "\"" + toStandardAlgebraicNotation(move, board) + "\":";

        moves += "{";

        moves += "\"from\":\"" + move.toString().substr(0, 2) + "\",";
        moves += "\"to\":\"" + move.toString().substr(2, 2) + "\"";

        moves += "}";

        moves += ",";
    }

    if(moves.length() > 1)
        moves.pop_back();
    
    moves += "}";

    legalMoves = new char[moves.length() + 1];
    strcpy(legalMoves, moves.c_str());

    return legalMoves;
}

char* getVariationAnalysis() {
    if(variationAnalysis != nullptr) {
        delete[] variationAnalysis;
    }

    std::vector<Variation> variations = st.getVariations();

    std::string analysis = "{";

    analysis += "\"depth\":\"" + std::to_string(st.getLastSearchDepth()) + "\",";

    int16_t score = st.getBestMoveScore();

    if(board.getSideToMove() == BLACK)
        score *= -1;

    analysis += "\"score\":\"" + std::to_string(score) + "\",";

    analysis += "\"nodes\":\"" + std::to_string(st.getNodesSearched()) + "\",";

    Move bestMove = st.getBestMove();
    if(bestMove.exists()) {
        analysis += "\"bestMove\":\"" + toStandardAlgebraicNotation(bestMove, board) + "\",";
    }

    analysis += "\"variations\":[";

    for(Variation v : variations) {
        analysis += "{";

        int16_t score = v.score;

        if(board.getSideToMove() == BLACK)
            score *= -1;

        analysis += "\"score\":\"" + std::to_string(score) + "\",";

        analysis += "\"moves\":\"";

        for(std::string move : variationToFigurineAlgebraicNotation(v.moves, board))
            analysis += move + " ";

        analysis.pop_back();
        analysis += "\"";

        analysis += "},";
    }

    if(variations.size() > 0)
        analysis.pop_back();

    analysis += "]";
    analysis += "}";

    variationAnalysis = new char[analysis.length() + 1];
    strcpy(variationAnalysis, analysis.c_str());

    return variationAnalysis;
}

char* getErrorMsg() {
    if(errorMsg == nullptr) {
        errorMsg = new char[1];
        errorMsg[0] = '\0';
    }

    return errorMsg;
}

void clearErrorMsg() {
    if(errorMsg != nullptr) {
        delete[] errorMsg;
        errorMsg = nullptr;
    }
}

#endif