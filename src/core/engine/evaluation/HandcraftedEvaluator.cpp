#include "core/engine/evaluation/HandcraftedEvaluator.h"

void HandcraftedEvaluator::updateBeforeMove(Move m) {
    evaluationHistory.push_back(evaluationVars);

    Score delta{0, 0};

    int32_t side = board.getSideToMove();
    int32_t piece = TYPEOF(board.pieceAt(m.getOrigin()));
    int32_t originPSQT = m.getOrigin();
    int32_t destinationPSQT = m.getDestination();

    // Für schwarze Figuren muss der Rang gespiegelt werden,
    // um die Positionstabelle zu indizieren
    if(side == BLACK) {
        originPSQT = Square::flipY(originPSQT);
        destinationPSQT = Square::flipY(destinationPSQT);
    }

    delta.mg += HCE_PARAMS.getMGPSQT(piece,destinationPSQT) - HCE_PARAMS.getMGPSQT(piece, originPSQT);
    delta.eg += HCE_PARAMS.getEGPSQT(piece, destinationPSQT) - HCE_PARAMS.getEGPSQT(piece, originPSQT);

    // Spezialfall: Schlagzug
    if(m.isCapture()) {
        int32_t capturedPiece = board.pieceAt(m.getDestination());
        int32_t capturedPieceSquare = destinationPSQT;

        // Spezialfall: En-Passant
        if(m.isEnPassant()) {
            capturedPiece = PAWN;
            capturedPieceSquare += SOUTH;
        }

        // Für geschlagene Figuren muss der Rang gespiegelt werden,
        // um die Positionstabelle zu indizieren
        capturedPieceSquare = Square::flipY(capturedPieceSquare);

        delta.mg += HCE_PARAMS.getMGPieceValue(TYPEOF(capturedPiece));
        delta.eg += HCE_PARAMS.getEGPieceValue(TYPEOF(capturedPiece));

        delta.mg += HCE_PARAMS.getMGPSQT(TYPEOF(capturedPiece), capturedPieceSquare);
        delta.eg += HCE_PARAMS.getEGPSQT(TYPEOF(capturedPiece), capturedPieceSquare);

        // Aktualisiere die Phasengewichte
        evaluationVars.phaseWeight += PIECE_WEIGHT[TYPEOF(capturedPiece)];
    }

    // Spezialfall: Rochade
    if(m.isCastle()) {
        if(m.isKingsideCastle()) {
            // Königsseite
            delta.mg += HCE_PARAMS.getMGPSQT(ROOK, F1) - HCE_PARAMS.getMGPSQT(ROOK, H1);
            delta.eg += HCE_PARAMS.getEGPSQT(ROOK, F1) - HCE_PARAMS.getEGPSQT(ROOK, H1);
        } else {
            // Damenseite
            delta.mg += HCE_PARAMS.getMGPSQT(ROOK, D1) - HCE_PARAMS.getMGPSQT(ROOK, A1);
            delta.eg += HCE_PARAMS.getEGPSQT(ROOK, D1) - HCE_PARAMS.getEGPSQT(ROOK, A1);
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

        delta.mg += HCE_PARAMS.getMGPieceValue(promotedPiece) - HCE_PARAMS.getMGPieceValue(PAWN);
        delta.eg += HCE_PARAMS.getEGPieceValue(promotedPiece) - HCE_PARAMS.getEGPieceValue(PAWN);

        delta.mg += HCE_PARAMS.getMGPSQT(promotedPiece, destinationPSQT) - HCE_PARAMS.getMGPSQT(PAWN, destinationPSQT);
        delta.eg += HCE_PARAMS.getEGPSQT(promotedPiece, destinationPSQT) - HCE_PARAMS.getEGPSQT(PAWN, destinationPSQT);

        // Aktualisiere die Phasengewichte
        evaluationVars.phaseWeight -= PIECE_WEIGHT[promotedPiece] - PIECE_WEIGHT[PAWN];
    }

    evaluationVars.materialScore += delta * (side == WHITE ? 1 : -1);

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
    Score score{0, 0};

    // Material
    for(int32_t piece = PAWN; piece < KING; piece++) {
        score.mg += board.getPieceBitboard(WHITE | piece).popcount() * HCE_PARAMS.getMGPieceValue(piece);
        score.eg += board.getPieceBitboard(WHITE | piece).popcount() * HCE_PARAMS.getEGPieceValue(piece);
        score.mg -= board.getPieceBitboard(BLACK | piece).popcount() * HCE_PARAMS.getMGPieceValue(piece);
        score.eg -= board.getPieceBitboard(BLACK | piece).popcount() * HCE_PARAMS.getEGPieceValue(piece);
    }

    // Positionstabellen
    for(int32_t piece = PAWN; piece <= KING; piece++) {
        Bitboard pieceBB = board.getPieceBitboard(WHITE | piece);
        while(pieceBB) {
            int32_t square = pieceBB.popFSB();
            score.mg += HCE_PARAMS.getMGPSQT(piece, square);
            score.eg += HCE_PARAMS.getEGPSQT(piece, square);
        }

        pieceBB = board.getPieceBitboard(BLACK | piece);
        while(pieceBB) {
            int32_t square = pieceBB.popFSB();

            // Für schwarze Figuren muss der Rang gespiegelt werden
            square = Square::flipY(square);

            score.mg -= HCE_PARAMS.getMGPSQT(piece, square);
            score.eg -= HCE_PARAMS.getEGPSQT(piece, square);
        }
    }

    evaluationVars.materialScore = score;
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
    while(connectedWhitePawns) {
        int32_t rank = Square::rankOf(connectedWhitePawns.popFSB());
        score.mg += HCE_PARAMS.getMGConnectedPawnBonus(rank);
        score.eg += HCE_PARAMS.getEGConnectedPawnBonus(rank);
    }

    while(connectedBlackPawns) {
        int32_t rank = Square::rankOf(Square::flipY(connectedBlackPawns.popFSB()));
        score.mg -= HCE_PARAMS.getMGConnectedPawnBonus(rank);
        score.eg -= HCE_PARAMS.getEGConnectedPawnBonus(rank);
    }

    // Doppelbauern
    while(doubleWhitePawns) {
        int32_t file = Square::fileOf(doubleWhitePawns.popFSB());
        score.mg += HCE_PARAMS.getMGDoubledPawnPenalty(file);
        score.eg += HCE_PARAMS.getEGDoubledPawnPenalty(file);
    }

    while(doubleBlackPawns) {
        int32_t file = Square::fileOf(doubleBlackPawns.popFSB());
        score.mg -= HCE_PARAMS.getMGDoubledPawnPenalty(file);
        score.eg -= HCE_PARAMS.getEGDoubledPawnPenalty(file);
    }

    // Isolierte Bauern
    while(isolatedWhitePawns) {
        int32_t file = Square::fileOf(isolatedWhitePawns.popFSB());
        score.mg += HCE_PARAMS.getMGIsolatedPawnPenalty(file);
        score.eg += HCE_PARAMS.getEGIsolatedPawnPenalty(file);
    }

    while(isolatedBlackPawns) {
        int32_t file = Square::fileOf(isolatedBlackPawns.popFSB());
        score.mg -= HCE_PARAMS.getMGIsolatedPawnPenalty(file);
        score.eg -= HCE_PARAMS.getEGIsolatedPawnPenalty(file);
    }

    // Rückständige Bauern
    while(backwardWhitePawns) {
        int32_t rank = Square::rankOf(backwardWhitePawns.popFSB());
        score.mg += HCE_PARAMS.getMGBackwardPawnPenalty(rank);
        score.eg += HCE_PARAMS.getEGBackwardPawnPenalty(rank);
    }

    while(backwardBlackPawns) {
        int32_t rank = Square::rankOf(Square::flipY(backwardBlackPawns.popFSB()));
        score.mg -= HCE_PARAMS.getMGBackwardPawnPenalty(rank);
        score.eg -= HCE_PARAMS.getEGBackwardPawnPenalty(rank);
    }

    // Freibauern
    while(whitePassedPawns) {
        int32_t rank = Square::rankOf(whitePassedPawns.popFSB());
        score.mg += HCE_PARAMS.getMGPassedPawnBonus(rank);
        score.eg += HCE_PARAMS.getEGPassedPawnBonus(rank);
    }

    while(blackPassedPawns) {
        int32_t rank = Square::rankOf(Square::flipY(blackPassedPawns.popFSB()));
        score.mg -= HCE_PARAMS.getMGPassedPawnBonus(rank);
        score.eg -= HCE_PARAMS.getEGPassedPawnBonus(rank);
    }

    // Starke Felder
    while(whiteStrongSqaures) {
        int32_t sq = whiteStrongSqaures.popFSB();
        int32_t rank = Square::rankOf(sq), file = Square::fileOf(sq);
        score.mg += HCE_PARAMS.getMGStrongSquareBonus(rank, file);
        score.eg += HCE_PARAMS.getEGStrongSquareBonus(rank, file);
    }

    while(blackStrongSqaures) {
        int32_t sq = Square::flipY(blackStrongSqaures.popFSB());
        int32_t rank = Square::rankOf(sq), file = Square::fileOf(sq);
        score.mg -= HCE_PARAMS.getMGStrongSquareBonus(rank, file);
        score.eg -= HCE_PARAMS.getEGStrongSquareBonus(rank, file);
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
    return evaluateKingAttackZone() + evaluateOpenFiles() + evaluatePawnStorm();
}

Score HandcraftedEvaluator::evaluateKingAttackZone() {
    Score score = {0, 0};

    int32_t whiteKingSquare = board.getKingSquare(WHITE);
    int32_t blackKingSquare = board.getKingSquare(BLACK);

    // Bestimme die Anzahl der Angreifer pro Figurentyp auf die Felder um den weißen König
    Bitboard kingZone = kingAttackZone[whiteKingSquare];
    int32_t numBlackAttackers = 0;
    Score blackAttackersWeight = {0, 0};

    Bitboard blackKnights = board.getPieceBitboard(BLACK_KNIGHT);
    Bitboard blackBishops = board.getPieceBitboard(BLACK_BISHOP);

    Bitboard blackMinorPieces = blackKnights | blackBishops;

    while(blackKnights) {
        int32_t sq = blackKnights.popFSB();
        Bitboard attacks = knightAttackBitboard(sq) & kingZone;
        if(attacks) {
            numBlackAttackers++;
            blackAttackersWeight += attacks.popcount() * Score{HCE_PARAMS.getMGKnightAttackBonus(), HCE_PARAMS.getEGKnightAttackBonus()};
        }
    }

    while(blackBishops) {
        int32_t sq = blackBishops.popFSB();
        Bitboard attacks = diagonalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(BLACK_KING)) & kingZone;
        if(attacks) {
            numBlackAttackers++;
            blackAttackersWeight += attacks.popcount() * Score{HCE_PARAMS.getMGBishopAttackBonus(), HCE_PARAMS.getEGBishopAttackBonus()};
        }
    }

    Bitboard blackRooks = board.getPieceBitboard(BLACK_ROOK);
    while(blackRooks) {
        int32_t sq = blackRooks.popFSB();
        Bitboard attacks = horizontalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(BLACK_KING)) & kingZone;
        if(attacks) {
            numBlackAttackers++;
            blackAttackersWeight += attacks.popcount() * Score{HCE_PARAMS.getMGRookAttackBonus(), HCE_PARAMS.getEGRookAttackBonus()};
        }
    }

    Bitboard blackQueens = board.getPieceBitboard(BLACK_QUEEN);
    while(blackQueens) {
        int32_t sq = blackQueens.popFSB();
        Bitboard attacks = (diagonalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(BLACK_KING)) |
                            horizontalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(BLACK_KING))) & kingZone;

        if(attacks) {
            numBlackAttackers++;
            blackAttackersWeight += attacks.popcount() * Score{HCE_PARAMS.getMGQueenAttackBonus(), HCE_PARAMS.getEGQueenAttackBonus()};
        }
    }

    numBlackAttackers = std::min(numBlackAttackers, 5);

    // Bestimme die Anzahl der Angreifer pro Figurentyp auf die Felder um den schwarzen König
    kingZone = kingAttackZone[blackKingSquare];
    int32_t numWhiteAttackers = 0;
    Score whiteAttackersWeight = {0, 0};

    Bitboard whiteKnights = board.getPieceBitboard(WHITE_KNIGHT);
    Bitboard whiteBishops = board.getPieceBitboard(WHITE_BISHOP);

    Bitboard whiteMinorPieces = whiteKnights | whiteBishops;

    while(whiteKnights) {
        int32_t sq = whiteKnights.popFSB();
        Bitboard attacks = knightAttackBitboard(sq) & kingZone;
        if(attacks) {
            numWhiteAttackers++;
            whiteAttackersWeight += attacks.popcount() * Score{HCE_PARAMS.getMGKnightAttackBonus(), HCE_PARAMS.getEGKnightAttackBonus()};
        }
    }

    while(whiteBishops) {
        int32_t sq = whiteBishops.popFSB();
        Bitboard attacks = diagonalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(WHITE_KING)) & kingZone;
        if(attacks) {
            numWhiteAttackers++;
            whiteAttackersWeight += attacks.popcount() * Score{HCE_PARAMS.getMGBishopAttackBonus(), HCE_PARAMS.getEGBishopAttackBonus()};
        }
    }

    Bitboard whiteRooks = board.getPieceBitboard(WHITE_ROOK);
    while(whiteRooks) {
        int32_t sq = whiteRooks.popFSB();
        Bitboard attacks = horizontalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(WHITE_KING)) & kingZone;
        if(attacks) {
            numWhiteAttackers++;
            whiteAttackersWeight += attacks.popcount() * Score{HCE_PARAMS.getMGRookAttackBonus(), HCE_PARAMS.getEGRookAttackBonus()};
        }
    }

    Bitboard whiteQueens = board.getPieceBitboard(WHITE_QUEEN);
    while(whiteQueens) {
        int32_t sq = whiteQueens.popFSB();
        Bitboard attacks = (diagonalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(WHITE_KING)) |
                            horizontalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(WHITE_KING))) & kingZone;
                        
        if(attacks) {
            numWhiteAttackers++;
            whiteAttackersWeight += attacks.popcount() * Score{HCE_PARAMS.getMGQueenAttackBonus(), HCE_PARAMS.getEGQueenAttackBonus()};
        }
    }

    numWhiteAttackers = std::min(numWhiteAttackers, 5);

    Bitboard whitePawns = board.getPieceBitboard(WHITE_PAWN);
    Bitboard blackPawns = board.getPieceBitboard(BLACK_PAWN);

    Bitboard whitePawnShieldMask = pawnShieldMask[WHITE / COLOR_MASK][whiteKingSquare];
    Bitboard blackPawnShieldMask = pawnShieldMask[BLACK / COLOR_MASK][blackKingSquare];

    Bitboard whitePawnShield = whitePawns & whitePawnShieldMask;
    Bitboard blackPawnShield = blackPawns & blackPawnShieldMask;

    int32_t whitePawnShieldSize = whitePawnShield.popcount();
    int32_t blackPawnShieldSize = blackPawnShield.popcount();

    whitePawnShieldSize = std::min(whitePawnShieldSize, 3);
    blackPawnShieldSize = std::min(blackPawnShieldSize, 3);

    int32_t whiteNumDefendingPieces = (whiteMinorPieces & kingAttackZone[whiteKingSquare]).popcount();
    int32_t blackNumDefendingPieces = (blackMinorPieces & kingAttackZone[blackKingSquare]).popcount();

    // Berechne die Bewertung
    score.mg += std::max(whiteAttackersWeight.mg * HCE_PARAMS.getMGNumAttackerWeight(numWhiteAttackers) / (100 * HCE_PARAMS.VALUE_ONE) -
                        blackNumDefendingPieces * HCE_PARAMS.getMGMinorPieceDefenderBonus() -
                        HCE_PARAMS.getMGPawnShieldSizeBonus(blackPawnShieldSize), 0);
    score.mg -= std::max(blackAttackersWeight.mg * HCE_PARAMS.getMGNumAttackerWeight(numBlackAttackers) / (100 * HCE_PARAMS.VALUE_ONE) -
                        whiteNumDefendingPieces * HCE_PARAMS.getMGMinorPieceDefenderBonus() -
                        HCE_PARAMS.getMGPawnShieldSizeBonus(whitePawnShieldSize), 0);

    score.eg += std::max(whiteAttackersWeight.eg * HCE_PARAMS.getEGNumAttackerWeight(numWhiteAttackers) / (100 * HCE_PARAMS.VALUE_ONE) -
                        blackNumDefendingPieces * HCE_PARAMS.getEGMinorPieceDefenderBonus() -
                        HCE_PARAMS.getEGPawnShieldSizeBonus(blackPawnShieldSize), 0);
    score.eg -= std::max(blackAttackersWeight.eg * HCE_PARAMS.getEGNumAttackerWeight(numBlackAttackers) / (100 * HCE_PARAMS.VALUE_ONE) -
                        whiteNumDefendingPieces * HCE_PARAMS.getEGMinorPieceDefenderBonus() -
                        HCE_PARAMS.getEGPawnShieldSizeBonus(whitePawnShieldSize), 0);

    return score;
}

