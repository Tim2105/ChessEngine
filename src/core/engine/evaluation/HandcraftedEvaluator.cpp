#include "core/engine/evaluation/HandcraftedEvaluator.h"
#include "core/utils/magics/Magics.h"

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

    Bitboard whitePawnAttacks = board.getAttackBitboard(WHITE_PAWN);
    Bitboard blackPawnAttacks = board.getAttackBitboard(BLACK_PAWN);

    Bitboard whitePawnsWestEast = whitePawns.shiftWest() | whitePawns.shiftEast();
    Bitboard blackPawnsWestEast = blackPawns.shiftWest() | blackPawns.shiftEast();

    // Doppelbauern
    Bitboard doubledWhitePawns = whitePawns.shiftSouth().extrudeSouth() & whitePawns;
    Bitboard doubledBlackPawns = blackPawns.shiftNorth().extrudeNorth() & blackPawns;
    score.mg += hceParams.getMGDoubledPawnPenalty() * (doubledWhitePawns.popcount() - doubledBlackPawns.popcount());
    score.eg += hceParams.getEGDoubledPawnPenalty() * (doubledWhitePawns.popcount() - doubledBlackPawns.popcount());

    // Bauerninseln
    uint32_t whitePawnOnFile = (uint8_t)whitePawns.extrudeSouth().toU64();
    uint32_t blackPawnOnFile = (uint8_t)blackPawns.extrudeSouth().toU64();

    int32_t whitePawnIslands = std::popcount(~whitePawnOnFile & (whitePawnOnFile << 1));
    int32_t blackPawnIslands = std::popcount(~blackPawnOnFile & (blackPawnOnFile << 1));

    score.mg += hceParams.getMGPawnIslandPenalty() * (whitePawnIslands - blackPawnIslands);
    score.eg += hceParams.getEGPawnIslandPenalty() * (whitePawnIslands - blackPawnIslands);

    // Rückständige Bauern
    Bitboard backwardWhitePawns = whitePawns & ~whitePawnsWestEast & whitePawnsWestEast.extrudeSouth() & (blackPawns | blackPawnAttacks).shiftSouth() & ~whitePawnAttacks.extrudeNorth();
    Bitboard backwardBlackPawns = blackPawns & ~blackPawnsWestEast & blackPawnsWestEast.extrudeNorth() & (whitePawns | whitePawnAttacks).shiftNorth() & ~blackPawnAttacks.extrudeSouth();
    evaluationVars.whiteBackwardPawns = backwardWhitePawns;
    evaluationVars.blackBackwardPawns = backwardBlackPawns;
    score.mg += hceParams.getMGBackwardPawnPenalty() * (backwardWhitePawns.popcount() - backwardBlackPawns.popcount());
    score.eg += hceParams.getEGBackwardPawnPenalty() * (backwardWhitePawns.popcount() - backwardBlackPawns.popcount());

    // Verbundene Bauern
    Bitboard connectedWhitePawns = (whitePawnsWestEast | whitePawnsWestEast.shiftNorth() | whitePawnsWestEast.shiftSouth()) & whitePawns;
    while(connectedWhitePawns) {
        int32_t rank = Square::rankOf(connectedWhitePawns.popFSB());
        score.mg += hceParams.getMGConnectedPawnBonus(rank);
        score.eg += hceParams.getEGConnectedPawnBonus(rank);
    }

    Bitboard connectedBlackPawns = (blackPawnsWestEast | blackPawnsWestEast.shiftSouth() | blackPawnsWestEast.shiftNorth()) & blackPawns;
    while(connectedBlackPawns) {
        int32_t rank = Square::rankOf(Square::flipY(connectedBlackPawns.popFSB()));
        score.mg -= hceParams.getMGConnectedPawnBonus(rank);
        score.eg -= hceParams.getEGConnectedPawnBonus(rank);
    }

    // Freibauern
    Bitboard whitePassedPawns = whitePawns & ~doubledWhitePawns & ~((blackPawns | blackPawnAttacks).extrudeSouth());
    evaluationVars.whitePassedPawns = whitePassedPawns;
    while(whitePassedPawns) {
        int32_t rank = Square::rankOf(whitePassedPawns.popFSB());
        score.mg += hceParams.getMGPassedPawnBonus(rank);
        score.eg += hceParams.getEGPassedPawnBonus(rank);
    }

    Bitboard blackPassedPawns = blackPawns & ~doubledBlackPawns & ~((whitePawns | whitePawnAttacks).extrudeNorth());
    evaluationVars.blackPassedPawns = blackPassedPawns;
    while(blackPassedPawns) {
        int32_t rank = Square::rankOf(Square::flipY(blackPassedPawns.popFSB()));
        score.mg -= hceParams.getMGPassedPawnBonus(rank);
        score.eg -= hceParams.getEGPassedPawnBonus(rank);
    }

    // Freibauerkandidaten
    Bitboard whiteCandidatePassedPawns = whitePawns & ~(evaluationVars.whitePassedPawns | blackPawns.extrudeSouth());
    Bitboard blackCandidatePassedPawns = blackPawns & ~(evaluationVars.blackPassedPawns | whitePawns.extrudeNorth());

    Bitboard whiteSupportCandidates = whitePawns & ~doubledWhitePawns;
    Bitboard blackSupportCandidates = blackPawns & ~doubledBlackPawns;

    while(whiteCandidatePassedPawns) {
        Bitboard sq = 1ULL << whiteCandidatePassedPawns.popFSB();
        Bitboard pathToPromotion = sq.extrudeNorth();
        Bitboard blackOpposition = (pathToPromotion.shiftNorthWest() | pathToPromotion.shiftNorthEast()) & blackPawns;
        Bitboard whiteSupport = blackOpposition.shiftSouth(2).extrudeSouth() & whiteSupportCandidates & ~((sq.shiftWest() | sq.shiftEast()).extrudeSouth() & blackPawns).extrudeSouth();
        
        if(whiteSupport.popcount() >= blackOpposition.popcount()) {
            int32_t rank = Square::rankOf(sq.getFSB());
            score += Score{hceParams.getMGCandidatePassedPawnBonus(rank), hceParams.getEGCandidatePassedPawnBonus(rank)};
        }
    }

    while(blackCandidatePassedPawns) {
        Bitboard sq = 1ULL << blackCandidatePassedPawns.popFSB();
        Bitboard pathToPromotion = sq.extrudeSouth();
        Bitboard whiteOpposition = (pathToPromotion.shiftSouthWest() | pathToPromotion.shiftSouthEast()) & whitePawns;
        Bitboard blackSupport = whiteOpposition.shiftNorth(2).extrudeNorth() & blackSupportCandidates & ~((sq.shiftWest() | sq.shiftEast()).extrudeNorth() & whitePawns).extrudeNorth();

        if(blackSupport.popcount() >= whiteOpposition.popcount()) {
            int32_t rank = Square::rankOf(Square::flipY(sq.getFSB()));
            score -= Score{hceParams.getMGCandidatePassedPawnBonus(rank), hceParams.getEGCandidatePassedPawnBonus(rank)};
        }
    }

    // Unbewegbare Bauern
    Bitboard pawns = whitePawns | blackPawns;
    evaluationVars.whiteImmobilePawns = whitePawns & pawns.shiftSouth() & ~blackPawnAttacks;
    evaluationVars.blackImmobilePawns = blackPawns & pawns.shiftNorth() & ~whitePawnAttacks;

    // Starke Felder
    constexpr Bitboard RANK_4_TO_6 = 0xFFFFFF000000ULL;
    constexpr Bitboard RANK_3_TO_5 = 0xFFFFFF0000ULL;
    evaluationVars.whiteStrongSquares = whitePawnAttacks & ~blackPawnAttacks.extrudeSouth() & RANK_4_TO_6;
    evaluationVars.blackStrongSquares = blackPawnAttacks & ~whitePawnAttacks.extrudeNorth() & RANK_3_TO_5;

    score.mg += hceParams.getMGStrongSquareBonus() * (evaluationVars.whiteStrongSquares.popcount() - evaluationVars.blackStrongSquares.popcount());

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

    Bitboard whitePawns = board.getPieceBitboard(WHITE_PAWN);
    Bitboard blackPawns = board.getPieceBitboard(BLACK_PAWN);

    // Bestimme die Anzahl der Angreifer pro Figurentyp auf die Felder um den weißen König
    Bitboard kingZone = kingAttackZone[whiteKingSquare];
    Bitboard vulnerableKingZone = kingZone & ~(whitePawns.shiftNorth().extrudeNorth() | board.getAttackBitboard(WHITE_PAWN));
    int32_t numBlackAttackers = 0;
    int32_t blackAttackersWeight = 0;

    Bitboard blackKnights = board.getPieceBitboard(BLACK_KNIGHT);
    while(blackKnights) {
        int32_t sq = blackKnights.popFSB();
        Bitboard attacks = knightAttackBitboard(sq) & kingZone;
        if(attacks) {
            numBlackAttackers++;
            numBlackAttackers += (bool)(attacks & vulnerableKingZone);
            blackAttackersWeight += attacks.popcount() * hceParams.getKnightAttackBonus();
        }
    }

    Bitboard blackBishops = board.getPieceBitboard(BLACK_BISHOP);
    while(blackBishops) {
        int32_t sq = blackBishops.popFSB();
        Bitboard attacks = diagonalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(BLACK_KING)) & kingZone;
        if(attacks) {
            numBlackAttackers++;
            numBlackAttackers += (bool)(attacks & vulnerableKingZone);
            blackAttackersWeight += attacks.popcount() * hceParams.getBishopAttackBonus();
        }
    }

    Bitboard blackRooks = board.getPieceBitboard(BLACK_ROOK);
    while(blackRooks) {
        int32_t sq = blackRooks.popFSB();
        Bitboard attacks = horizontalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(BLACK_KING)) & kingZone;
        if(attacks) {
            numBlackAttackers++;
            numBlackAttackers += (bool)(attacks & vulnerableKingZone);
            blackAttackersWeight += attacks.popcount() * hceParams.getRookAttackBonus();
        }
    }

    Bitboard blackQueens = board.getPieceBitboard(BLACK_QUEEN);
    while(blackQueens) {
        int32_t sq = blackQueens.popFSB();
        Bitboard attacks = (diagonalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(BLACK_KING)) |
                            horizontalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(BLACK_KING))) & kingZone;

        if(attacks) {
            numBlackAttackers++;
            numBlackAttackers += (bool)(attacks & vulnerableKingZone);
            blackAttackersWeight += attacks.popcount() * hceParams.getQueenAttackBonus();
        }
    }

    numBlackAttackers = std::min(numBlackAttackers, 9);

    // Bestimme die Anzahl der Angreifer pro Figurentyp auf die Felder um den schwarzen König
    kingZone = kingAttackZone[blackKingSquare];
    vulnerableKingZone = kingZone & ~(blackPawns.shiftSouth().extrudeSouth() | board.getAttackBitboard(BLACK_PAWN));
    int32_t numWhiteAttackers = 0;
    int32_t whiteAttackersWeight = 0;

    Bitboard whiteKnights = board.getPieceBitboard(WHITE_KNIGHT);
    while(whiteKnights) {
        int32_t sq = whiteKnights.popFSB();
        Bitboard attacks = knightAttackBitboard(sq) & kingZone;
        if(attacks) {
            numWhiteAttackers++;
            numWhiteAttackers += (bool)(attacks & vulnerableKingZone);
            whiteAttackersWeight += attacks.popcount() * hceParams.getKnightAttackBonus();
        }
    }

    Bitboard whiteBishops = board.getPieceBitboard(WHITE_BISHOP);
    while(whiteBishops) {
        int32_t sq = whiteBishops.popFSB();
        Bitboard attacks = diagonalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(WHITE_KING)) & kingZone;
        if(attacks) {
            numWhiteAttackers++;
            numWhiteAttackers += (bool)(attacks & vulnerableKingZone);
            whiteAttackersWeight += attacks.popcount() * hceParams.getBishopAttackBonus();
        }
    }

    Bitboard whiteRooks = board.getPieceBitboard(WHITE_ROOK);
    while(whiteRooks) {
        int32_t sq = whiteRooks.popFSB();
        Bitboard attacks = horizontalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(WHITE_KING)) & kingZone;
        if(attacks) {
            numWhiteAttackers++;
            numWhiteAttackers += (bool)(attacks & vulnerableKingZone);
            whiteAttackersWeight += attacks.popcount() * hceParams.getRookAttackBonus();
        }
    }

    Bitboard whiteQueens = board.getPieceBitboard(WHITE_QUEEN);
    while(whiteQueens) {
        int32_t sq = whiteQueens.popFSB();
        Bitboard attacks = (diagonalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(WHITE_KING)) |
                            horizontalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(WHITE_KING))) & kingZone;
                        
        if(attacks) {
            numWhiteAttackers++;
            numWhiteAttackers += (bool)(attacks & vulnerableKingZone);
            whiteAttackersWeight += attacks.popcount() * hceParams.getQueenAttackBonus();
        }
    }

    numWhiteAttackers = std::min(numWhiteAttackers, 9);

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

    Bitboard whitePawnStorm = pawnStormMask[WHITE / COLOR_MASK][whiteKingSquare] & blackPawns & ~evaluationVars.blackImmobilePawns;
    Bitboard blackPawnStorm = pawnStormMask[BLACK / COLOR_MASK][blackKingSquare] & whitePawns & ~evaluationVars.whiteImmobilePawns;

    while(whitePawnStorm) {
        int32_t rank = Square::rankOf(whitePawnStorm.popFSB());
        score += hceParams.getMGPawnStormPenalty(rank);
    }

    while(blackPawnStorm) {
        int32_t rank = Square::rankOf(Square::flipY(blackPawnStorm.popFSB()));
        score -= hceParams.getMGPawnStormPenalty(rank);
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
    return evaluateAttackedPieces() + evaluatePieceMobility() + evaluateSafeCenterSpace() + evaluateMinorPiecesOnStrongSquares() +
           evaluateBadBishops() + evaluateRooksOnOpenFiles() + evaluateRooksBehindPassedPawns() + evaluateBlockedPassedPawns() +
           evaluateKingPawnProximity() + evaluateRuleOfTheSquare();
}

