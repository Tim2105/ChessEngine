#ifdef __EMSCRIPTEN__
#ifndef WEB_API_H
#define WEB_API_H

#include "core/engine/MinimaxEngine.h"
#include "core/engine/StaticEvaluator.h"

#include <emscripten.h>
#include <wasm_simd128.h>
#include <immintrin.h>

extern "C" {

    EMSCRIPTEN_KEEPALIVE bool setBoard(const char* fen);

    EMSCRIPTEN_KEEPALIVE char* getBoard();

    EMSCRIPTEN_KEEPALIVE char* getLegalMoves();

    EMSCRIPTEN_KEEPALIVE int32_t getGameStatus();

    EMSCRIPTEN_KEEPALIVE void initPlayEngine();

    EMSCRIPTEN_KEEPALIVE int16_t getBestMove(int32_t remainingTime);

    EMSCRIPTEN_KEEPALIVE int16_t getBestMoveInStaticTime(int32_t time);

    EMSCRIPTEN_KEEPALIVE bool makeMove(int16_t move);

    EMSCRIPTEN_KEEPALIVE bool undoMove();

    EMSCRIPTEN_KEEPALIVE void initAnalysis(void (*callback)(), uint32_t callbackInterval);

    EMSCRIPTEN_KEEPALIVE bool startAnalysis();

    EMSCRIPTEN_KEEPALIVE bool stopAnalysis();

    EMSCRIPTEN_KEEPALIVE char* getAnalysisData();

    EMSCRIPTEN_KEEPALIVE char* getError();

    EMSCRIPTEN_KEEPALIVE char* moveToFigurineNotation(int16_t move);
}

#endif
#endif