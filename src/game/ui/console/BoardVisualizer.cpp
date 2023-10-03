#include "game/ui/console/BoardVisualizer.h"

#include <sstream>
#include <wchar.h>

std::string visualizeBoard(const Board& board, const std::string* pieceSymbols, size_t pieceSymbolWidth, bool flipped) {
    std::stringstream ss;

    Move lastMove = board.getLastMove();

    int32_t lastMoveFrom = lastMove.getOrigin();
    int32_t lastMoveTo = lastMove.getDestination();
    int32_t lastMoveFromRank = SQ2R(lastMoveFrom);
    int32_t lastMoveFromFile = SQ2F(lastMoveFrom);
    int32_t lastMoveToRank = SQ2R(lastMoveTo);
    int32_t lastMoveToFile = SQ2F(lastMoveTo);

    size_t boardWidth = 8 * (pieceSymbolWidth + 3) - 1;

    ss << "   +" << std::string(boardWidth, '-') << "+" << std::endl;

    int8_t rankStart = flipped ? 0 : 7;
    int8_t rankEnd = flipped ? 8 : -1;
    int8_t rankIncrement = flipped ? 1 : -1;

    for (int8_t rank = rankStart; rank != rankEnd; rank += rankIncrement) {
        int numRowPadding = pieceSymbolWidth - 2;
        ss << " " << (rank + 1) << " |";

        for (int8_t file = 0; file < 8; file++) {
            if(file != FILE_A)
                ss << "|";

            int32_t square = FR2SQ(file, rank);

            std::string separationChars = "  ";
            if(lastMove) {
                // Füge ein vertikales Trennzeichen um das Ausgangs- und das Zielfeld des letzten Zuges herum ein
                if((lastMoveFromRank == rank && lastMoveFromFile == file) ||
                   (lastMoveToRank == rank && lastMoveToFile == file))
                    separationChars = "><";
            }

            ss << separationChars[0];

            if (board.pieceAt(square) != EMPTY) {
                int32_t piece = board.pieceAt(square);

                ss << pieceSymbols[piece] << std::string(pieceSymbolWidth - 1, ' ');

            } else
                ss << std::string(pieceSymbolWidth, ' ');

            ss << separationChars[1];
        }

        ss << "|" << std::endl;

        if(rank != rankEnd - rankIncrement) {
            for(int i = 0; i < numRowPadding / 2; i++) {
                ss << "   |";
                ss << std::string(boardWidth, ' ');
                ss << "|" << std::endl;
            }

            ss << "   |";
            ss << std::string(boardWidth, '-');
            ss << "|" << std::endl;

            for(int i = 0; i < numRowPadding / 2; i++) {
                ss << "   |";
                ss << std::string(boardWidth, ' ');
                ss << "|" << std::endl;
            }
        }
    }

    ss << "   +" << std::string(boardWidth, '-') << "+" << std::endl;
    size_t spacesBetweenFiles = (pieceSymbolWidth + 2) / 2;
    ss << "    " << std::string(spacesBetweenFiles, ' ');
    for (int8_t file = 0; file < 8; file++) {
        ss << (char) ('a' + file) << std::string(pieceSymbolWidth + 2, ' ');
    }
    ss << std::endl;

    return ss.str();
}