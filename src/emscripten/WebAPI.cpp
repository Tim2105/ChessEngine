#ifdef __EMSCRIPTEN__
#include "emscripten/WebAPI.h"

#include "core/chess/Referee.h"
#include "core/utils/magics/Magics.h"

#include "core/engine/MinimaxEngine.h"
#include "core/engine/StaticEvaluator.h"

#include "core/utils/MoveNotations.h"

#include <memory>

// Initialisiere die Magic Bitboards
int main() {
    Magics::initializeMagics();

    return 0;
}

Board board;
StaticEvaluator evaluator(board);

SearchDetails searchDetails;

std::unique_ptr<MinimaxEngine> playEngine;
std::unique_ptr<MinimaxEngine> analysisEngine;

std::string error;
std::string fen;
std::string legalMoves;
std::string analysisData;
std::string perftData;
std::string figurineNotation;

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

/**
 * @brief Gibt den Status des Spiels zurück
 * 0: Spiel läuft
 * 1: Weiß hat gewonnen
 * 2: Schwarz hat gewonnen
 * 3: Unentschieden durch Patt
 * 4: Unentschieden durch 50-Züge-Regel
 * 5: Unentschieden durch dreifache Stellungswiederholung
 * 6: Unentschieden durch Material
 */
int32_t getGameStatus() {
    if(Referee::isCheckmate(board))
        return board.getSideToMove() == WHITE ? 2 : 1;
    
    if(!board.isCheck() && board.generateLegalMoves().size() == 0)
        return 3;

    if(board.getFiftyMoveCounter() >= 100)
        return 4;

    if(board.repetitionCount() >= 3)
        return 5;

    if(Referee::isDrawByMaterial(board))
        return 6;

    return 0;
}

void initPlayEngine() {
    playEngine = std::make_unique<MinimaxEngine>(evaluator);
}

int16_t getBestMove(int32_t remainingTime) {
    if(playEngine == nullptr) {
        error = std::string("Play engine not initialized");
        return Move::nullMove();
    }

    playEngine->search(remainingTime, true);
    return playEngine->getBestMove();
}

int16_t getBestMoveInStaticTime(int32_t time) {
    if(playEngine == nullptr) {
        error = std::string("Play engine not initialized");
        return Move::nullMove();
    }

    playEngine->search(time, false);
    return playEngine->getBestMove();
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

bool undoMove() {
    if(board.getMoveHistory().size() > 0) {
        board.undoMove();
        return true;
    } else {
        error = std::string("No moves to undo");
        return false;
    }
}

void initAnalysis(void (*callback)(), uint32_t callbackInterval) {
    analysisEngine = std::make_unique<MinimaxEngine>(evaluator, 3, callbackInterval, [&callback]() {
        searchDetails = analysisEngine->getSearchDetails();
        callback();
    });
}

bool startAnalysis() {
    if(analysisEngine == nullptr) {
        error = std::string("Analysis engine not initialized");
        return false;
    }

    analysisEngine->search(0);

    // Schreib den letzten Stand in die searchDetails
    searchDetails = analysisEngine->getSearchDetails();

    return true;
}

bool stopAnalysis() {
    if(analysisEngine == nullptr) {
        error = std::string("Analysis engine not initialized");
        return false;
    }

    analysisEngine->stop();

    return true;
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
            analysisData += std::to_string(searchDetails.variations[i].moves[j]);
            
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

char* moveToFigurineNotation(int16_t move) {
    Move m(move);
    figurineNotation = toFigurineAlgebraicNotation(m, board);
    return (char*)figurineNotation.c_str();
}

#endif