#include "core/engine/evaluation/Evaluator.h"

#include "core/chess/Referee.h"

#include <algorithm>

bool Evaluator::isDraw() {
    // 50-Züge-Regel
    if(board.getFiftyMoveCounter() >= 100)
        return true;
    
    // Anstatt der 3-fachen Stellungswiederholung wird die 2-fache Stellungswiederholung,
    // um Zugwiederholungen in der Suche zu vermeiden
    if(board.repetitionCount() >= 2)
        return true;

    // Unzureichendes Material oder Remisstellung
    return isDrawnKPKEndgame() || Referee::isDrawByMaterial(board);
}

int32_t Evaluator::getSmallestAttacker(int32_t to, int32_t side) {
    int32_t otherSide = side ^ COLOR_MASK;

    if(!board.getAttackBitboard(side).getBit(to))
        return NO_SQ;
    
    Bitboard pawnAttackers = pawnAttackBitboard(to, otherSide) & board.getPieceBitboard(side | PAWN);
    if(pawnAttackers)
        return pawnAttackers.getFSB();

    Bitboard knightAttackers = knightAttackBitboard(to) & board.getPieceBitboard(side | KNIGHT);
    if(knightAttackers)
        return knightAttackers.getFSB();

    Bitboard bishopAttackers = diagonalAttackBitboard(to, board.getPieceBitboard() | board.getPieceBitboard(side | KING))
                                                        & board.getPieceBitboard(side | BISHOP);
    if(bishopAttackers)
        return bishopAttackers.getFSB();

    Bitboard rookAttackers = horizontalAttackBitboard(to, board.getPieceBitboard() | board.getPieceBitboard(side | KING))
                                                        & board.getPieceBitboard(side | ROOK);
    if(rookAttackers)
        return rookAttackers.getFSB();

    Bitboard queenAttackers = (diagonalAttackBitboard(to, board.getPieceBitboard() | board.getPieceBitboard(side | KING))
                                | horizontalAttackBitboard(to, board.getPieceBitboard() | board.getPieceBitboard(side | KING)))
                                & board.getPieceBitboard(side | QUEEN);
    if(queenAttackers)
        return queenAttackers.getFSB();
    
    Bitboard kingAttackers = kingAttackBitboard(to) & board.getPieceBitboard(side | KING);
    if(kingAttackers) {
        // Der König darf nur schlagen, wenn die Figur nicht verteidigt wird
        if(board.getAttackBitboard(otherSide).getBit(to))
            return NO_SQ;

        return kingAttackers.getFSB();
    }

    return NO_SQ;
}

int32_t Evaluator::see(Move m) {
    int32_t score = 0;
    int32_t side = board.getSideToMove() ^ COLOR_MASK;

    board.makeMove(m);

    int32_t attackerSq = getSmallestAttacker(m.getDestination(), side);
    if(attackerSq != NO_SQ) {
        Move newMove(attackerSq, m.getDestination(), MOVE_CAPTURE);
        int32_t capturedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(board.pieceAt(m.getDestination()))];
        score = std::max(score, capturedPieceValue - see(newMove));
    }

    board.undoMove();

    return score;
}

bool Evaluator::seeGreaterEqual(Move m, int32_t threshold) {
    int32_t movedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(board.pieceAt(m.getOrigin()))];
    int32_t capturedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(board.pieceAt(m.getDestination()))];

    if(capturedPieceValue - movedPieceValue >= threshold)
        return true;

    board.makeMove(m);

    int32_t attackerSq = getSmallestAttacker(m.getDestination(), board.getSideToMove());
    if(attackerSq != NO_SQ) {
        Move newMove(attackerSq, m.getDestination(), MOVE_CAPTURE);
        bool result = !seeGreaterEqual(newMove, capturedPieceValue - threshold + 1);
        board.undoMove();
        return result;
    }

    board.undoMove();

    return capturedPieceValue >= threshold;
}

bool Evaluator::isDrawnKPKEndgame() {
    Bitboard whitePawns = board.getPieceBitboard(WHITE_PAWN);
    Bitboard blackPawns = board.getPieceBitboard(BLACK_PAWN);

    // Das Endspiel ist nicht KPK
    if(whitePawns.popcount() + blackPawns.popcount() != 1 || board.getPieceBitboard() != (whitePawns | blackPawns))
        return false;

    int32_t whiteKingSquare = board.getKingSquare(WHITE);
    int32_t blackKingSquare = board.getKingSquare(BLACK);

    if(whitePawns)
        return (whitePawns.shiftSouth() | whitePawns.shiftSouthWestEast()).getBit(whiteKingSquare) &&
               (blackPawns.shiftNorth() | blackPawns.shiftNorthWestEast()).getBit(blackKingSquare) &&
                board.getSideToMove() == WHITE && Square::rankOf(blackKingSquare) != RANK_8;
    else
        return (blackPawns.shiftNorth() | blackPawns.shiftNorthWestEast()).getBit(blackKingSquare) &&
               (whitePawns.shiftSouth() | whitePawns.shiftSouthWestEast()).getBit(whiteKingSquare) &&
                board.getSideToMove() == BLACK && Square::rankOf(whiteKingSquare) != RANK_1;
}

int16_t Evaluator::evaluateMoveSEE(Move m) {
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
    int32_t capturedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(board.pieceAt(m.getDestination()))];
    if(m.isEnPassant())
        capturedPieceValue = SIMPLE_PIECE_VALUE[PAWN];

    moveScore += capturedPieceValue - see(m);
    
    return moveScore;
}

bool Evaluator::isSEEGreaterEqual(Move m, int32_t threshold) {
    int32_t movedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(board.pieceAt(m.getOrigin()))];

    if(m.isPromotion()) {
        if(m.isPromotionQueen())
            movedPieceValue -= SIMPLE_PIECE_VALUE[QUEEN] - SIMPLE_PIECE_VALUE[PAWN];
        else if(m.isPromotionRook())
            movedPieceValue -= SIMPLE_PIECE_VALUE[ROOK] - SIMPLE_PIECE_VALUE[PAWN];
        else if(m.isPromotionBishop())
            movedPieceValue -= SIMPLE_PIECE_VALUE[BISHOP] - SIMPLE_PIECE_VALUE[PAWN];
        else if(m.isPromotionKnight())
            movedPieceValue -= SIMPLE_PIECE_VALUE[KNIGHT] - SIMPLE_PIECE_VALUE[PAWN];
    }

    // SEE-Heuristik
    int32_t capturedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(board.pieceAt(m.getDestination()))];
    if(m.isEnPassant())
        capturedPieceValue = SIMPLE_PIECE_VALUE[PAWN];

    if(capturedPieceValue - movedPieceValue >= threshold)
        return true;

    board.makeMove(m);

    int32_t attackerSq = getSmallestAttacker(m.getDestination(), board.getSideToMove());
    if(attackerSq != NO_SQ) {
        Move newMove(attackerSq, m.getDestination(), MOVE_CAPTURE);
        bool result = !seeGreaterEqual(newMove, capturedPieceValue - threshold + 1);
        board.undoMove();
        return result;
    }

    board.undoMove();

    return capturedPieceValue >= threshold;
}

int16_t Evaluator::evaluateMoveMVVLVA(Move m) {
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
        int32_t movedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(board.pieceAt(m.getOrigin()))];
        int32_t capturedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(board.pieceAt(m.getDestination()))];

        moveScore += capturedPieceValue - movedPieceValue;
    }
    
    return moveScore;
}
