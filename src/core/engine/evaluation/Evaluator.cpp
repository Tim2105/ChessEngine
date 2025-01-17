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
    return isDrawnKPKEndgame() || isWrongBishopAndRookPawnEndgame() || Referee::isDrawByMaterial(board);
}

int Evaluator::getSmallestAttacker(int to, int side) {
    int otherSide = side ^ COLOR_MASK;

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

int Evaluator::see(Move m, AtomicU64& nodes) {
    nodes.fetch_add(1);
    int score = 0;
    int side = board.getSideToMove() ^ COLOR_MASK;

    board.makeMove(m);

    int attackerSq = getSmallestAttacker(m.getDestination(), side);
    if(attackerSq != NO_SQ) {
        Move newMove(attackerSq, m.getDestination(), MOVE_CAPTURE);
        int capturedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(board.pieceAt(m.getDestination()))];
        score = std::max(score, capturedPieceValue - see(newMove, nodes));
    }

    board.undoMove();

    return score;
}

bool Evaluator::seeGreaterEqual(Move m, int threshold, AtomicU64& nodes) {
    nodes.fetch_add(1);
    int movedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(board.pieceAt(m.getOrigin()))];
    int capturedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(board.pieceAt(m.getDestination()))];

    if(capturedPieceValue - movedPieceValue >= threshold)
        return true;

    board.makeMove(m);

    int attackerSq = getSmallestAttacker(m.getDestination(), board.getSideToMove());
    if(attackerSq != NO_SQ) {
        Move newMove(attackerSq, m.getDestination(), MOVE_CAPTURE);
        bool result = !seeGreaterEqual(newMove, capturedPieceValue - threshold + 1, nodes);
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

    Bitboard whiteKing = board.getPieceBitboard(WHITE_KING);
    Bitboard blackKing = board.getPieceBitboard(BLACK_KING);
    int sideToMove = board.getSideToMove();

    if(whitePawns) {
        if(whitePawns.shiftNorth() & blackKing &&
         ((whitePawns.shiftSouth() & whiteKing && sideToMove == BLACK) ||
          (whitePawns.shiftSouthWestEast() & whiteKing && sideToMove == WHITE)))
            return true;

        // Opposition
        if(sideToMove == WHITE) {
            Bitboard whiteKingNextToPawn = (whitePawns.shiftWest() | whitePawns.shiftEast()) & whiteKing;
            if(whiteKingNextToPawn && whiteKingNextToPawn.shiftNorth(2) & blackKing)
                return true;
        } else {
            Bitboard whiteKingDiagBehindPawn = whitePawns.shiftSouthWestEast() & whiteKing;
            if(whiteKingDiagBehindPawn && whiteKingDiagBehindPawn.shiftNorth(2) & blackKing)
                return true;
        }
    } else {
        if(blackPawns.shiftSouth() & whiteKing &&
         ((blackPawns.shiftNorth() & blackKing && sideToMove == WHITE) ||
          (blackPawns.shiftNorthWestEast() & blackKing && sideToMove == BLACK)))
            return true;

        // Opposition
        if(sideToMove == BLACK) {
            Bitboard blackKingNextToPawn = (blackPawns.shiftWest() | blackPawns.shiftEast()) & blackKing;
            if(blackKingNextToPawn && blackKingNextToPawn.shiftSouth(2) & whiteKing)
                return true;
        } else {
            Bitboard blackKingDiagBehindPawn = blackPawns.shiftNorthWestEast() & blackKing;
            if(blackKingDiagBehindPawn && blackKingDiagBehindPawn.shiftSouth(2) & whiteKing)
                return true;
        }
    }

    return false;
}

