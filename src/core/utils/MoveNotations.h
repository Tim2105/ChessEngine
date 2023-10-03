#ifndef MOVE_NOTATIONS_H
#define MOVE_NOTATIONS_H

#include "core/chess/Board.h"

#include <ostream>
#include <string>

/**
 * @brief Symbole für die Figuren. Die Indizes entsprechen den Werten der Piece-Enum-Klasse.
 */
extern const std::string pieceFigurineSymbols[];

/**
 * @brief ASCII-Symbole für die Figuren. Die Indizes entsprechen den Werten der Piece-Enum-Klasse.
 */
extern const std::string pieceAsciiSymbols[];

/**
 * @brief Konvertiert einen Zug in die Standard-Algebraische Notation.
 * Dazu ist es notwendig, dass das Spielfeld, auf dem der Zug ausgeführt wird, übergeben wird.
 * 
 * @param m Der Zug.
 * @param board Das Spielfeld.
 */
std::string toStandardAlgebraicNotation(const Move& m, Board& board);

/**
 * @brief Konvertiert einen Zug in die Figurine-Algebraische Notation.
 * Dazu ist es notwendig, dass das Spielfeld, auf dem der Zug ausgeführt wird, übergeben wird.
 * 
 * @param m Der Zug.
 * @param board Das Spielfeld.
 */
std::string toFigurineAlgebraicNotation(const Move& m, Board& board);

/**
 * @brief Konvertiert eine Variation in die Standard-Algebraische Notation.
 * Dazu ist es notwendig, dass das Spielfeld, auf dem die Variation ausgeführt wird, übergeben wird.
 * 
 * @param moves Die Variation.
 * @param board Das Spielfeld.
 */
std::vector<std::string> variationToStandardAlgebraicNotation(const std::vector<Move>& moves, Board& board);

/**
 * @brief Konvertiert eine Variation in die Figurine-Algebraische Notation.
 * Dazu ist es notwendig, dass das Spielfeld, auf dem die Variation ausgeführt wird, übergeben wird.
 * 
 * @param moves Die Variation.
 * @param board Das Spielfeld.
 * @param customPly Die Anzahl der Halbzüge, die bereits gespielt wurden. Wird diese Zahl angegeben, wird sie als Startwert für die Nummerierung der Züge verwendet.
 */
std::vector<std::string> variationToFigurineAlgebraicNotation(const std::vector<Move>& moves, Board& board, int32_t customPly = -1);

#endif