#include "core/engine/evaluation/HandcraftedEvaluator.h"

#include <cmath>

void HandcraftedEvaluator::updateBeforeMove(Move m) {
    evaluationHistory.push_back(evaluationVars);

    Score psqtDelta{0, 0};
    int32_t pieceValueDelta = 0;

    int32_t side = board.getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;
    int32_t piece = TYPEOF(board.pieceAt(m.getOrigin()));
    int32_t originPSQT = m.getOrigin();
    int32_t destinationPSQT = m.getDestination();

    // Für schwarze Figuren muss der Rang gespiegelt werden,
    // um die Positionstabelle zu indizieren
    if(side == BLACK) {
        originPSQT = Square::flipY(originPSQT);
        destinationPSQT = Square::flipY(destinationPSQT);
    }

    psqtDelta.mg += hceParams.getMGPSQT(piece,destinationPSQT) - hceParams.getMGPSQT(piece, originPSQT);
    psqtDelta.eg += hceParams.getEGPSQT(piece, destinationPSQT) - hceParams.getEGPSQT(piece, originPSQT);

    // Spezialfall: Schlagzug
    if(m.isCapture()) {
        int32_t capturedPiece = board.pieceAt(m.getDestination());
        int32_t capturedPieceSquare = destinationPSQT;

        // Spezialfall: En-Passant
        if(m.isEnPassant()) {
            capturedPiece = otherSide | PAWN;
            capturedPieceSquare += SOUTH;
        }

        int32_t capturedPieceType = TYPEOF(capturedPiece);

        // Für geschlagene Figuren muss der Rang gespiegelt werden,
        // um die Positionstabelle zu indizieren
        capturedPieceSquare = Square::flipY(capturedPieceSquare);

        // Aktualisiere die linearen und quadratischen Terme
        pieceValueDelta += hceParams.getLinearPieceValue(capturedPieceType);

        int32_t numCapPiece = board.getPieceBitboard(capturedPiece).popcount();
        pieceValueDelta += hceParams.getQuadraticPieceValue(capturedPieceType) * (numCapPiece * numCapPiece - (numCapPiece - 1) * (numCapPiece - 1));

        // Aktualisiere die gemischten Terme
        for(int32_t piece = PAWN; piece <= QUEEN; piece++) {
            if(piece == capturedPieceType)
                continue;

            int16_t crossedPieceValue = hceParams.getCrossedPieceValue(piece, capturedPieceType);
            pieceValueDelta += crossedPieceValue * board.getPieceBitboard(otherSide | piece).popcount();
        }

        psqtDelta.mg += hceParams.getMGPSQT(capturedPieceType, capturedPieceSquare);
        psqtDelta.eg += hceParams.getEGPSQT(capturedPieceType, capturedPieceSquare);

        // Aktualisiere die Phasengewichte
        evaluationVars.phaseWeight += PIECE_WEIGHT[capturedPieceType];
    }

    // Spezialfall: Rochade
    if(m.isCastle()) {
        if(m.isKingsideCastle()) {
            // Königsseite
            psqtDelta.mg += hceParams.getMGPSQT(ROOK, F1) - hceParams.getMGPSQT(ROOK, H1);
            psqtDelta.eg += hceParams.getEGPSQT(ROOK, F1) - hceParams.getEGPSQT(ROOK, H1);
        } else {
            // Damenseite
            psqtDelta.mg += hceParams.getMGPSQT(ROOK, D1) - hceParams.getMGPSQT(ROOK, A1);
            psqtDelta.eg += hceParams.getEGPSQT(ROOK, D1) - hceParams.getEGPSQT(ROOK, A1);
        }
    }

    // Spezialfall: Promotion
    if(m.isPromotion()) {
        int32_t promotedPiece = KNIGHT;

        if(m.isPromotionQueen())
            promotedPiece = QUEEN;
        else if(m.isPromotionRook())
            promotedPiece = ROOK;
        else if(m.isPromotionBishop())
            promotedPiece = BISHOP;

        // Aktualisiere die linearen Terme
        pieceValueDelta += hceParams.getLinearPieceValue(promotedPiece) - hceParams.getLinearPieceValue(PAWN);

        // Aktualisiere die quadratischen Terme
        int32_t numPromotedPiece = board.getPieceBitboard(side | promotedPiece).popcount();
        int32_t numPawns = board.getPieceBitboard(side | PAWN).popcount();
        int16_t quadraticPromotedPieceValue = hceParams.getQuadraticPieceValue(promotedPiece);
        int16_t quadraticPawnValue = hceParams.getQuadraticPieceValue(PAWN);
        pieceValueDelta += quadraticPromotedPieceValue * ((numPromotedPiece + 1) * (numPromotedPiece + 1) - numPromotedPiece * numPromotedPiece) -
                           quadraticPawnValue * (numPawns * numPawns - (numPawns - 1) * (numPawns - 1));

        // Aktualisiere die gemischten Terme für alle Paare von Figuren
        // mit Ausnahme des Bauerns und der aufgewerteten Figur
        for(int32_t piece = KNIGHT; piece <= QUEEN; piece++) {
            int32_t numPieces = board.getPieceBitboard(side | piece).popcount();
            if(piece != promotedPiece) {
                int16_t crossedPieceValue = hceParams.getCrossedPieceValue(piece, promotedPiece);
                pieceValueDelta += crossedPieceValue * numPieces;

                int16_t crossedPawnValue = hceParams.getCrossedPieceValue(piece, PAWN);
                pieceValueDelta -= crossedPawnValue * numPieces;
            }
        }

        // Aktualisiere die gemischten Terme für das Paar Bauer und aufgewertete Figur
        int16_t crossedPawnValue = hceParams.getCrossedPieceValue(PAWN, promotedPiece);
        pieceValueDelta -= crossedPawnValue * numPawns * numPromotedPiece;
        pieceValueDelta += crossedPawnValue * (numPawns - 1) * (numPromotedPiece + 1);

        psqtDelta.mg += hceParams.getMGPSQT(promotedPiece, destinationPSQT) - hceParams.getMGPSQT(PAWN, destinationPSQT);
        psqtDelta.eg += hceParams.getEGPSQT(promotedPiece, destinationPSQT) - hceParams.getEGPSQT(PAWN, destinationPSQT);

        // Aktualisiere die Phasengewichte
        evaluationVars.phaseWeight -= PIECE_WEIGHT[promotedPiece] - PIECE_WEIGHT[PAWN];
    }

    evaluationVars.materialScore += (psqtDelta + Score{pieceValueDelta, pieceValueDelta}) * (side == WHITE ? 1 : -1);

    evaluationVars.phase = evaluationVars.phaseWeight / (double)TOTAL_WEIGHT;
    evaluationVars.phase = evaluationVars.phase * (MAX_PHASE - MIN_PHASE) + MIN_PHASE; // phase in [MIN_PHASE, MAX_PHASE]
    evaluationVars.phase = std::clamp(evaluationVars.phase, 0.0, 1.0); // phase auf [0, 1] begrenzen
}

