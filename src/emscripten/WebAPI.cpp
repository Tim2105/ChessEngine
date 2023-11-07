#ifdef __EMSCRIPTEN__
#include "emscripten/WebAPI.h"

Board board;
StaticEvaluator evaluator(board);
MinimaxEngine engine(evaluator);

std::string error;
std::string fen;

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

int16_t getBestMove(int32_t remainingTime) {
    engine.search(remainingTime, true);
    return engine.getBestMove();
}

bool makeMove(int16_t move) {
    if(board.isMoveLegal(move)) {
        board.makeMove(move);
        return true;
    } else {
        return false;
    }
}

char* getError() {
    return (char*)error.c_str();
}

#endif