bool Evaluator::isWrongBishopAndRookPawnEndgame() {
    Bitboard whiteBishops = board.getPieceBitboard(WHITE_BISHOP);
    Bitboard blackBishops = board.getPieceBitboard(BLACK_BISHOP);
    Bitboard whitePawns = board.getPieceBitboard(WHITE_PAWN);
    Bitboard blackPawns = board.getPieceBitboard(BLACK_PAWN);

    int promotionSq = NO_SQ;
    int strongerKingSq, weakerKingSq, pawnSq;
    int strongerSide;

    if(whiteBishops) {
        if(board.getPieceBitboard() != (whiteBishops | whitePawns) ||
           whiteBishops.popcount() != 1 || whitePawns.popcount() != 1)
            return false;

        strongerKingSq = board.getPieceBitboard(WHITE_KING).getFSB();
        weakerKingSq = board.getPieceBitboard(BLACK_KING).getFSB();
        pawnSq = whitePawns.getFSB();
        strongerSide = WHITE;

        if(whiteBishops & lightSquares) {
            if(Square::fileOf(whitePawns.getFSB()) == FILE_H)
                promotionSq = H8;
        } else {
            if(Square::fileOf(whitePawns.getFSB()) == FILE_A)
                promotionSq = A8;
        }
    } else {
        if(board.getPieceBitboard() != (blackBishops | blackPawns) ||
           blackBishops.popcount() != 1 || blackPawns.popcount() != 1)
            return false;

        strongerKingSq = board.getPieceBitboard(BLACK_KING).getFSB();
        weakerKingSq = board.getPieceBitboard(WHITE_KING).getFSB();
        pawnSq = blackPawns.getFSB();
        strongerSide = BLACK;

        if(blackBishops & lightSquares) {
            if(Square::fileOf(blackPawns.getFSB()) == FILE_A)
                promotionSq = A1;
        } else {
            if(Square::fileOf(blackPawns.getFSB()) == FILE_H)
                promotionSq = H1;
        }
    }

    if(promotionSq == NO_SQ)
        return false;

    int distStronger = std::max(std::abs(Square::fileOf(strongerKingSq) - Square::fileOf(promotionSq)),
                                std::abs(Square::rankOf(strongerKingSq) - Square::rankOf(promotionSq)));

    int distWeaker = std::max(std::abs(Square::fileOf(weakerKingSq) - Square::fileOf(promotionSq)),
                              std::abs(Square::rankOf(weakerKingSq) - Square::rankOf(promotionSq))) +
                              (board.getSideToMove() == strongerSide); // Betrachtet Tempo

    int distPawn = std::min(std::abs(Square::rankOf(pawnSq) - Square::rankOf(promotionSq)), 5); // Betrachtet Doppelzug

    return distWeaker < distStronger && distWeaker < distPawn;
}

int Evaluator::evaluateMoveSEE(Move m, AtomicU64& nodes) {
    int moveScore = 0;

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
    int capturedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(board.pieceAt(m.getDestination()))];
    if(m.isEnPassant())
        capturedPieceValue = SIMPLE_PIECE_VALUE[PAWN];

    moveScore += capturedPieceValue - see(m, nodes);
    
    return moveScore;
}

bool Evaluator::isSEEGreaterEqual(Move m, int threshold, AtomicU64& nodes) {
    int movedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(board.pieceAt(m.getOrigin()))];

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
    int capturedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(board.pieceAt(m.getDestination()))];
    if(m.isEnPassant())
        capturedPieceValue = SIMPLE_PIECE_VALUE[PAWN];

    if(capturedPieceValue - movedPieceValue >= threshold)
        return true;

    board.makeMove(m);

    int attackerSq = getSmallestAttacker(m.getDestination(), board.getSideToMove());
    if(attackerSq != NO_SQ) {
        Move newMove(attackerSq, m.getDestination(), MOVE_CAPTURE);
        bool result = !seeGreaterEqual(newMove, capturedPieceValue - threshold + 1, nodes);
        board.undoMove();
        return result;
    }

    board.undoMove();

    return capturedPieceValue >= threshold;
}

int Evaluator::evaluateMoveMVVLVA(Move m) {
    int moveScore = 0;

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
        int movedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(board.pieceAt(m.getOrigin()))];
        int capturedPieceValue = SIMPLE_PIECE_VALUE[TYPEOF(board.pieceAt(m.getDestination()))];

        moveScore += capturedPieceValue - movedPieceValue;
    }
    
    return moveScore;
}
