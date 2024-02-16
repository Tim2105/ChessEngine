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
    int32_t whitePawns = b.getPieceBitboard(WHITE_PAWN).popcount();
    int32_t blackPawns = b.getPieceBitboard(BLACK_PAWN).popcount();
    int32_t whiteKnights = b.getPieceBitboard(WHITE_KNIGHT).popcount();
    int32_t blackKnights = b.getPieceBitboard(BLACK_KNIGHT).popcount();
    int32_t whiteBishops = b.getPieceBitboard(WHITE_BISHOP).popcount();
    int32_t blackBishops = b.getPieceBitboard(BLACK_BISHOP).popcount();
    int32_t whiteRooks = b.getPieceBitboard(WHITE_ROOK).popcount();
    int32_t blackRooks = b.getPieceBitboard(BLACK_ROOK).popcount();
    int32_t whiteQueens = b.getPieceBitboard(WHITE_QUEEN).popcount();
    int32_t blackQueens = b.getPieceBitboard(BLACK_QUEEN).popcount();

    // Wenn Bauern, Türme oder Damen auf dem Spielfeld sind, ist noch genug Material vorhanden
    if(!(whitePawns > 0 || blackPawns > 0 || whiteRooks > 0 ||
       blackRooks > 0 || whiteQueens > 0 || blackQueens > 0)) {
        
        // König gegen König
        if(whiteKnights == 0 && whiteBishops == 0 &&
        blackKnights == 0 && blackBishops == 0)
            return true;
        
        // König und Springer gegen König
        if(whiteBishops == 0 && blackBishops == 0 &&
        ((whiteKnights == 1 && blackKnights == 0) || (whiteKnights == 0 && blackKnights == 1)))
            return true;
        
        // König und Läufer gegen König
        if(whiteKnights == 0 && blackKnights == 0 &&
        ((whiteBishops == 1 && blackBishops == 0) || (whiteBishops == 0 && blackBishops == 1)))
            return true;
        
        // König und Läufer gegen König und Läufer mit gleicher Farbe
        if(whiteKnights == 0 && blackKnights == 0 &&
        whiteBishops == 1 && blackBishops == 1) {
            
            int32_t whiteBishopSq = b.getPieceBitboard(WHITE_BISHOP).getFSB();
            int32_t blackBishopSq = b.getPieceBitboard(BLACK_BISHOP).getFSB();

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