Score HandcraftedEvaluator::evaluateAttackedPieces() {
    Score score{0, 0};

    Bitboard whitePawnAttacks = board.getAttackBitboard(WHITE_PAWN);
    Bitboard blackPawnAttacks = board.getAttackBitboard(BLACK_PAWN);

    Bitboard whiteMinorPieceAttacks = board.getAttackBitboard(WHITE_KNIGHT) | board.getAttackBitboard(WHITE_BISHOP);
    Bitboard blackMinorPieceAttacks = board.getAttackBitboard(BLACK_KNIGHT) | board.getAttackBitboard(BLACK_BISHOP);

    Bitboard whiteRookAttacks = board.getAttackBitboard(WHITE_ROOK);
    Bitboard blackRookAttacks = board.getAttackBitboard(BLACK_ROOK);

    int32_t attackDiff = (board.getPieceBitboard(BLACK_PAWN) & ~blackPawnAttacks & whiteMinorPieceAttacks).popcount() -
                         (board.getPieceBitboard(WHITE_PAWN) & ~whitePawnAttacks & blackMinorPieceAttacks).popcount();

    score += {
        attackDiff * hceParams.getMGAttackByMinorPieceBonus(PAWN),
        attackDiff * hceParams.getEGAttackByMinorPieceBonus(PAWN)
    };

    attackDiff = (board.getPieceBitboard(BLACK_PAWN) & ~blackPawnAttacks & whiteRookAttacks).popcount() -
                 (board.getPieceBitboard(WHITE_PAWN) & ~whitePawnAttacks & blackRookAttacks).popcount();

    score += {
        attackDiff * hceParams.getMGAttackByRookBonus(PAWN),
        attackDiff * hceParams.getEGAttackByRookBonus(PAWN)
    };

    attackDiff = (board.getPieceBitboard(BLACK_KNIGHT) & whiteMinorPieceAttacks).popcount() -
                 (board.getPieceBitboard(WHITE_KNIGHT) & blackMinorPieceAttacks).popcount();

    score += {
        attackDiff * hceParams.getMGAttackByMinorPieceBonus(KNIGHT),
        attackDiff * hceParams.getEGAttackByMinorPieceBonus(KNIGHT)
    };

    attackDiff = (board.getPieceBitboard(BLACK_KNIGHT) & ~blackPawnAttacks & whiteRookAttacks).popcount() -
                 (board.getPieceBitboard(WHITE_KNIGHT) & ~whitePawnAttacks & blackRookAttacks).popcount();

    score += {
        attackDiff * hceParams.getMGAttackByRookBonus(KNIGHT),
        attackDiff * hceParams.getEGAttackByRookBonus(KNIGHT)
    };

    attackDiff = (board.getPieceBitboard(BLACK_BISHOP) & whiteMinorPieceAttacks).popcount() -
                 (board.getPieceBitboard(WHITE_BISHOP) & blackMinorPieceAttacks).popcount();

    score += {
        attackDiff * hceParams.getMGAttackByMinorPieceBonus(BISHOP),
        attackDiff * hceParams.getEGAttackByMinorPieceBonus(BISHOP)
    };

    attackDiff = (board.getPieceBitboard(BLACK_BISHOP) & ~blackPawnAttacks & whiteRookAttacks).popcount() -
                 (board.getPieceBitboard(WHITE_BISHOP) & ~whitePawnAttacks & blackRookAttacks).popcount();

    score += {
        attackDiff * hceParams.getMGAttackByRookBonus(BISHOP),
        attackDiff * hceParams.getEGAttackByRookBonus(BISHOP)
    };

    attackDiff = (board.getPieceBitboard(BLACK_ROOK) & whiteMinorPieceAttacks).popcount() -
                 (board.getPieceBitboard(WHITE_ROOK) & blackMinorPieceAttacks).popcount();

    score += {
        attackDiff * hceParams.getMGAttackByMinorPieceBonus(ROOK),
        attackDiff * hceParams.getEGAttackByMinorPieceBonus(ROOK)
    };

    attackDiff = (board.getPieceBitboard(BLACK_ROOK) & whiteRookAttacks).popcount() -
                 (board.getPieceBitboard(WHITE_ROOK) & blackRookAttacks).popcount();

    score += {
        attackDiff * hceParams.getMGAttackByRookBonus(ROOK),
        attackDiff * hceParams.getEGAttackByRookBonus(ROOK)
    };

    attackDiff = (board.getPieceBitboard(BLACK_QUEEN) & whiteMinorPieceAttacks).popcount() -
                 (board.getPieceBitboard(WHITE_QUEEN) & blackMinorPieceAttacks).popcount();

    score += {
        attackDiff * hceParams.getMGAttackByMinorPieceBonus(QUEEN),
        attackDiff * hceParams.getEGAttackByMinorPieceBonus(QUEEN)
    };

    attackDiff = (board.getPieceBitboard(BLACK_QUEEN) & whiteRookAttacks).popcount() -
                 (board.getPieceBitboard(WHITE_QUEEN) & blackRookAttacks).popcount();

    score += {
        attackDiff * hceParams.getMGAttackByRookBonus(QUEEN),
        attackDiff * hceParams.getEGAttackByRookBonus(QUEEN)
    };

    return score;
}