void HandcraftedEvaluator::updateAfterMove() {
    Move m = board.getLastMove();

    int32_t movedPiece = TYPEOF(board.pieceAt(m.getDestination()));
    int32_t capturedPiece = TYPEOF(board.getLastMoveHistoryEntry().capturedPiece);

    // Aktualisiere die Bauernbewertung,
    // wenn ein Bauer bewegt, geschlagen oder aufgewertet wurde
    if(movedPiece == PAWN || capturedPiece == PAWN || m.isPromotion())
        calculatePawnScore();
}

void HandcraftedEvaluator::updateBeforeUndo() {
    evaluationVars = evaluationHistory.back();
    evaluationHistory.pop_back();
}

void HandcraftedEvaluator::calculateMaterialScore() {
    Score psqtScore{0, 0};
    int32_t pieceScore = 0;

    // Material
    for(int32_t piece = PAWN; piece <= QUEEN; piece++) {
        int32_t numWhitePieces = board.getPieceBitboard(WHITE | piece).popcount();
        int32_t numBlackPieces = board.getPieceBitboard(BLACK | piece).popcount();

        // lineare Terme
        pieceScore += hceParams.getLinearPieceValue(piece) * (numWhitePieces - numBlackPieces);

        // quadratische Terme
        pieceScore += hceParams.getQuadraticPieceValue(piece) * (numWhitePieces * numWhitePieces - numBlackPieces * numBlackPieces);

        // gemischte Terme
        for(int32_t otherPiece = PAWN; otherPiece < piece; otherPiece++) {
            int16_t crossedPieceValue = hceParams.getCrossedPieceValue(piece, otherPiece);
            pieceScore += crossedPieceValue * (numWhitePieces * board.getPieceBitboard(WHITE | otherPiece).popcount() -
                                               numBlackPieces * board.getPieceBitboard(BLACK | otherPiece).popcount());
        }
    }

    // Positionstabellen
    for(int32_t piece = PAWN; piece <= KING; piece++) {
        Bitboard pieceBB = board.getPieceBitboard(WHITE | piece);
        while(pieceBB) {
            int32_t square = pieceBB.popFSB();
            psqtScore.mg += hceParams.getMGPSQT(piece, square);
            psqtScore.eg += hceParams.getEGPSQT(piece, square);
        }

        pieceBB = board.getPieceBitboard(BLACK | piece);
        while(pieceBB) {
            int32_t square = pieceBB.popFSB();

            // Für schwarze Figuren muss der Rang gespiegelt werden
            square = Square::flipY(square);

            psqtScore.mg -= hceParams.getMGPSQT(piece, square);
            psqtScore.eg -= hceParams.getEGPSQT(piece, square);
        }
    }

    evaluationVars.materialScore = psqtScore + Score{pieceScore, pieceScore};
}

