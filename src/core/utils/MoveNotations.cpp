#include "core/utils/MoveNotations.h"

/**
 * @brief Ein Array mit den Figurenbezeichnungen für die Figurine Notation.
 * Hinter jeder Figur steht ein Leerzeichen, weil das Figurenzeichen sonst mit dem Rest des String überlappt.
 */
const std::string pieceFigurineSymbols[7] = {
    " ", // Empty
    "♙", // Pawn
    "♘", // Knight
    "♗", // Bishop
    "♖", // Rook
    "♕", // Queen
    "♔"  // King
};

/**
 * @brief Ein Array mit den Figurenbezeichnungen für die ASCII Notation.
 */
const std::string pieceAsciiSymbols[7] = {
    " ", // Empty
    "P", // Pawn
    "N", // Knight
    "B", // Bishop
    "R", // Rook
    "Q", // Queen
    "K"  // King
};

/**
 * @brief Gibt einen Zug im Algebraischen Notationsformat mit Variablen Figurenbezeichnungen zurück.
 * Die Methode führt keine Legalitätstests durch. Es wird davon ausgegangen, dass der Zug legal ist.
 * 
 * @param move Der Zug.
 * @param board Das Brett, auf dem der Zug ausgeführt werden soll.
 * @param pieceSymbols Ein Array mit den Figurenbezeichnungen. Das Array muss 7 Elemente haben(Leer, Bauer, Springer, Läufer, Turm, Dame, König).
 */
