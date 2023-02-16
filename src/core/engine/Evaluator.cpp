#include "core/engine/Evaluator.h"
#include "core/chess/MailboxDefinitions.h"
#include <algorithm>

bool Evaluator::isDraw() {
    // Fifty-move rule
    if(b.getFiftyMoveCounter() >= 100)
        return true;
    
    if(b.repetitionCount() >= 3)
        return true;

    // Unzureichendes Material
    int32_t whitePawns = b.getPieceBitboard(WHITE_PAWN).getNumberOfSetBits();
    int32_t blackPawns = b.getPieceBitboard(BLACK_PAWN).getNumberOfSetBits();
    int32_t whiteKnights = b.getPieceBitboard(WHITE_KNIGHT).getNumberOfSetBits();
    int32_t blackKnights = b.getPieceBitboard(BLACK_KNIGHT).getNumberOfSetBits();
    int32_t whiteBishops = b.getPieceBitboard(WHITE_BISHOP).getNumberOfSetBits();
    int32_t blackBishops = b.getPieceBitboard(BLACK_BISHOP).getNumberOfSetBits();
    int32_t whiteRooks = b.getPieceBitboard(WHITE_ROOK).getNumberOfSetBits();
    int32_t blackRooks = b.getPieceBitboard(BLACK_ROOK).getNumberOfSetBits();
    int32_t whiteQueens = b.getPieceBitboard(WHITE_QUEEN).getNumberOfSetBits();
    int32_t blackQueens = b.getPieceBitboard(BLACK_QUEEN).getNumberOfSetBits();

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
            
            int32_t whiteBishopSq = b.getPieceList(WHITE_BISHOP).front();
            int32_t blackBishopSq = b.getPieceList(BLACK_BISHOP).front();

            int32_t whiteBishopColor = whiteBishopSq % 20 < 10 ? whiteBishopSq % 2 : 1 - whiteBishopSq % 2;
            int32_t blackBishopColor = blackBishopSq % 20 < 10 ? blackBishopSq % 2 : 1 - blackBishopSq % 2;

            if(whiteBishopColor == blackBishopColor)
                return true;
        }
    }

    return false;
}

int32_t Evaluator::getSmallestAttacker(int32_t to, int32_t side) {
    int32_t otherSide = side ^ COLOR_MASK;
    int32_t to64 = Mailbox::mailbox[to];

    if(!b.getAttackBitboard(side).getBit(to64))
        return NO_SQ;
    
    Bitboard pawnAttackers = pawnAttackBitboard(to64, otherSide) & b.getPieceBitboard(side | PAWN);
    if(pawnAttackers)
        return Mailbox::mailbox64[pawnAttackers.getFirstSetBit()];

    Bitboard knightAttackers = knightAttackBitboard(to64) & b.getPieceBitboard(side | KNIGHT);
    if(knightAttackers)
        return Mailbox::mailbox64[knightAttackers.getFirstSetBit()];

    Bitboard bishopAttackers = diagonalAttackBitboard(to64, b.getOccupiedBitboard() | b.getPieceBitboard(side | KING))
                                                        & b.getPieceBitboard(side | BISHOP);
    if(bishopAttackers)
        return Mailbox::mailbox64[bishopAttackers.getFirstSetBit()];

    Bitboard rookAttackers = straightAttackBitboard(to64, b.getOccupiedBitboard() | b.getPieceBitboard(side | KING))
                                                        & b.getPieceBitboard(side | ROOK);
    if(rookAttackers)
        return Mailbox::mailbox64[rookAttackers.getFirstSetBit()];

    Bitboard queenAttackers = (diagonalAttackBitboard(to64, b.getOccupiedBitboard() | b.getPieceBitboard(side | KING))
                                | straightAttackBitboard(to64, b.getOccupiedBitboard() | b.getPieceBitboard(side | KING)))
                                & b.getPieceBitboard(side | QUEEN);
    if(queenAttackers)
        return Mailbox::mailbox64[queenAttackers.getFirstSetBit()];
    
    Bitboard kingAttackers = kingAttackBitboard(to64) & b.getPieceBitboard(side | KING);
    if(kingAttackers) {
        // Der König darf nur schlagen, wenn die Figur nicht verteidigt wird
        if(b.getAttackBitboard(otherSide).getBit(to64))
            return NO_SQ;

        return Mailbox::mailbox64[kingAttackers.getFirstSetBit()];
    }

    return NO_SQ;
}

int32_t Evaluator::see(Move& m) {
    int32_t score = 0;
    int32_t side = b.getSideToMove() ^ COLOR_MASK;

    b.makeMove(m);

    int32_t attackerSq = getSmallestAttacker(m.getDestination(), side);
    if(attackerSq != NO_SQ) {
        Move newMove(attackerSq, m.getDestination(), MOVE_CAPTURE);
        int32_t capturedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(b.pieceAt(m.getDestination()))];
        score = std::max(score, capturedPieceValue - see(newMove));
    }

    b.undoMove();

    return score;
}

int32_t Evaluator::evaluateMoveSEE(Move m) {
    int32_t moveScore = 0;

    if(m.isPromotion()) {
        if(m.isPromotionQueen())
            moveScore += SIMPLE_PIECE_VALUE[QUEEN] - SIMPLE_PIECE_VALUE[PAWN];
        else if(m.isPromotionRook())
            moveScore += SIMPLE_PIECE_VALUE[ROOK] - SIMPLE_PIECE_VALUE[PAWN];
        else if(m.isPromotionBishop())
            moveScore += SIMPLE_PIECE_VALUE[BISHOP] - SIMPLE_PIECE_VALUE[PAWN];
        else if(m.isPromotionKnight())
            moveScore += SIMPLE_PIECE_VALUE[KNIGHT] - SIMPLE_PIECE_VALUE[PAWN];
    }

    // SEE-Heuristik
    int32_t capturedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(b.pieceAt(m.getDestination()))];
    moveScore += capturedPieceValue - see(m);
    
    return moveScore;
}

int32_t Evaluator::evaluateMoveMVVLVA(Move m) {
    int32_t moveScore = 0;

    if(m.isPromotion()) {
        if(m.isPromotionQueen())
            moveScore += SIMPLE_PIECE_VALUE[QUEEN] - SIMPLE_PIECE_VALUE[PAWN];
        else if(m.isPromotionRook())
            moveScore += SIMPLE_PIECE_VALUE[ROOK] - SIMPLE_PIECE_VALUE[PAWN];
        else if(m.isPromotionBishop())
            moveScore += SIMPLE_PIECE_VALUE[BISHOP] - SIMPLE_PIECE_VALUE[PAWN];
        else if(m.isPromotionKnight())
            moveScore += SIMPLE_PIECE_VALUE[KNIGHT] - SIMPLE_PIECE_VALUE[PAWN];
    }

    if(m.isCapture()) {
        // MVVLVA-Heuristik
        int32_t movedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(b.pieceAt(m.getOrigin()))];
        int32_t capturedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(b.pieceAt(m.getDestination()))];

        moveScore += capturedPieceValue - movedPieceValue;
    }
    
    return moveScore;
}
