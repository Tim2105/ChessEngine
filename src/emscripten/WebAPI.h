#ifdef __EMSCRIPTEN__
#ifndef WEB_API_H
#define WEB_API_H

#include "core/engine/ScoutEngine.h"
#include "core/engine/StaticEvaluator.h"

#include <emscripten.h>

extern Board board;
extern StaticEvaluator evaluator;
extern ScoutEngine st;

extern char* fen;
extern char* legalMoves;
extern char* variationAnalysis;
extern char* errorMsg;

extern bool isAnalysis;
extern bool searchRunning;

EMSCRIPTEN_KEEPALIVE extern "C" void setBoard(const char* fen);

EMSCRIPTEN_KEEPALIVE extern "C" char* getFen();

EMSCRIPTEN_KEEPALIVE extern "C" void initGame();

EMSCRIPTEN_KEEPALIVE extern "C" void initAnalysis(int32_t lines);

EMSCRIPTEN_KEEPALIVE extern "C" void search(int32_t time);

EMSCRIPTEN_KEEPALIVE extern "C" void stop();

EMSCRIPTEN_KEEPALIVE extern "C" bool isSearchRunning();

EMSCRIPTEN_KEEPALIVE extern "C" bool makeMove(const char* move);

EMSCRIPTEN_KEEPALIVE extern "C" void undoMove();

EMSCRIPTEN_KEEPALIVE extern "C" char* getLegalMoves();

EMSCRIPTEN_KEEPALIVE extern "C" char* getVariationAnalysis();

EMSCRIPTEN_KEEPALIVE extern "C" char* getErrorMsg();

EMSCRIPTEN_KEEPALIVE extern "C" void clearErrorMsg();

#endif
#endif