#ifndef MOVE_NOTATIONS_H
#define MOVE_NOTATIONS_H

#include <string>
#include "Board.h"
#include <ostream>

extern const std::string pieceFigurineSymbols[];

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
 * Der Rückgabetyp ist ein std::u16string, da die Figurine-Symbole Unicode-Zeichen sind und die Zeichen
 * sonst in der Windows-Konsole nicht richtig dargestellt werden.
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
 * Der Rückgabetyp ist ein std::vector<std::u16string>, da die Figurine-Symbole Unicode-Zeichen sind und die Zeichen
 * sonst in der Windows-Konsole nicht richtig dargestellt werden.
 * 
 * @param moves Die Variation.
 * @param board Das Spielfeld.
 */
std::vector<std::string> variationToFigurineAlgebraicNotation(const std::vector<Move>& moves, Board& board);

#endif