Score HandcraftedEvaluator::evaluateOpenFiles() {
    Score score = {0, 0};

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

    score += {HCE_PARAMS.getMGKingOpenFilePenalty(whiteOpenFiles),
              HCE_PARAMS.getEGKingOpenFilePenalty(whiteOpenFiles)};

    score -= {HCE_PARAMS.getMGKingOpenFilePenalty(blackOpenFiles),
              HCE_PARAMS.getEGKingOpenFilePenalty(blackOpenFiles)};

    return score;
}

Score HandcraftedEvaluator::evaluatePawnStorm() {
    Score score = {0, 0};

    int32_t whiteKingSquare = board.getKingSquare(WHITE);
    int32_t blackKingSquare = board.getKingSquare(BLACK);

    Bitboard whitePawns = board.getPieceBitboard(WHITE_PAWN);
    Bitboard blackPawns = board.getPieceBitboard(BLACK_PAWN);

    Bitboard whitePawnStorm = pawnStormMask[WHITE / COLOR_MASK][whiteKingSquare] & blackPawns;
    Bitboard blackPawnStorm = pawnStormMask[BLACK / COLOR_MASK][blackKingSquare] & whitePawns;

    while(whitePawnStorm) {
        int32_t rank = Square::rankOf(whitePawnStorm.popFSB());
        score += {HCE_PARAMS.getMGPawnStormBonus(rank),
                  HCE_PARAMS.getEGPawnStormBonus(rank)};
    }

    while(blackPawnStorm) {
        int32_t rank = Square::rankOf(Square::flipY(blackPawnStorm.popFSB()));
        score -= {HCE_PARAMS.getMGPawnStormBonus(rank),
                  HCE_PARAMS.getEGPawnStormBonus(rank)};
    }

    return score;
}

