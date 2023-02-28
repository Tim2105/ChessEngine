#ifdef __EMSCRIPTEN__
#include "emscripten/WebAPI.h"

#include "core/utils/MoveNotations.h"

#include <string>

extern "C" void EMSCRIPTEN_KEEPALIVE setBoard(const char* fen) {
    board = Board(fen);
}

extern "C" void EMSCRIPTEN_KEEPALIVE initGame() {
    st.setBoard(board);
    st.setNumVariations(1);

    isAnalysis = false;
}

extern "C" void EMSCRIPTEN_KEEPALIVE initAnalysis() {
    st.setBoard(board);
    st.setNumVariations(4);
    
    isAnalysis = true;
}

extern "C" void EMSCRIPTEN_KEEPALIVE search(int32_t time) {
    st.search(time, isAnalysis);
}

extern "C" void EMSCRIPTEN_KEEPALIVE stop() {
    st.stop();
}

extern "C" bool EMSCRIPTEN_KEEPALIVE makeMove(const char* moveStr) {
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

extern "C" void EMSCRIPTEN_KEEPALIVE undoMove() {
    board.undoMove();
}

extern "C" char* EMSCRIPTEN_KEEPALIVE getLegalMoves() {
    if(legalMoves != nullptr) {
        delete[] legalMoves;
    }

    std::string moves = "[";

    for(Move move : board.generateLegalMoves()) {
        moves += "\"" + toFigurineAlgebraicNotation(move, board) + "\",";
    }


    if(moves.length() > 1)
        moves.pop_back();
    
    moves += "]";

    legalMoves = new char[moves.length() + 1];
    strcpy(legalMoves, moves.c_str());

    return legalMoves;
}

extern "C" char* EMSCRIPTEN_KEEPALIVE getVariationAnalysis() {
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

#endif