#ifndef PERFT_H
#define PERFT_H

#include "core/chess/Board.h"

uint64_t perft(Board& board, int depth);
void printPerftResults(Board& board, int depth);

#endif