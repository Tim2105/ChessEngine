#ifndef BOARD_VISUALIZER_H
#define BOARD_VISUALIZER_H

#include "core/chess/Board.h"

#include <string>

std::string visualizeBoard(const Board& board, const std::string* pieceSymbols, size_t pieceSymbolWidth = 1, bool flipped = false);

inline std::string visualizeBoardWithFigurines(const Board& board, bool flipped = false) {
    std::string pieceSymbols[15] = {
        " ", "♙", "♘", "♗", "♖", "♕", "♔", " ",
        " ", "♟", "♞", "♝", "♜", "♛", "♚"
    };

    return visualizeBoard(board, pieceSymbols, 2, flipped);
}

inline std::string visualizeBoardWithLetters(const Board& board, bool flipped = false) {
    std::string pieceSymbols[15] = {
        " ", "P", "N", "B", "R", "Q", "K", " ",
        " ", "p", "n", "b", "r", "q", "k"
    };

    return visualizeBoard(board, pieceSymbols, 1, flipped);
}

#endif