Score HandcraftedEvaluator::evaluatePieceMobility() {
    Score score{0, 0};

    Bitboard whiteBlockedPawns = board.getPieceBitboard(WHITE_PAWN) & (board.getPieceBitboard(BLACK) | board.getPieceBitboard(BLACK_KING)).shiftSouth();
    Bitboard blackBlockedPawns = board.getPieceBitboard(BLACK_PAWN) & (board.getPieceBitboard(WHITE) | board.getPieceBitboard(WHITE_KING)).shiftNorth();

    Bitboard whiteNotReachableSquares = whiteBlockedPawns | board.getPieceBitboard(WHITE_KING) | board.getAttackBitboard(BLACK_PAWN);
    Bitboard blackNotReachableSquares = blackBlockedPawns | board.getPieceBitboard(BLACK_KING) | board.getAttackBitboard(WHITE_PAWN);

    Bitboard whiteKnights = board.getPieceBitboard(WHITE_KNIGHT);
    while(whiteKnights) {
        int32_t sq = whiteKnights.popFSB();
        Bitboard attacks = knightAttackBitboard(sq) & ~whiteNotReachableSquares;
        int32_t numAttacks = attacks.popcount();

        if(numAttacks == 0)
            score += {hceParams.getMGPieceNoMobilityPenalty(KNIGHT), hceParams.getEGPieceNoMobilityPenalty(KNIGHT)};
        else {
            numAttacks = (16 - std::countl_zero((uint16_t)(numAttacks * numAttacks))) * hceParams.getMGPieceMobilityBonus(KNIGHT);
            score += {numAttacks * hceParams.getMGPieceMobilityBonus(KNIGHT), numAttacks * hceParams.getEGPieceMobilityBonus(KNIGHT)};
        }
    }

    Bitboard blackKnights = board.getPieceBitboard(BLACK_KNIGHT);
    while(blackKnights) {
        int32_t sq = blackKnights.popFSB();
        Bitboard attacks = knightAttackBitboard(sq) & ~blackNotReachableSquares;
        int32_t numAttacks = attacks.popcount();

        if(numAttacks == 0)
            score -= {hceParams.getMGPieceNoMobilityPenalty(KNIGHT), hceParams.getEGPieceNoMobilityPenalty(KNIGHT)};
        else {
            numAttacks = (16 - std::countl_zero((uint16_t)(numAttacks * numAttacks))) * hceParams.getMGPieceMobilityBonus(KNIGHT);
            score -= {numAttacks * hceParams.getMGPieceMobilityBonus(KNIGHT), numAttacks * hceParams.getEGPieceMobilityBonus(KNIGHT)};
        }
    }

    Bitboard whiteBishops = board.getPieceBitboard(WHITE_BISHOP);
    while(whiteBishops) {
        int32_t sq = whiteBishops.popFSB();
        Bitboard attacks = diagonalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(WHITE_KING)) & ~whiteNotReachableSquares;
        int32_t numAttacks = attacks.popcount();

        if(numAttacks == 0)
            score += {hceParams.getMGPieceNoMobilityPenalty(BISHOP), hceParams.getEGPieceNoMobilityPenalty(BISHOP)};
        else {
            numAttacks = (16 - std::countl_zero((uint16_t)(numAttacks * numAttacks))) * hceParams.getMGPieceMobilityBonus(BISHOP);
            score += {numAttacks * hceParams.getMGPieceMobilityBonus(BISHOP), numAttacks * hceParams.getEGPieceMobilityBonus(BISHOP)};
        }
    }

    Bitboard blackBishops = board.getPieceBitboard(BLACK_BISHOP);
    while(blackBishops) {
        int32_t sq = blackBishops.popFSB();
        Bitboard attacks = diagonalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(BLACK_KING)) & ~blackNotReachableSquares;
        int32_t numAttacks = attacks.popcount();

        if(numAttacks == 0)
            score -= {hceParams.getMGPieceNoMobilityPenalty(BISHOP), hceParams.getEGPieceNoMobilityPenalty(BISHOP)};
        else {
            numAttacks = (16 - std::countl_zero((uint16_t)(numAttacks * numAttacks))) * hceParams.getMGPieceMobilityBonus(BISHOP);
            score -= {numAttacks * hceParams.getMGPieceMobilityBonus(BISHOP), numAttacks * hceParams.getEGPieceMobilityBonus(BISHOP)};
        }
    }

    whiteNotReachableSquares |= board.getAttackBitboard(BLACK_KNIGHT) | board.getAttackBitboard(BLACK_BISHOP);
    blackNotReachableSquares |= board.getAttackBitboard(WHITE_KNIGHT) | board.getAttackBitboard(WHITE_BISHOP);

    Bitboard whiteRooks = board.getPieceBitboard(WHITE_ROOK);
    while(whiteRooks) {
        int32_t sq = whiteRooks.popFSB();
        Bitboard attacks = horizontalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(WHITE_KING)) & ~whiteNotReachableSquares;
        int32_t numAttacks = attacks.popcount();

        if(numAttacks == 0)
            score += {hceParams.getMGPieceNoMobilityPenalty(ROOK), hceParams.getEGPieceNoMobilityPenalty(ROOK)};
        else {
            numAttacks = (16 - std::countl_zero((uint16_t)(numAttacks * numAttacks))) * hceParams.getMGPieceMobilityBonus(ROOK);
            score += {numAttacks * hceParams.getMGPieceMobilityBonus(ROOK), numAttacks * hceParams.getEGPieceMobilityBonus(ROOK)};
        }
    }

    Bitboard blackRooks = board.getPieceBitboard(BLACK_ROOK);
    while(blackRooks) {
        int32_t sq = blackRooks.popFSB();
        Bitboard attacks = horizontalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(BLACK_KING)) & ~blackNotReachableSquares;
        int32_t numAttacks = attacks.popcount();

        if(numAttacks == 0)
            score -= {hceParams.getMGPieceNoMobilityPenalty(ROOK), hceParams.getEGPieceNoMobilityPenalty(ROOK)};
        else {
            numAttacks = (16 - std::countl_zero((uint16_t)(numAttacks * numAttacks))) * hceParams.getMGPieceMobilityBonus(ROOK);
            score -= {numAttacks * hceParams.getMGPieceMobilityBonus(ROOK), numAttacks * hceParams.getEGPieceMobilityBonus(ROOK)};
        }
    }

    whiteNotReachableSquares |= board.getAttackBitboard(BLACK_ROOK);
    blackNotReachableSquares |= board.getAttackBitboard(WHITE_ROOK);

    Bitboard whiteQueens = board.getPieceBitboard(WHITE_QUEEN);
    while(whiteQueens) {
        int32_t sq = whiteQueens.popFSB();
        Bitboard attacks = (diagonalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(WHITE_KING)) |
                            horizontalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(WHITE_KING))) & ~whiteNotReachableSquares;
        int32_t numAttacks = attacks.popcount();

        if(numAttacks == 0)
            score += {hceParams.getMGPieceNoMobilityPenalty(QUEEN), hceParams.getEGPieceNoMobilityPenalty(QUEEN)};
        else {
            numAttacks = (16 - std::countl_zero((uint16_t)(numAttacks * numAttacks))) * hceParams.getMGPieceMobilityBonus(QUEEN);
            score += {numAttacks * hceParams.getMGPieceMobilityBonus(QUEEN), numAttacks * hceParams.getEGPieceMobilityBonus(QUEEN)};
        }
    }

    Bitboard blackQueens = board.getPieceBitboard(BLACK_QUEEN);
    while(blackQueens) {
        int32_t sq = blackQueens.popFSB();
        Bitboard attacks = (diagonalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(BLACK_KING)) |
                            horizontalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(BLACK_KING))) & ~blackNotReachableSquares;
        int32_t numAttacks = attacks.popcount();

        if(numAttacks == 0)
            score -= {hceParams.getMGPieceNoMobilityPenalty(QUEEN), hceParams.getEGPieceNoMobilityPenalty(QUEEN)};
        else {
            numAttacks = (16 - std::countl_zero((uint16_t)(numAttacks * numAttacks))) * hceParams.getMGPieceMobilityBonus(QUEEN);
            score -= {numAttacks * hceParams.getMGPieceMobilityBonus(QUEEN), numAttacks * hceParams.getEGPieceMobilityBonus(QUEEN)};
        }
    }

    return score;
}