void HandcraftedEvaluator::calculatePawnScore() {
    Score score{0, 0};

    Bitboard whitePawns = board.getPieceBitboard(WHITE_PAWN);
    Bitboard blackPawns = board.getPieceBitboard(BLACK_PAWN);

    Bitboard whitePawnAttacks = board.getPieceAttackBitboard(WHITE_PAWN);
    Bitboard blackPawnAttacks = board.getPieceAttackBitboard(BLACK_PAWN);

    Bitboard connectedWhitePawns = 0, connectedBlackPawns = 0;
    Bitboard doubleWhitePawns = 0, doubleBlackPawns = 0;
    Bitboard isolatedWhitePawns = 0, isolatedBlackPawns = 0;
    Bitboard backwardWhitePawns = 0, backwardBlackPawns = 0;
    Bitboard whitePassedPawns = 0, blackPassedPawns = 0;
    Bitboard whiteStrongSqaures = 0, blackStrongSqaures = 0;

    Bitboard tmp = whitePawns;
    while(tmp) {
        int32_t sq = tmp.popFSB();

        // Verbundene Bauern
        connectedWhitePawns |= whitePawns & connectedPawnMask[sq];

        // Doppelbauern
        doubleWhitePawns |= whitePawns & fileFacingEnemy[WHITE / COLOR_MASK][sq];

        // Isolierte Bauern
        int32_t file = Square::fileOf(sq);
        if(!(whitePawns & neighboringFiles[file]))
            isolatedWhitePawns.setBit(sq);
        else {
            // Rückständige Bauern
            int32_t rank = Square::rankOf(sq);
            if(!(whitePawns & backwardPawnMask[WHITE / COLOR_MASK][sq]) &&
            (blackPawns | blackPawnAttacks).getBit(Square::fromFileRank(file, rank + 1)) &&
            !whitePawnAttacks.getBit(Square::fromFileRank(file, rank + 1)))
                backwardWhitePawns.setBit(sq);
        }

        // Freibauern
        if(!(sentryMask[WHITE / COLOR_MASK][sq] & blackPawns) &&
           !doubleWhitePawns.getBit(sq))
            whitePassedPawns.setBit(sq);
    }

    tmp = whitePawnAttacks & ~blackPawnAttacks;
    while(tmp) {
        int32_t sq = tmp.popFSB();

        // Starke Felder
        if(!(blackPawns & strongSquareMask[WHITE / COLOR_MASK][sq]))
            whiteStrongSqaures.setBit(sq);
    }

    tmp = blackPawns;
    while(tmp) {
        int32_t sq = tmp.popFSB();

        // Verbundene Bauern
        connectedBlackPawns |= blackPawns & connectedPawnMask[sq];

        // Doppelbauern
        doubleBlackPawns |= blackPawns & fileFacingEnemy[BLACK / COLOR_MASK][sq];

        // Isolierte Bauern
        int32_t file = Square::fileOf(sq);
        if(!(blackPawns & neighboringFiles[file]))
            isolatedBlackPawns.setBit(sq);
        else {
            // Rückständige Bauern
            int32_t rank = Square::rankOf(sq);
            if(!(blackPawns & backwardPawnMask[BLACK / COLOR_MASK][sq]) &&
            (whitePawns | whitePawnAttacks).getBit(Square::fromFileRank(file, rank - 1)) &&
            !blackPawnAttacks.getBit(Square::fromFileRank(file, rank - 1)))
                backwardBlackPawns.setBit(sq);
        }

        // Freibauern
        if(!(sentryMask[BLACK / COLOR_MASK][sq] & whitePawns) &&
           !doubleBlackPawns.getBit(sq))
            blackPassedPawns.setBit(sq);
    }

    tmp = blackPawnAttacks & ~whitePawnAttacks;
    while(tmp) {
        int32_t sq = tmp.popFSB();

        // Starke Felder
        if(!(whitePawns & strongSquareMask[BLACK / COLOR_MASK][sq]))
            blackStrongSqaures.setBit(sq);
    }

    evaluationVars.whiteBackwardPawns = backwardWhitePawns;
    evaluationVars.blackBackwardPawns = backwardBlackPawns;
    evaluationVars.whitePassedPawns = whitePassedPawns;
    evaluationVars.blackPassedPawns = blackPassedPawns;
    evaluationVars.whiteStrongSquares = whiteStrongSqaures;
    evaluationVars.blackStrongSquares = blackStrongSqaures;

    // Verbundene Bauern
    score.mg += hceParams.getMGConnectedPawnBonus() * (connectedWhitePawns.popcount() - connectedBlackPawns.popcount());
    score.eg += hceParams.getEGConnectedPawnBonus() * (connectedWhitePawns.popcount() - connectedBlackPawns.popcount());

    // Doppelbauern
    score.mg += hceParams.getMGDoubledPawnPenalty() * (doubleWhitePawns.popcount() - doubleBlackPawns.popcount());
    score.eg += hceParams.getEGDoubledPawnPenalty() * (doubleWhitePawns.popcount() - doubleBlackPawns.popcount());

    // Isolierte Bauern
    score.mg += hceParams.getMGIsolatedPawnPenalty() * (isolatedWhitePawns.popcount() - isolatedBlackPawns.popcount());
    score.eg += hceParams.getEGIsolatedPawnPenalty() * (isolatedWhitePawns.popcount() - isolatedBlackPawns.popcount());

    // Rückständige Bauern
    score.mg += hceParams.getMGBackwardPawnPenalty() * (backwardWhitePawns.popcount() - backwardBlackPawns.popcount());
    score.eg += hceParams.getEGBackwardPawnPenalty() * (backwardWhitePawns.popcount() - backwardBlackPawns.popcount());

    // Freibauern
    while(whitePassedPawns) {
        int32_t rank = Square::rankOf(whitePassedPawns.popFSB());
        score.mg += hceParams.getMGPassedPawnBonus(rank);
        score.eg += hceParams.getEGPassedPawnBonus(rank);
    }

    while(blackPassedPawns) {
        int32_t rank = Square::rankOf(Square::flipY(blackPassedPawns.popFSB()));
        score.mg -= hceParams.getMGPassedPawnBonus(rank);
        score.eg -= hceParams.getEGPassedPawnBonus(rank);
    }

    // Starke Felder
    while(whiteStrongSqaures) {
        int32_t sq = whiteStrongSqaures.popFSB();
        int32_t rank = Square::rankOf(sq);
        score.mg += hceParams.getMGStrongSquareBonus(rank);
    }

    while(blackStrongSqaures) {
        int32_t sq = Square::flipY(blackStrongSqaures.popFSB());
        int32_t rank = Square::rankOf(sq);
        score.mg -= hceParams.getMGStrongSquareBonus(rank);
    }

    evaluationVars.pawnScore = score;
}