std::string toAlgebraicNotation(const Move& move, Board& board, const std::string* pieceSymbols) {
    // Rochaden werden als O-O(Königsseite) bzw. O-O-O(Damenseite) dargestellt
    if(move.isCastle()) {
        if(move.isKingsideCastle()) {
            return "O-O";
        } else {
            return "O-O-O";
        }
    }

    std::string result = "";

    int32_t dest = move.getDestination();

    int32_t originRank = SQ2R(move.getOrigin());
    int32_t originFile = SQ2F(move.getOrigin());
    int32_t destRank = SQ2R(move.getDestination());
    int32_t destFile = SQ2F(move.getDestination());

    int32_t side = board.getSideToMove();

    int32_t movedPiece = board.pieceAt(move.getOrigin());

    // Bauernzüge werden anders dargestellt als andere Züge
    if(TYPEOF(movedPiece) == PAWN) {
        // Schlagzüge bei Bauern fangen mit der Linie und einem x an
        if(move.isCapture()) {
            result += 'a' + originFile;
            result += 'x';
        }

        // Darauf folgt das Zielfeld
        result += 'a' + destFile;
        result += '1' + destRank;

        // Bei einem Bauernumwandlung wird die Figur angegeben, in die der Bauer umgewandelt wird
        if(move.isPromotion()) {
            if(move.isPromotionQueen())
                result += pieceSymbols[QUEEN];
            else if(move.isPromotionRook())
                result += pieceSymbols[ROOK];
            else if(move.isPromotionBishop())
                result += pieceSymbols[BISHOP];
            else if(move.isPromotionKnight())
                result += pieceSymbols[KNIGHT];
        }

        // Überprüfe ob der Zug den Gegner in Schach oder Schachmatt setzt
        board.makeMove(move);

        if(board.isCheck()) {
            if(board.generateLegalMoves().size() == 0)
                result += '#'; // Schachmatt
            else
                result += '+'; // Schach
        }

        board.undoMove();

        return result;
    }

    // Bei anderen Figuren wird zuerst die Figur angegeben, die bewegt wird
    result += pieceSymbols[TYPEOF(board.pieceAt(move.getOrigin()))];

    // Bei Springer-, Läufer-, Turm- und Damezügen kann es sein, dass mehrere Figuren
    // das Zielfeld angreifen können. In diesem Fall wird der Ausgangsrang oder die Ausgangslinie
    // mit angegeben. In seltenen Fällen mit z.B. 3 Damen auf dem Brett kann es vorkommen,
    // dass sowohl der Ausgangsrang als auch die Ausgangslinie angegeben werden müssen.
    // Königszüge können nicht mehrdeutig sein, da es immer nur einen König pro Farbe geben kann.

    bool ambiguousRank = false;
    bool ambiguousFile = false;
    bool ambiguousRankAndFile = false;

    switch(TYPEOF(movedPiece)) {
        case KNIGHT: {
            Bitboard knightsAttackingDest = knightAttackBitboard(dest) & board.getPieceBitboard(side | KNIGHT);
            if(knightsAttackingDest.popcount() > 1) {
                int32_t numOriginFile = 0;
                int32_t numOriginRank = 0;

                while(knightsAttackingDest) {
                    int32_t knightSq = knightsAttackingDest.getFSB();
                    knightsAttackingDest.clearBit(knightSq);

                    if(knightSq % 8 == originFile)
                        numOriginFile++;
                    
                    if(knightSq / 8 == originRank)
                        numOriginRank++;
                }

                if(numOriginFile > 1)
                    ambiguousFile = true;
                
                if(numOriginRank > 1)
                    ambiguousRank = true;

                if(ambiguousFile && ambiguousRank)
                    ambiguousRankAndFile = true;
                
                if(!ambiguousFile && !ambiguousRank)
                    ambiguousRank = true;
            }
            break;
        } case BISHOP: {
            Bitboard bishopsAttackingDest = diagonalAttackBitboard(dest, board.getOccupiedBitboard() |
                                            board.getPieceBitboard(WHITE | KING) |
                                            board.getPieceBitboard(BLACK | KING)) &
                        board.getPieceBitboard(side | BISHOP);

            if(bishopsAttackingDest.popcount() > 1) {
                int32_t numOriginFile = 0;
                int32_t numOriginRank = 0;

                while(bishopsAttackingDest) {
                    int32_t bishopSq = bishopsAttackingDest.getFSB();
                    bishopsAttackingDest.clearBit(bishopSq);

                    if(bishopSq % 8 == originFile)
                        numOriginFile++;
                    
                    if(bishopSq / 8 == originRank)
                        numOriginRank++;
                }

                if(numOriginFile > 1)
                    ambiguousFile = true;
                
                if(numOriginRank > 1)
                    ambiguousRank = true;

                if(ambiguousFile && ambiguousRank)
                    ambiguousRankAndFile = true;
                
                if(!ambiguousFile && !ambiguousRank)
                    ambiguousRank = true;
            }
            break;
        } case ROOK: {
            Bitboard rooksAttackingDest = horizontalAttackBitboard(dest, board.getOccupiedBitboard() |
                                            board.getPieceBitboard(WHITE | KING) |
                                            board.getPieceBitboard(BLACK | KING)) &
                        board.getPieceBitboard(side | ROOK);

            if(rooksAttackingDest.popcount() > 1) {
                int32_t numOriginFile = 0;
                int32_t numOriginRank = 0;

                while(rooksAttackingDest) {
                    int32_t rookSq = rooksAttackingDest.getFSB();
                    rooksAttackingDest.clearBit(rookSq);

                    if(rookSq % 8 == originFile)
                        numOriginFile++;
                    
                    if(rookSq / 8 == originRank)
                        numOriginRank++;
                }

                if(numOriginFile > 1)
                    ambiguousFile = true;
                
                if(numOriginRank > 1)
                    ambiguousRank = true;

                if(ambiguousFile && ambiguousRank)
                    ambiguousRankAndFile = true;

                if(!ambiguousFile && !ambiguousRank)
                    ambiguousRank = true;
            }
            break;
        } case QUEEN: {
            Bitboard queensAttackingDest = (horizontalAttackBitboard(dest, board.getOccupiedBitboard() |
                                            board.getPieceBitboard(WHITE | KING) |
                                            board.getPieceBitboard(BLACK | KING)) |
                                            diagonalAttackBitboard(dest, board.getOccupiedBitboard() |
                                            board.getPieceBitboard(WHITE | KING) |
                                            board.getPieceBitboard(BLACK | KING))) &
                            board.getPieceBitboard(side | QUEEN);

            if(queensAttackingDest.popcount() > 1) {
                int32_t numOriginFile = 0;
                int32_t numOriginRank = 0;

                while(queensAttackingDest) {
                    int32_t queenSq = queensAttackingDest.getFSB();
                    queensAttackingDest.clearBit(queenSq);

                    if(queenSq % 8 == originFile)
                        numOriginFile++;
                    
                    if(queenSq / 8 == originRank)
                        numOriginRank++;
                }

                if(numOriginFile > 1)
                    ambiguousFile = true;
                
                if(numOriginRank > 1)
                    ambiguousRank = true;

                if(ambiguousFile && ambiguousRank)
                    ambiguousRankAndFile = true;

                if(!ambiguousFile && !ambiguousRank)
                    ambiguousRank = true;
            }
            break;
        }
    }

    if(ambiguousRankAndFile) {
        result += 'a' + originFile;
        result += '1' + originRank;
    } else if(ambiguousRank) 
        result += 'a' + originFile;
    else if(ambiguousFile)
        result += '1' + originRank;

    // Schlagzüge werden mit einem x gekennzeichnet
    if(move.isCapture())
        result += 'x';

    // Füge das Zielfeld hinzu
    result += 'a' + destFile;
    result += '1' + destRank;

    // Überprüfe ob der Zug den Gegner in Schach oder Schachmatt setzt
    board.makeMove(move);

    if(board.isCheck()) {
        if(board.generateLegalMoves().size() == 0)
            result += '#'; // Schachmatt
        else
            result += '+'; // Schach
    }

    board.undoMove();

    return result;
}