Score HandcraftedEvaluator::evaluateSafeCenterSpace() {
    Bitboard whiteSafeCenterSquares = ~board.getAttackBitboard(BLACK) & extendedCenter.shiftSouth().extrudeSouth();
    Bitboard blackSafeCenterSquares = ~board.getAttackBitboard(WHITE) & extendedCenter.shiftNorth().extrudeNorth();

    int32_t numWhiteSafeCenterSquares = whiteSafeCenterSquares.popcount();
    int32_t numBlackSafeCenterSquares = blackSafeCenterSquares.popcount();

    Bitboard whitePawns = board.getPieceBitboard(WHITE_PAWN);
    Bitboard blackPawns = board.getPieceBitboard(BLACK_PAWN);

    // Zähle Felder hinter eigenen Bauern doppelt
    numWhiteSafeCenterSquares += (whitePawns.shiftSouth().extrudeSouth() & whiteSafeCenterSquares).popcount();
    numBlackSafeCenterSquares += (blackPawns.shiftNorth().extrudeNorth() & blackSafeCenterSquares).popcount();

    return {(int16_t)(numWhiteSafeCenterSquares - numBlackSafeCenterSquares) * hceParams.getMGSpaceBonus(), 0};
}

Score HandcraftedEvaluator::evaluateMinorPiecesOnStrongSquares() {
    Bitboard whiteKnights = board.getPieceBitboard(WHITE_KNIGHT);
    Bitboard blackKnights = board.getPieceBitboard(BLACK_KNIGHT);
    Bitboard whiteBishops = board.getPieceBitboard(WHITE_BISHOP);
    Bitboard blackBishops = board.getPieceBitboard(BLACK_BISHOP);

    int32_t numWhiteKnightsOnStrongSquares = (whiteKnights & evaluationVars.whiteStrongSquares).popcount();
    int32_t numBlackKnightsOnStrongSquares = (blackKnights & evaluationVars.blackStrongSquares).popcount();
    int32_t numWhiteBishopsOnStrongSquares = (whiteBishops & evaluationVars.whiteStrongSquares).popcount();
    int32_t numBlackBishopsOnStrongSquares = (blackBishops & evaluationVars.blackStrongSquares).popcount();

    return {hceParams.getMGKnightOnStrongSquareBonus() * (numWhiteKnightsOnStrongSquares - numBlackKnightsOnStrongSquares) +
            hceParams.getMGBishopOnStrongSquareBonus() * (numWhiteBishopsOnStrongSquares - numBlackBishopsOnStrongSquares), 0};
}