void HandcraftedEvaluator::calculateGamePhase() {
    evaluationVars.phaseWeight = TOTAL_WEIGHT;

    evaluationVars.phaseWeight -= board.getPieceBitboard(WHITE_PAWN).popcount() * PAWN_WEIGHT;
    evaluationVars.phaseWeight -= board.getPieceBitboard(BLACK_PAWN).popcount() * PAWN_WEIGHT;
    evaluationVars.phaseWeight -= board.getPieceBitboard(WHITE_KNIGHT).popcount() * KNIGHT_WEIGHT;
    evaluationVars.phaseWeight -= board.getPieceBitboard(BLACK_KNIGHT).popcount() * KNIGHT_WEIGHT;
    evaluationVars.phaseWeight -= board.getPieceBitboard(WHITE_BISHOP).popcount() * BISHOP_WEIGHT;
    evaluationVars.phaseWeight -= board.getPieceBitboard(BLACK_BISHOP).popcount() * BISHOP_WEIGHT;
    evaluationVars.phaseWeight -= board.getPieceBitboard(WHITE_ROOK).popcount() * ROOK_WEIGHT;
    evaluationVars.phaseWeight -= board.getPieceBitboard(BLACK_ROOK).popcount() * ROOK_WEIGHT;
    evaluationVars.phaseWeight -= board.getPieceBitboard(WHITE_QUEEN).popcount() * QUEEN_WEIGHT;
    evaluationVars.phaseWeight -= board.getPieceBitboard(BLACK_QUEEN).popcount() * QUEEN_WEIGHT;

    evaluationVars.phase = evaluationVars.phaseWeight / (double)TOTAL_WEIGHT;
    evaluationVars.phase = evaluationVars.phase * (MAX_PHASE - MIN_PHASE) + MIN_PHASE; // phase in [MIN_PHASE, MAX_PHASE]
    evaluationVars.phase = std::clamp(evaluationVars.phase, 0.0, 1.0); // phase auf [0, 1] begrenzen
}

Score HandcraftedEvaluator::calculateKingSafetyScore() {
    int32_t kingSafety = evaluateKingAttackZone();
    int32_t kingMGSafety = evaluateOpenFiles() + evaluatePawnSafety();

    return {kingSafety + kingMGSafety, kingSafety};
}

