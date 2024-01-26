#include "core/engine/evaluation/Evaluator.h"

#include "core/chess/Referee.h"

#include <algorithm>

bool Evaluator::isDraw() {
    // Fifty-move rule
    if(b->getFiftyMoveCounter() >= 100)
        return true;
    
    if(b->repetitionCount() >= 3)
        return true;

    // Material draw
    return Referee::isDrawByMaterial(*b);
}

int32_t Evaluator::getSmallestAttacker(int32_t to, int32_t side) {
    int32_t otherSide = side ^ COLOR_MASK;

    if(!b->getAttackBitboard(side).getBit(to))
        return NO_SQ;
    
    Bitboard pawnAttackers = pawnAttackBitboard(to, otherSide) & b->getPieceBitboard(side | PAWN);
    if(pawnAttackers)
        return pawnAttackers.getFirstSetBit();

    Bitboard knightAttackers = knightAttackBitboard(to) & b->getPieceBitboard(side | KNIGHT);
    if(knightAttackers)
        return knightAttackers.getFirstSetBit();

    Bitboard bishopAttackers = diagonalAttackBitboard(to, b->getOccupiedBitboard() | b->getPieceBitboard(side | KING))
                                                        & b->getPieceBitboard(side | BISHOP);
    if(bishopAttackers)
        return bishopAttackers.getFirstSetBit();

    Bitboard rookAttackers = horizontalAttackBitboard(to, b->getOccupiedBitboard() | b->getPieceBitboard(side | KING))
                                                        & b->getPieceBitboard(side | ROOK);
    if(rookAttackers)
        return rookAttackers.getFirstSetBit();

    Bitboard queenAttackers = (diagonalAttackBitboard(to, b->getOccupiedBitboard() | b->getPieceBitboard(side | KING))
                                | horizontalAttackBitboard(to, b->getOccupiedBitboard() | b->getPieceBitboard(side | KING)))
                                & b->getPieceBitboard(side | QUEEN);
    if(queenAttackers)
        return queenAttackers.getFirstSetBit();
    
    Bitboard kingAttackers = kingAttackBitboard(to) & b->getPieceBitboard(side | KING);
    if(kingAttackers) {
        // Der König darf nur schlagen, wenn die Figur nicht verteidigt wird
        if(b->getAttackBitboard(otherSide).getBit(to))
            return NO_SQ;

        return kingAttackers.getFirstSetBit();
    }

    return NO_SQ;
}

int32_t Evaluator::see(Move& m, uint64_t& nodesSearched) {
    nodesSearched++;

    int32_t score = 0;
    int32_t side = b->getSideToMove() ^ COLOR_MASK;

    b->makeMove(m);

    int32_t attackerSq = getSmallestAttacker(m.getDestination(), side);
    if(attackerSq != NO_SQ) {
        Move newMove(attackerSq, m.getDestination(), MOVE_CAPTURE);
        int32_t capturedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(b->pieceAt(m.getDestination()))];
        score = std::max(score, capturedPieceValue - see(newMove, nodesSearched));
    }

    b->undoMove();

    return score;
}

int16_t Evaluator::evaluateMoveSEE(Move m, uint64_t& nodesSearched) {
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
    int32_t capturedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(b->pieceAt(m.getDestination()))];
    moveScore += capturedPieceValue - see(m, nodesSearched);
    
    return moveScore;
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
        int32_t movedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(b->pieceAt(m.getOrigin()))];
        int32_t capturedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(b->pieceAt(m.getDestination()))];

        moveScore += capturedPieceValue - movedPieceValue;
    }
    
    return moveScore;
}