Score HandcraftedEvaluator::evaluateBadBishops() {
    Bitboard whiteBishops = board.getPieceBitboard(WHITE_BISHOP);
    Bitboard blackBishops = board.getPieceBitboard(BLACK_BISHOP);

    Bitboard whitePawns = board.getPieceBitboard(WHITE_PAWN);
    Bitboard blackPawns = board.getPieceBitboard(BLACK_PAWN);

    Bitboard whitePawnAttacks = board.getAttackBitboard(WHITE_PAWN);
    Bitboard blackPawnAttacks = board.getAttackBitboard(BLACK_PAWN);

    Bitboard whiteBlockedSquares = evaluationVars.whiteImmobilePawns | (blackPawns & blackPawnAttacks);
    Bitboard blackBlockedSquares = evaluationVars.blackImmobilePawns | (whitePawns & whitePawnAttacks);

    Score score = {0, 0};

    while(whiteBishops) {
        int32_t sq = whiteBishops.popFSB();
        score += Score{hceParams.getMGBadBishopPenalty(), hceParams.getEGBadBishopPenalty()} * (badBishopMask[WHITE / COLOR_MASK][sq] & whiteBlockedSquares).popcount();
    }

    while(blackBishops) {
        int32_t sq = blackBishops.popFSB();
        score -= Score{hceParams.getMGBadBishopPenalty(), hceParams.getEGBadBishopPenalty()} * (badBishopMask[BLACK / COLOR_MASK][sq] & blackBlockedSquares).popcount();
    }

    return score;
}