int32_t HandcraftedEvaluator::evaluateKingAttackZone() {
    int32_t score = 0;

    int32_t whiteKingSquare = board.getKingSquare(WHITE);
    int32_t blackKingSquare = board.getKingSquare(BLACK);

    // Bestimme die Anzahl der Angreifer pro Figurentyp auf die Felder um den weißen König
    Bitboard kingZone = kingAttackZone[whiteKingSquare];
    int32_t numBlackAttackers = 0;
    int32_t blackAttackersWeight = 0;

    Bitboard blackKnights = board.getPieceBitboard(BLACK_KNIGHT);
    Bitboard blackBishops = board.getPieceBitboard(BLACK_BISHOP);

    Bitboard blackMinorPieces = blackKnights | blackBishops;

    while(blackKnights) {
        int32_t sq = blackKnights.popFSB();
        Bitboard attacks = knightAttackBitboard(sq) & kingZone;
        if(attacks) {
            numBlackAttackers++;
            blackAttackersWeight += attacks.popcount() * hceParams.getKnightAttackBonus();
        }
    }

    while(blackBishops) {
        int32_t sq = blackBishops.popFSB();
        Bitboard attacks = diagonalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(BLACK_KING)) & kingZone;
        if(attacks) {
            numBlackAttackers++;
            blackAttackersWeight += attacks.popcount() * hceParams.getBishopAttackBonus();
        }
    }

    Bitboard blackRooks = board.getPieceBitboard(BLACK_ROOK);
    while(blackRooks) {
        int32_t sq = blackRooks.popFSB();
        Bitboard attacks = horizontalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(BLACK_KING)) & kingZone;
        if(attacks) {
            numBlackAttackers++;
            blackAttackersWeight += attacks.popcount() * hceParams.getRookAttackBonus();
        }
    }

    Bitboard blackQueens = board.getPieceBitboard(BLACK_QUEEN);
    while(blackQueens) {
        int32_t sq = blackQueens.popFSB();
        Bitboard attacks = (diagonalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(BLACK_KING)) |
                            horizontalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(BLACK_KING))) & kingZone;

        if(attacks) {
            numBlackAttackers++;
            blackAttackersWeight += attacks.popcount() * hceParams.getQueenAttackBonus();
        }
    }

    numBlackAttackers = std::min(numBlackAttackers, 5);

    // Bestimme die Anzahl der Angreifer pro Figurentyp auf die Felder um den schwarzen König
    kingZone = kingAttackZone[blackKingSquare];
    int32_t numWhiteAttackers = 0;
    int32_t whiteAttackersWeight = 0;

    Bitboard whiteKnights = board.getPieceBitboard(WHITE_KNIGHT);
    Bitboard whiteBishops = board.getPieceBitboard(WHITE_BISHOP);

    Bitboard whiteMinorPieces = whiteKnights | whiteBishops;

    while(whiteKnights) {
        int32_t sq = whiteKnights.popFSB();
        Bitboard attacks = knightAttackBitboard(sq) & kingZone;
        if(attacks) {
            numWhiteAttackers++;
            whiteAttackersWeight += attacks.popcount() * hceParams.getKnightAttackBonus();
        }
    }

    while(whiteBishops) {
        int32_t sq = whiteBishops.popFSB();
        Bitboard attacks = diagonalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(WHITE_KING)) & kingZone;
        if(attacks) {
            numWhiteAttackers++;
            whiteAttackersWeight += attacks.popcount() * hceParams.getBishopAttackBonus();
        }
    }

    Bitboard whiteRooks = board.getPieceBitboard(WHITE_ROOK);
    while(whiteRooks) {
        int32_t sq = whiteRooks.popFSB();
        Bitboard attacks = horizontalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(WHITE_KING)) & kingZone;
        if(attacks) {
            numWhiteAttackers++;
            whiteAttackersWeight += attacks.popcount() * hceParams.getRookAttackBonus();
        }
    }

    Bitboard whiteQueens = board.getPieceBitboard(WHITE_QUEEN);
    while(whiteQueens) {
        int32_t sq = whiteQueens.popFSB();
        Bitboard attacks = (diagonalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(WHITE_KING)) |
                            horizontalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(WHITE_KING))) & kingZone;
                        
        if(attacks) {
            numWhiteAttackers++;
            whiteAttackersWeight += attacks.popcount() * hceParams.getQueenAttackBonus();
        }
    }

    numWhiteAttackers = std::min(numWhiteAttackers, 5);

    // Berechne die Bewertung
    score = whiteAttackersWeight * hceParams.getNumAttackerWeight(numWhiteAttackers) / 100 -
            blackAttackersWeight * hceParams.getNumAttackerWeight(numBlackAttackers) / 100;

    return score;
}