std::string toStandardAlgebraicNotation(const Move& move, Board& board) {
    return toAlgebraicNotation(move, board, pieceAsciiSymbols);
}

std::string toFigurineAlgebraicNotation(const Move& move, Board& board) {
    return toAlgebraicNotation(move, board, pieceFigurineSymbols);
}

std::vector<std::string> variationToStandardAlgebraicNotation(const std::vector<Move>& moves, Board& board) {
    std::vector<std::string> result;

    std::string moveString;

    if(board.getSideToMove() == WHITE)
        moveString += std::to_string(board.getPly() / 2 + 1) + ".";
    else
        moveString += std::to_string(board.getPly() / 2 + 1) + "... ";

    for(Move m : moves) {
        moveString += toStandardAlgebraicNotation(m, board);
        result.push_back(moveString);
        board.makeMove(m);

        moveString.clear();

        if(board.getSideToMove() == WHITE)
            moveString += std::to_string(board.getPly() / 2 + 1) + ".";
    }

    for(size_t i = 0; i < moves.size(); i++)
        board.undoMove();

    return result;
}

std::vector<std::string> variationToFigurineAlgebraicNotation(const std::vector<Move>& moves, Board& board, int32_t customPly) {
    std::vector<std::string> result;

    std::string moveString;

    if(customPly < 0)
       customPly = board.getPly();

    if(board.getSideToMove() == WHITE)
        moveString += std::to_string(customPly / 2 + 1) + ".";
    else
        moveString += std::to_string(customPly / 2 + 1) + "... ";

    for(Move m : moves) {
        moveString += toFigurineAlgebraicNotation(m, board);
        result.push_back(moveString);
        board.makeMove(m);
        customPly++;

        moveString.clear();

        if(board.getSideToMove() == WHITE)
            moveString += std::to_string(customPly / 2 + 1) + ".";
    }

    for(size_t i = 0; i < moves.size(); i++)
        board.undoMove();

    return result;
}