#ifdef __EMSCRIPTEN__
#ifndef WEB_API_H
#define WEB_API_H

#include "core/engine/MinimaxEngine.h"
#include "core/engine/StaticEvaluator.h"

#include <emscripten.h>
#include <wasm_simd128.h>
#include <xmmintrin.h>

EMSCRIPTEN_KEEPALIVE bool setBoard(const char* fen);

EMSCRIPTEN_KEEPALIVE char* getBoard();

EMSCRIPTEN_KEEPALIVE char* getLegalMoves();

EMSCRIPTEN_KEEPALIVE int16_t getBestMove(int32_t remainingTime);

EMSCRIPTEN_KEEPALIVE bool makeMove(int16_t move);

EMSCRIPTEN_KEEPALIVE void initAnalysis(void (*callback)());

EMSCRIPTEN_KEEPALIVE void startAnalysis();

EMSCRIPTEN_KEEPALIVE void stopAnalysis();

EMSCRIPTEN_KEEPALIVE char* getAnalysisData();

EMSCRIPTEN_KEEPALIVE char* getError();

#endif
#endif