int32_t HandcraftedEvaluator::evaluateOpenFiles() {
    int32_t whiteKingSquare = board.getKingSquare(WHITE);
    int32_t blackKingSquare = board.getKingSquare(BLACK);

    int32_t whiteKingFile = Square::fileOf(whiteKingSquare);
    int32_t blackKingFile = Square::fileOf(blackKingSquare);

    Bitboard whitePawns = board.getPieceBitboard(WHITE_PAWN);
    Bitboard blackPawns = board.getPieceBitboard(BLACK_PAWN);

    int32_t whiteOpenFiles = 0;
    int32_t blackOpenFiles = 0;

    for(int32_t file : nearbyFiles[whiteKingFile])
        if(!(whitePawns & fileMasks[file]))
            whiteOpenFiles++;

    for(int32_t file : nearbyFiles[blackKingFile])
        if(!(blackPawns & fileMasks[file]))
            blackOpenFiles++;

    return hceParams.getMGKingOpenFilePenalty(whiteOpenFiles) - hceParams.getMGKingOpenFilePenalty(blackOpenFiles);
}

int32_t HandcraftedEvaluator::evaluatePawnSafety() {
    int32_t score = 0;

    int32_t whiteKingSquare = board.getKingSquare(WHITE);
    int32_t blackKingSquare = board.getKingSquare(BLACK);

    Bitboard whitePawns = board.getPieceBitboard(WHITE_PAWN);
    Bitboard blackPawns = board.getPieceBitboard(BLACK_PAWN);

    Bitboard whitePawnStorm = pawnStormMask[WHITE / COLOR_MASK][whiteKingSquare] & blackPawns;
    Bitboard blackPawnStorm = pawnStormMask[BLACK / COLOR_MASK][blackKingSquare] & whitePawns;

    while(whitePawnStorm) {
        int32_t rank = Square::rankOf(whitePawnStorm.popFSB());
        score += hceParams.getMGPawnStormBonus(rank);
    }

    while(blackPawnStorm) {
        int32_t rank = Square::rankOf(Square::flipY(blackPawnStorm.popFSB()));
        score -= hceParams.getMGPawnStormBonus(rank);
    }

    Bitboard whitePawnShield = pawnShieldMask[WHITE / COLOR_MASK][whiteKingSquare] & whitePawns;
    Bitboard blackPawnShield = pawnShieldMask[BLACK / COLOR_MASK][blackKingSquare] & blackPawns;

    int32_t whitePawnShieldSize = whitePawnShield.popcount();
    int32_t blackPawnShieldSize = blackPawnShield.popcount();

    whitePawnShieldSize = std::min(whitePawnShieldSize, 3);
    blackPawnShieldSize = std::min(blackPawnShieldSize, 3);

    score += hceParams.getMGPawnShieldSizeBonus(whitePawnShieldSize) - hceParams.getMGPawnShieldSizeBonus(blackPawnShieldSize);

    return score;
}

Score HandcraftedEvaluator::calculatePieceScore() {
    return evaluatePieceMobility() + evaluateSafeCenterSpace() + evaluateMinorPiecesOnStrongSquares() +
           evaluateRooksOnOpenFiles() + evaluateRooksBehindPassedPawns() + evaluateKingPawnProximity();
}

Score HandcraftedEvaluator::evaluatePieceMobility() {
    int32_t whiteKnightMobility = std::sqrt(board.getPieceAttackBitboard(WHITE_KNIGHT).popcount());
    int32_t blackKnightMobility = std::sqrt(board.getPieceAttackBitboard(BLACK_KNIGHT).popcount());
    int32_t whiteBishopMobility = std::sqrt(board.getPieceAttackBitboard(WHITE_BISHOP).popcount());
    int32_t blackBishopMobility = std::sqrt(board.getPieceAttackBitboard(BLACK_BISHOP).popcount());
    int32_t whiteRookMobility = std::sqrt(board.getPieceAttackBitboard(WHITE_ROOK).popcount());
    int32_t blackRookMobility = std::sqrt(board.getPieceAttackBitboard(BLACK_ROOK).popcount());
    int32_t whiteQueenMobility = std::sqrt(board.getPieceAttackBitboard(WHITE_QUEEN).popcount());
    int32_t blackQueenMobility = std::sqrt(board.getPieceAttackBitboard(BLACK_QUEEN).popcount());

    int32_t mgMobilityScore = whiteKnightMobility * hceParams.getMGPieceMobilityBonus(KNIGHT) +
                              whiteBishopMobility * hceParams.getMGPieceMobilityBonus(BISHOP) +
                              whiteRookMobility * hceParams.getMGPieceMobilityBonus(ROOK) +
                              whiteQueenMobility * hceParams.getMGPieceMobilityBonus(QUEEN) -
                              blackKnightMobility * hceParams.getMGPieceMobilityBonus(KNIGHT) -
                              blackBishopMobility * hceParams.getMGPieceMobilityBonus(BISHOP) -
                              blackRookMobility * hceParams.getMGPieceMobilityBonus(ROOK) -
                              blackQueenMobility * hceParams.getMGPieceMobilityBonus(QUEEN);

    int32_t egMobilityScore = whiteKnightMobility * hceParams.getEGPieceMobilityBonus(KNIGHT) +
                              whiteBishopMobility * hceParams.getEGPieceMobilityBonus(BISHOP) +
                              whiteRookMobility * hceParams.getEGPieceMobilityBonus(ROOK) +
                              whiteQueenMobility * hceParams.getEGPieceMobilityBonus(QUEEN) -
                              blackKnightMobility * hceParams.getEGPieceMobilityBonus(KNIGHT) -
                              blackBishopMobility * hceParams.getEGPieceMobilityBonus(BISHOP) -
                              blackRookMobility * hceParams.getEGPieceMobilityBonus(ROOK) -
                              blackQueenMobility * hceParams.getEGPieceMobilityBonus(QUEEN);

    return {mgMobilityScore, egMobilityScore};
}