Score HandcraftedEvaluator::evaluateRooksOnOpenFiles() {
    Bitboard whiteRooks = board.getPieceBitboard(WHITE_ROOK);
    Bitboard blackRooks = board.getPieceBitboard(BLACK_ROOK);

    Bitboard whitePawns = board.getPieceBitboard(WHITE_PAWN);
    Bitboard blackPawns = board.getPieceBitboard(BLACK_PAWN);

    Bitboard openFiles = ~(whitePawns | blackPawns).extrudeVertically();
    Bitboard whiteSemiOpenFiles = ~(whitePawns.extrudeVertically() | openFiles);
    Bitboard blackSemiOpenFiles = ~(blackPawns.extrudeVertically() | openFiles);

    int32_t numWhiteRooksOnOpenFiles = (whiteRooks & openFiles).popcount();
    int32_t numBlackRooksOnOpenFiles = (blackRooks & openFiles).popcount();
    int32_t numWhiteRooksOnSemiOpenFiles = (whiteRooks & whiteSemiOpenFiles).popcount();
    int32_t numBlackRooksOnSemiOpenFiles = (blackRooks & blackSemiOpenFiles).popcount();

    return {hceParams.getMGRookOnOpenFileBonus() * (numWhiteRooksOnOpenFiles - numBlackRooksOnOpenFiles) +
            hceParams.getMGRookOnSemiOpenFileBonus() * (numWhiteRooksOnSemiOpenFiles - numBlackRooksOnSemiOpenFiles), 0};
}