Score HandcraftedEvaluator::calculatePieceScore() {
    return evaluatePieceMobility() + evaluateSafeCenterSpace() + evaluateMinorPiecesOnStrongSquares() + evaluateBishopPairs() +
           evaluateRooksOnOpenFiles() + evaluateRooksBehindPassedPawns() + evaluateKingPawnProximity();
}

Score HandcraftedEvaluator::evaluatePieceMobility() {
    Bitboard whiteMobilityMask = ~(board.getWhiteOccupiedBitboard() | board.getPieceBitboard(WHITE_KING) | board.getPieceAttackBitboard(BLACK_PAWN));
    Bitboard blackMobilityMask = ~(board.getBlackOccupiedBitboard() | board.getPieceBitboard(BLACK_KING) | board.getPieceAttackBitboard(WHITE_PAWN));
    Bitboard whiteKnightAttacks = board.getPieceAttackBitboard(WHITE_KNIGHT) & whiteMobilityMask;
    Bitboard blackKnightAttacks = board.getPieceAttackBitboard(BLACK_KNIGHT) & blackMobilityMask;
    Bitboard whiteBishopAttacks = board.getPieceAttackBitboard(WHITE_BISHOP) & whiteMobilityMask;
    Bitboard blackBishopAttacks = board.getPieceAttackBitboard(BLACK_BISHOP) & blackMobilityMask;
    Bitboard whiteRookAttacks = board.getPieceAttackBitboard(WHITE_ROOK) & whiteMobilityMask;
    Bitboard blackRookAttacks = board.getPieceAttackBitboard(BLACK_ROOK) & blackMobilityMask;
    Bitboard whiteQueenAttacks = board.getPieceAttackBitboard(WHITE_QUEEN) & whiteMobilityMask;
    Bitboard blackQueenAttacks = board.getPieceAttackBitboard(BLACK_QUEEN) & blackMobilityMask;

    int32_t mgMobilityScore = whiteKnightAttacks.popcount() * HCE_PARAMS.getMGPieceMobilityBonus(KNIGHT) +
                              whiteBishopAttacks.popcount() * HCE_PARAMS.getMGPieceMobilityBonus(BISHOP) +
                              whiteRookAttacks.popcount() * HCE_PARAMS.getMGPieceMobilityBonus(ROOK) +
                              whiteQueenAttacks.popcount() * HCE_PARAMS.getMGPieceMobilityBonus(QUEEN) -
                              blackKnightAttacks.popcount() * HCE_PARAMS.getMGPieceMobilityBonus(KNIGHT) -
                              blackBishopAttacks.popcount() * HCE_PARAMS.getMGPieceMobilityBonus(BISHOP) -
                              blackRookAttacks.popcount() * HCE_PARAMS.getMGPieceMobilityBonus(ROOK) -
                              blackQueenAttacks.popcount() * HCE_PARAMS.getMGPieceMobilityBonus(QUEEN);

    int32_t egMobilityScore = whiteKnightAttacks.popcount() * HCE_PARAMS.getEGPieceMobilityBonus(KNIGHT) +
                              whiteBishopAttacks.popcount() * HCE_PARAMS.getEGPieceMobilityBonus(BISHOP) +
                              whiteRookAttacks.popcount() * HCE_PARAMS.getEGPieceMobilityBonus(ROOK) +
                              whiteQueenAttacks.popcount() * HCE_PARAMS.getEGPieceMobilityBonus(QUEEN) -
                              blackKnightAttacks.popcount() * HCE_PARAMS.getEGPieceMobilityBonus(KNIGHT) -
                              blackBishopAttacks.popcount() * HCE_PARAMS.getEGPieceMobilityBonus(BISHOP) -
                              blackRookAttacks.popcount() * HCE_PARAMS.getEGPieceMobilityBonus(ROOK) -
                              blackQueenAttacks.popcount() * HCE_PARAMS.getEGPieceMobilityBonus(QUEEN);

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

    return {(whiteNumSafeCenterSquares - blackNumSafeCenterSquares) * HCE_PARAMS.getMGSpaceBonus(), 0};
}