Score HandcraftedEvaluator::evaluateSafeCenterSpace() {
    Bitboard whiteSafeCenterSquares = extendedCenter & ~board.getPieceAttackBitboard(BLACK_PAWN);
    Bitboard blackSafeCenterSquares = extendedCenter & ~board.getPieceAttackBitboard(WHITE_PAWN);

    Bitboard whitePawns = board.getPieceBitboard(WHITE_PAWN);
    Bitboard blackPawns = board.getPieceBitboard(BLACK_PAWN);

    int32_t whiteNumSafeCenterSquares = whiteSafeCenterSquares.popcount();
    int32_t blackNumSafeCenterSquares = blackSafeCenterSquares.popcount();

    // Zähle Felder hinter eigenen Bauern doppelt
    Bitboard whiteCenterPawns = whitePawns & extendedCenter;
    while(whiteCenterPawns) {
        int32_t sq = whiteCenterPawns.popFSB();
        int32_t rank = Square::rankOf(sq);
        whiteNumSafeCenterSquares += rank - RANK_2;
    }

    Bitboard blackCenterPawns = blackPawns & extendedCenter;
    while(blackCenterPawns) {
        int32_t sq = blackCenterPawns.popFSB();
        int32_t rank = Square::rankOf(sq);
        blackNumSafeCenterSquares += RANK_7 - rank;
    }

    return {(int16_t)(std::sqrt(whiteNumSafeCenterSquares) - std::sqrt(blackNumSafeCenterSquares)) * hceParams.getMGSpaceBonus(), 0};
}

Score HandcraftedEvaluator::evaluateMinorPiecesOnStrongSquares() {
    Bitboard whiteMinorPieces = board.getPieceBitboard(WHITE_KNIGHT) | board.getPieceBitboard(WHITE_BISHOP);
    Bitboard blackMinorPieces = board.getPieceBitboard(BLACK_KNIGHT) | board.getPieceBitboard(BLACK_BISHOP);
    Bitboard whiteMinorPiecesAttacks = board.getPieceAttackBitboard(WHITE_KNIGHT) | board.getPieceAttackBitboard(WHITE_BISHOP);
    Bitboard blackMinorPiecesAttacks = board.getPieceAttackBitboard(BLACK_KNIGHT) | board.getPieceAttackBitboard(BLACK_BISHOP);

    int32_t numWhiteMinorPcOnStrongSquares = ((whiteMinorPieces | whiteMinorPiecesAttacks) & evaluationVars.whiteStrongSquares).popcount();
    int32_t numBlackMinorPcOnStrongSquares = ((blackMinorPieces | blackMinorPiecesAttacks) & evaluationVars.blackStrongSquares).popcount();

    return {numWhiteMinorPcOnStrongSquares * hceParams.getMGMinorPieceOnStrongSquareBonus() -
            numBlackMinorPcOnStrongSquares * hceParams.getMGMinorPieceOnStrongSquareBonus(), 0};
}

Score HandcraftedEvaluator::evaluateRooksOnOpenFiles() {
    Bitboard whiteRooks = board.getPieceBitboard(WHITE_ROOK);
    Bitboard blackRooks = board.getPieceBitboard(BLACK_ROOK);

    Bitboard whitePawns = board.getPieceBitboard(WHITE_PAWN);
    Bitboard blackPawns = board.getPieceBitboard(BLACK_PAWN);

    Score score = {0, 0};

    while(whiteRooks) {
        int32_t sq = whiteRooks.popFSB();
        int32_t file = Square::fileOf(sq);
        if(!(whitePawns & fileMasks[file])) {
            if(!(blackPawns & fileMasks[file]))
                score.mg += hceParams.getMGRookOnOpenFileBonus();
            else
                score.mg += hceParams.getMGRookOnSemiOpenFileBonus();
        }
    }

    while(blackRooks) {
        int32_t sq = blackRooks.popFSB();
        int32_t file = Square::fileOf(sq);
        if(!(blackPawns & fileMasks[file])) {
            if(!(whitePawns & fileMasks[file]))
                score.mg -= hceParams.getMGRookOnOpenFileBonus();
            else
                score.mg -= hceParams.getMGRookOnSemiOpenFileBonus();
        }
    }

    return score;
}