Score HandcraftedEvaluator::evaluateRooksBehindPassedPawns() {
    Bitboard whiteRooks = board.getPieceBitboard(WHITE_ROOK);
    Bitboard blackRooks = board.getPieceBitboard(BLACK_ROOK);

    Bitboard whiteLinesBehindPassedPawns = evaluationVars.whitePassedPawns.shiftSouth().extrudeSouth();
    Bitboard blackLinesBehindPassedPawns = evaluationVars.blackPassedPawns.shiftNorth().extrudeNorth();

    Bitboard occupancy = board.getPieceBitboard() | board.getPieceBitboard(WHITE_KING) | board.getPieceBitboard(BLACK_KING);

    Score score = {0, 0};

    Bitboard whiteCandidates = whiteRooks & whiteLinesBehindPassedPawns;
    Bitboard blackCandidates = blackRooks & blackLinesBehindPassedPawns;

    while(whiteCandidates) {
        int32_t sq = whiteCandidates.popFSB();
        score.eg += !(fileFacingEnemy[WHITE / COLOR_MASK][sq] & whiteLinesBehindPassedPawns & occupancy) * hceParams.getEGRookBehindPassedPawnBonus();
    }

    while(blackCandidates) {
        int32_t sq = blackCandidates.popFSB();
        score.eg -= !(fileFacingEnemy[BLACK / COLOR_MASK][sq] & blackLinesBehindPassedPawns & occupancy) * hceParams.getEGRookBehindPassedPawnBonus();
    }

    return score;
}