Score HandcraftedEvaluator::evaluateMinorPiecesOnStrongSquares() {
    Bitboard whiteMinorPieces = board.getPieceBitboard(WHITE_KNIGHT) | board.getPieceBitboard(WHITE_BISHOP);
    Bitboard blackMinorPieces = board.getPieceBitboard(BLACK_KNIGHT) | board.getPieceBitboard(BLACK_BISHOP);
    Bitboard whiteMinorPiecesAttacks = board.getPieceAttackBitboard(WHITE_KNIGHT) | board.getPieceAttackBitboard(WHITE_BISHOP);
    Bitboard blackMinorPiecesAttacks = board.getPieceAttackBitboard(BLACK_KNIGHT) | board.getPieceAttackBitboard(BLACK_BISHOP);

    int32_t numWhiteMinorPcOnStrongSquares = ((whiteMinorPieces | whiteMinorPiecesAttacks) & evaluationVars.whiteStrongSquares).popcount();
    int32_t numBlackMinorPcOnStrongSquares = ((blackMinorPieces | blackMinorPiecesAttacks) & evaluationVars.blackStrongSquares).popcount();

    return {numWhiteMinorPcOnStrongSquares * HCE_PARAMS.getMGMinorPieceOnStrongSquareBonus() -
            numBlackMinorPcOnStrongSquares * HCE_PARAMS.getMGMinorPieceOnStrongSquareBonus(),
            numWhiteMinorPcOnStrongSquares * HCE_PARAMS.getEGMinorPieceOnStrongSquareBonus() -
            numBlackMinorPcOnStrongSquares * HCE_PARAMS.getEGMinorPieceOnStrongSquareBonus()};
}