Score HandcraftedEvaluator::evaluateRooksBehindPassedPawns() {
    Bitboard whiteRooks = board.getPieceBitboard(WHITE_ROOK);
    Bitboard blackRooks = board.getPieceBitboard(BLACK_ROOK);

    Bitboard passedPawns = evaluationVars.whitePassedPawns | evaluationVars.blackPassedPawns;

    Score score = {0, 0};

    while(whiteRooks) {
        int32_t sq = whiteRooks.popFSB();
        if(passedPawns & fileFacingEnemy[WHITE / COLOR_MASK][sq])
            score.eg += hceParams.getEGRookBehindPassedPawnBonus();
    }

    while(blackRooks) {
        int32_t sq = blackRooks.popFSB();
        if(passedPawns & fileFacingEnemy[BLACK / COLOR_MASK][sq])
            score.eg -= hceParams.getEGRookBehindPassedPawnBonus();
    }

    return score;
}

int32_t kingDistance(int32_t sq1, int32_t sq2) {
    int32_t file1 = Square::fileOf(sq1);
    int32_t rank1 = Square::rankOf(sq1);
    int32_t file2 = Square::fileOf(sq2);
    int32_t rank2 = Square::rankOf(sq2);

    return std::max(std::abs(file1 - file2), std::abs(rank1 - rank2));
}

Score HandcraftedEvaluator::evaluateKingPawnProximity() {
    Score score = {0, 0};

    int32_t whiteKingSquare = board.getKingSquare(WHITE);
    int32_t blackKingSquare = board.getKingSquare(BLACK);

    Bitboard pawns = board.getPieceBitboard(WHITE_PAWN) | board.getPieceBitboard(BLACK_PAWN);
    Bitboard backwardPawns = evaluationVars.whiteBackwardPawns | evaluationVars.blackBackwardPawns;
    Bitboard passedPawns = evaluationVars.whitePassedPawns | evaluationVars.blackPassedPawns;

    while(pawns) {
        int32_t sq = pawns.popFSB();
        int32_t whiteKingDist = kingDistance(whiteKingSquare, sq);
        int32_t blackKingDist = kingDistance(blackKingSquare, sq);

        if(passedPawns.getBit(sq)) {
            score.eg += (7 - whiteKingDist) * hceParams.getEGKingProximityPassedPawnWeight();
            score.eg -= (7 - blackKingDist) * hceParams.getEGKingProximityPassedPawnWeight();
        } else if(backwardPawns.getBit(sq)) {
            score.eg += (7 - whiteKingDist) * hceParams.getEGKingProximityBackwardPawnWeight();
            score.eg -= (7 - blackKingDist) * hceParams.getEGKingProximityBackwardPawnWeight();
        } else {
            score.eg += (7 - whiteKingDist) * hceParams.getEGKingProximityPawnWeight();
            score.eg -= (7 - blackKingDist) * hceParams.getEGKingProximityPawnWeight();
        }
    }

    return score;
}

bool HandcraftedEvaluator::isWinnable(int32_t side) {
    Bitboard pawns = board.getPieceBitboard(side | PAWN);
    Bitboard minorPieces = board.getPieceBitboard(side | KNIGHT) | board.getPieceBitboard(side | BISHOP);
    Bitboard heavyPieces = board.getPieceBitboard(side | ROOK) | board.getPieceBitboard(side | QUEEN);

    return pawns || minorPieces.popcount() >= 2 || heavyPieces;
}

int32_t HandcraftedEvaluator::evaluateKNBKEndgame(int32_t b, int32_t k) {
    int32_t evaluation = std::abs(evaluationVars.materialScore.eg);

    b = b < 0 ? -1 : 0;
    k = (k >> 3) + ((k ^ b) & 7);
    int32_t manhattanDistToClosestCornerOfBishopSqColor = (15 * (k >> 3) ^ k) - (k >> 3);
    evaluation += (7 - manhattanDistToClosestCornerOfBishopSqColor) * EG_SPECIAL_MATE_PROGRESS_BONUS;

    return evaluation + EG_WINNING_BONUS;
}

int32_t HandcraftedEvaluator::evaluateWinningNoPawnsEndgame(int32_t k) {
    int32_t evaluation = std::abs(evaluationVars.materialScore.eg);

    int32_t file = SQ2F(k);
    int32_t rank = SQ2R(k);

    file ^= (file - 4) >> 8;
    rank ^= (rank - 4) >> 8;

    int32_t manhattanDistToCenter = (file + rank) & 7;
    evaluation += manhattanDistToCenter * EG_SPECIAL_MATE_PROGRESS_BONUS;

    return evaluation + EG_WINNING_BONUS;
}