Score HandcraftedEvaluator::evaluateBlockedPassedPawns() {
    Bitboard whitePassedPawns = evaluationVars.whitePassedPawns;
    Bitboard blackPassedPawns = evaluationVars.blackPassedPawns;

    Bitboard whitePieces = board.getPieceBitboard(WHITE) | board.getPieceBitboard(WHITE_KING);
    Bitboard blackPieces = board.getPieceBitboard(BLACK) | board.getPieceBitboard(BLACK_KING);

    Score score = {0, 0};

    while(whitePassedPawns) {
        int32_t sq = whitePassedPawns.popFSB();
        Bitboard pathToPromotion = fileFacingEnemy[WHITE / COLOR_MASK][sq];

        if(blackPieces & pathToPromotion)
            score.eg -= hceParams.getEGBlockedEnemyPassedPawnBonus();
        else if(whitePieces & pathToPromotion)
            score.eg += hceParams.getEGBlockedOwnPassedPawnPenalty();
    }

    while(blackPassedPawns) {
        int32_t sq = blackPassedPawns.popFSB();
        Bitboard pathToPromotion = fileFacingEnemy[BLACK / COLOR_MASK][sq];

        if(whitePieces & pathToPromotion)
            score.eg += hceParams.getEGBlockedEnemyPassedPawnBonus();
        else if(blackPieces & pathToPromotion)
            score.eg -= hceParams.getEGBlockedOwnPassedPawnPenalty();
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

Score HandcraftedEvaluator::evaluateRuleOfTheSquare() {
    Bitboard pawns = board.getPieceBitboard(WHITE_PAWN) | board.getPieceBitboard(BLACK_PAWN);
    Bitboard allPieces = board.getPieceBitboard();

    if(pawns == allPieces) {
        Score score = {0, 0};

        int32_t sideToMove = board.getSideToMove();

        Bitboard whitePassedPawns = evaluationVars.whitePassedPawns;

        int32_t blackKingSquare = board.getKingSquare(BLACK);
        int32_t blackKingRank = Square::rankOf(blackKingSquare);
        int32_t blackKingFile = Square::fileOf(blackKingSquare);

        while(whitePassedPawns) {
            int32_t sq = whitePassedPawns.popFSB();
            int32_t rank = Square::rankOf(sq);
            rank = std::max(rank, (int32_t)RANK_3); // Bauern auf der 2. Reihe können zwei Felder ziehen
            int32_t file = Square::fileOf(sq);

            if(std::max(std::abs(blackKingRank - rank), std::abs(blackKingFile - file)) - (sideToMove == BLACK) > RANK_8 - rank) {
                score += Score{hceParams.getRuleOfTheSquareBonus(), hceParams.getRuleOfTheSquareBonus()};
                break;
            }
        }

        Bitboard blackPassedPawns = evaluationVars.blackPassedPawns;

        int32_t whiteKingSquare = board.getKingSquare(WHITE);
        int32_t whiteKingRank = Square::rankOf(whiteKingSquare);
        int32_t whiteKingFile = Square::fileOf(whiteKingSquare);

        while(blackPassedPawns) {
            int32_t sq = blackPassedPawns.popFSB();
            int32_t rank = Square::rankOf(sq);
            rank = std::min(rank, (int32_t)RANK_6); // Bauern auf der 7. Reihe können zwei Felder ziehen
            int32_t file = Square::fileOf(sq);

            if(std::max(std::abs(whiteKingRank - rank), std::abs(whiteKingFile - file)) - (sideToMove == WHITE) > rank - RANK_1) {
                score -= Score{hceParams.getRuleOfTheSquareBonus(), hceParams.getRuleOfTheSquareBonus()};
                break;
            }
        }

        return score;
    }

    return {0, 0};
}

bool HandcraftedEvaluator::isWinnable(int32_t side) {
    Bitboard pawns = board.getPieceBitboard(side | PAWN);
    Bitboard minorPieces = board.getPieceBitboard(side | KNIGHT) | board.getPieceBitboard(side | BISHOP);
    Bitboard heavyPieces = board.getPieceBitboard(side | ROOK) | board.getPieceBitboard(side | QUEEN);

    return pawns || minorPieces.popcount() >= 2 || heavyPieces;
}

bool HandcraftedEvaluator::isOppositeColorBishopEndgame() {
    Bitboard pawns = board.getPieceBitboard(WHITE_PAWN) | board.getPieceBitboard(BLACK_PAWN);
    Bitboard whiteBishops = board.getPieceBitboard(WHITE_BISHOP);
    Bitboard blackBishops = board.getPieceBitboard(BLACK_BISHOP);

    return (pawns | whiteBishops | blackBishops) == board.getPieceBitboard() &&
            whiteBishops.popcount() == 1 && blackBishops.popcount() == 1 &&
           (bool)(whiteBishops & lightSquares) != (bool)(blackBishops & lightSquares);
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