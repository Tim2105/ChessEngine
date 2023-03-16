#ifdef __EMSCRIPTEN__
#ifndef WEB_API_H
#define WEB_API_H

#include "core/engine/SingleThreadedEngine.h"
#include "core/engine/StaticEvaluator.h"

#include <emscripten.h>

Board board;
StaticEvaluator evaluator(board);
SingleThreadedEngine st(evaluator);

char* legalMoves = nullptr;
char* variationAnalysis = nullptr;
char* errorMsg = nullptr;

bool isAnalysis = false;

extern "C" void EMSCRIPTEN_KEEPALIVE setBoard(const char* fen);

extern "C" void EMSCRIPTEN_KEEPALIVE initGame();

extern "C" void EMSCRIPTEN_KEEPALIVE initAnalysis(int32_t lines);

extern "C" void EMSCRIPTEN_KEEPALIVE search(int32_t time);

extern "C" void EMSCRIPTEN_KEEPALIVE stop();

extern "C" bool EMSCRIPTEN_KEEPALIVE makeMove(const char* move);

extern "C" void EMSCRIPTEN_KEEPALIVE undoMove();

extern "C" char* EMSCRIPTEN_KEEPALIVE getLegalMoves();

extern "C" char* EMSCRIPTEN_KEEPALIVE getVariationAnalysis();

extern "C" char* EMSCRIPTEN_KEEPALIVE getErrorMsg();

#endif
#endif