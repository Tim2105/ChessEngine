#include "core/chess/Referee.h"

bool Referee::isDraw(Board& b) {
    // Patt
    if(!b.isCheck() && b.generateLegalMoves().size() == 0)
        return true;

    // 50-Züge-Regel
    if(b.getFiftyMoveCounter() >= 100)
        return true;
    
    // Dreifache Stellungswiederholung
    if(b.repetitionCount() >= 3)
        return true;

    // Unzureichendes Material
    return isDrawByMaterial(b);
}

bool Referee::isDrawByMaterial(Board& b) {
    Bitboard whitePawns = b.getPieceBitboard(WHITE_PAWN);
    Bitboard blackPawns = b.getPieceBitboard(BLACK_PAWN);
    Bitboard whiteRooks = b.getPieceBitboard(WHITE_ROOK);
    Bitboard blackRooks = b.getPieceBitboard(BLACK_ROOK);
    Bitboard whiteQueens = b.getPieceBitboard(WHITE_QUEEN);
    Bitboard blackQueens = b.getPieceBitboard(BLACK_QUEEN);

    // Wenn Bauern, Türme oder Damen auf dem Spielfeld sind, ist noch genug Material vorhanden
    if(!(whitePawns || blackPawns || whiteRooks ||
         blackRooks || whiteQueens || blackQueens)) {

        Bitboard whiteKnights = b.getPieceBitboard(WHITE_KNIGHT);
        Bitboard blackKnights = b.getPieceBitboard(BLACK_KNIGHT);
        Bitboard whiteBishops = b.getPieceBitboard(WHITE_BISHOP);
        Bitboard blackBishops = b.getPieceBitboard(BLACK_BISHOP);
        
        // König gegen König
        if(!(whiteKnights || whiteBishops ||
             blackKnights || blackBishops))
            return true;
        
        // König und Springer gegen König
        if(!(whiteBishops || blackBishops) &&
          ((whiteKnights.popcount() == 1 && !blackKnights) || (!whiteKnights && blackKnights.popcount() == 1)))
            return true;

        int32_t numWhiteBishops = whiteBishops.popcount();
        int32_t numBlackBishops = blackBishops.popcount();
        
        // König und Läufer gegen König
        if(!(whiteKnights || blackKnights) &&
          ((numWhiteBishops == 1 && !blackBishops) || (!whiteBishops && numBlackBishops == 1)))
            return true;
        
        // König und Läufer gegen König und Läufer mit gleicher Farbe
        if(!(whiteKnights || blackKnights) &&
             numWhiteBishops == 1 && numBlackBishops == 1) {
            
            int32_t whiteBishopSq = whiteBishops.getFSB();
            int32_t blackBishopSq = blackBishops.getFSB();

            if(lightSquares.getBit(whiteBishopSq) == lightSquares.getBit(blackBishopSq))
                return true;
        }
    }

    return false;
}

bool Referee::isCheckmate(Board& b) {
    return b.isCheck() && b.generateLegalMoves().size() == 0;
}

bool Referee::isGameOver(Board& b) {
    return isCheckmate(b) || isDraw(b);
}