Score HandcraftedEvaluator::evaluateBishopPairs() {
    Bitboard whiteBishops = board.getPieceBitboard(WHITE_BISHOP);
    Bitboard blackBishops = board.getPieceBitboard(BLACK_BISHOP);

    int32_t numWhiteBishops = whiteBishops.popcount();
    int32_t numBlackBishops = blackBishops.popcount();

    Score score = {0, 0};

    if(numWhiteBishops >= 2)
        score += {HCE_PARAMS.getMGBishopPairBonus(), HCE_PARAMS.getEGBishopPairBonus()};

    if(numBlackBishops >= 2)
        score -= {HCE_PARAMS.getMGBishopPairBonus(), HCE_PARAMS.getEGBishopPairBonus()};

    return score;
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
                score += {HCE_PARAMS.getMGRookOnOpenFileBonus(),
                          HCE_PARAMS.getEGRookOnOpenFileBonus()};
            else
                score += {HCE_PARAMS.getMGRookOnSemiOpenFileBonus(),
                          HCE_PARAMS.getEGRookOnSemiOpenFileBonus()};
        }
    }

    while(blackRooks) {
        int32_t sq = blackRooks.popFSB();
        int32_t file = Square::fileOf(sq);
        if(!(blackPawns & fileMasks[file])) {
            if(!(whitePawns & fileMasks[file]))
                score -= {HCE_PARAMS.getMGRookOnOpenFileBonus(),
                          HCE_PARAMS.getEGRookOnOpenFileBonus()};
            else
                score -= {HCE_PARAMS.getMGRookOnSemiOpenFileBonus(),
                          HCE_PARAMS.getEGRookOnSemiOpenFileBonus()};
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
            score += {HCE_PARAMS.getMGRookBehindPassedPawnBonus(),
                      HCE_PARAMS.getEGRookBehindPassedPawnBonus()};
    }

    while(blackRooks) {
        int32_t sq = blackRooks.popFSB();
        if(passedPawns & fileFacingEnemy[BLACK / COLOR_MASK][sq])
            score -= {HCE_PARAMS.getMGRookBehindPassedPawnBonus(),
                      HCE_PARAMS.getEGRookBehindPassedPawnBonus()};
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
            score.eg += (7 - whiteKingDist) * HCE_PARAMS.getEGKingProximityPassedPawnWeight();
            score.eg -= (7 - blackKingDist) * HCE_PARAMS.getEGKingProximityPassedPawnWeight();
        } else if(backwardPawns.getBit(sq)) {
            score.eg += (7 - whiteKingDist) * HCE_PARAMS.getEGKingProximityBackwardPawnWeight();
            score.eg -= (7 - blackKingDist) * HCE_PARAMS.getEGKingProximityBackwardPawnWeight();
        } else {
            score.eg += (7 - whiteKingDist) * HCE_PARAMS.getEGKingProximityPawnWeight();
            score.eg -= (7 - blackKingDist) * HCE_PARAMS.getEGKingProximityPawnWeight();
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