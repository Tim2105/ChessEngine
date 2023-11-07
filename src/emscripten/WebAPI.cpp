#ifdef __EMSCRIPTEN__
#include "emscripten/WebAPI.h"

#include <memory>

Board board;
StaticEvaluator evaluator(board);
MinimaxEngine playEngine(evaluator);

SearchDetails searchDetails;

std::unique_ptr<MinimaxEngine> analysisEngine;

std::string error;
std::string fen;
std::string legalMoves;
std::string analysisData;

bool setBoard(const char* fen) {
    try {
        board = Board(std::string(fen));
        return true;
    } catch(std::invalid_argument& e) {
        error = std::string(e.what());
        return false;
    }
}

char* getBoard() {
    fen = board.fenString();
    return (char*)fen.c_str();
}

char* getLegalMoves() {
    legalMoves = "[";

    Array<Move, 256> moves = board.generateLegalMoves();
    for(size_t i = 0; i < moves.size(); i++) {
        legalMoves += std::to_string(moves[i].getMove());

        if(i < moves.size() - 1)
            legalMoves += ",";
    }

    legalMoves += "]";

    return (char*)legalMoves.c_str();
}

int16_t getBestMove(int32_t remainingTime) {
    playEngine.search(remainingTime, true);
    return playEngine.getBestMove();
}

bool makeMove(int16_t move) {
    if(board.isMoveLegal(move)) {
        board.makeMove(move);
        return true;
    } else {
        error = "Illegal move " + Move(move).toString();
        return false;
    }
}

void initAnalysis(void (*callback)()) {
    analysisEngine = std::make_unique<MinimaxEngine>(evaluator, 3, 50, [&callback]() {
        searchDetails = analysisEngine->getSearchDetails();
        callback();
    });
}

void startAnalysis() {
    analysisEngine->search(0);
}

void stopAnalysis() {
    analysisEngine->stop();
}

char* getAnalysisData() {
    analysisData = "{";

    analysisData += "\"depth\":" + std::to_string(searchDetails.depth) + ",";
    analysisData += "\"nodes\":" + std::to_string(searchDetails.nodesSearched) + ",";
    analysisData += "\"time\":" + std::to_string(searchDetails.timeTaken.count()) + ",";
    analysisData += "\"variations\":[";

    for(size_t i = 0; i < searchDetails.variations.size(); i++) {
        analysisData += "{";
        analysisData += "\"score\":" + std::to_string(searchDetails.variations[i].score) + ",";
        analysisData += "\"moves\":[";
        for(size_t j = 0; j < searchDetails.variations[i].moves.size(); j++) {
            analysisData += "\"" + searchDetails.variations[i].moves[j].toString() + "\"";
            
            if(j < searchDetails.variations[i].moves.size() - 1)
                analysisData += ",";
        }
        analysisData += "]";
        analysisData += "}";

        if(i < searchDetails.variations.size() - 1)
            analysisData += ",";
    }

    analysisData += "]}";

    return (char*)analysisData.c_str();
}

char* getError() {
    return (char*)error.c_str();
}

#endif