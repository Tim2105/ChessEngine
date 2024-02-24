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

    delta.mg += MG_PSQT[piece][destinationPSQT] - MG_PSQT[piece][originPSQT];
    delta.eg += EG_PSQT[piece][destinationPSQT] - EG_PSQT[piece][originPSQT];

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

        delta.mg += MG_PIECE_VALUE[TYPEOF(capturedPiece)];
        delta.eg += EG_PIECE_VALUE[TYPEOF(capturedPiece)];

        delta.mg += MG_PSQT[TYPEOF(capturedPiece)][capturedPieceSquare];
        delta.eg += EG_PSQT[TYPEOF(capturedPiece)][capturedPieceSquare];

        // Aktualisiere die Phasengewichte
        evaluationVars.phaseWeight += PIECE_WEIGHT[TYPEOF(capturedPiece)];
    }

    // Spezialfall: Rochade
    if(m.isCastle()) {
        if(m.isKingsideCastle()) {
            // Königsseite
            delta.mg += MG_PSQT[ROOK][F1] - MG_PSQT[ROOK][H1];
            delta.eg += EG_PSQT[ROOK][F1] - EG_PSQT[ROOK][H1];
        } else {
            // Damenseite
            delta.mg += MG_PSQT[ROOK][D1] - MG_PSQT[ROOK][A1];
            delta.eg += EG_PSQT[ROOK][D1] - EG_PSQT[ROOK][A1];
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

        delta.mg += MG_PIECE_VALUE[promotedPiece] - MG_PIECE_VALUE[PAWN];
        delta.eg += EG_PIECE_VALUE[promotedPiece] - EG_PIECE_VALUE[PAWN];

        delta.mg += MG_PSQT[promotedPiece][destinationPSQT] - MG_PSQT[PAWN][destinationPSQT];
        delta.eg += EG_PSQT[promotedPiece][destinationPSQT] - EG_PSQT[PAWN][destinationPSQT];

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
    for(int32_t piece = PAWN; piece <= KING; piece++) {
        score.mg += board.getPieceBitboard(WHITE | piece).popcount() * MG_PIECE_VALUE[piece];
        score.eg += board.getPieceBitboard(WHITE | piece).popcount() * EG_PIECE_VALUE[piece];
        score.mg -= board.getPieceBitboard(BLACK | piece).popcount() * MG_PIECE_VALUE[piece];
        score.eg -= board.getPieceBitboard(BLACK | piece).popcount() * EG_PIECE_VALUE[piece];
    }

    // Positionstabellen
    for(int32_t piece = PAWN; piece <= KING; piece++) {
        Bitboard pieceBB = board.getPieceBitboard(WHITE | piece);
        while(pieceBB) {
            int32_t square = pieceBB.popFSB();
            score.mg += MG_PSQT[piece][square];
            score.eg += EG_PSQT[piece][square];
        }

        pieceBB = board.getPieceBitboard(BLACK | piece);
        while(pieceBB) {
            int32_t square = pieceBB.popFSB();

            // Für schwarze Figuren muss der Rang gespiegelt werden
            square = Square::flipY(square);

            score.mg -= MG_PSQT[piece][square];
            score.eg -= EG_PSQT[piece][square];
        }
    }

    evaluationVars.materialScore = score;
}

void HandcraftedEvaluator::calculatePawnScore() {
    Score score{0, 0};

    Bitboard whitePawns = board.getPieceBitboard(WHITE_PAWN);
    Bitboard blackPawns = board.getPieceBitboard(BLACK_PAWN);

    Bitboard doubleWhitePawns = 0, doubleBlackPawns = 0;
    Bitboard isolatedWhitePawns = 0, isolatedBlackPawns = 0;
    Bitboard whitePassedPawns = 0, blackPassedPawns = 0;

    Bitboard tmp = whitePawns;
    while(tmp) {
        int32_t sq = tmp.popFSB();

        // Doppelbauern
        doubleWhitePawns |= whitePawns & fileFacingEnemy[WHITE / COLOR_MASK][sq];
        
        // Isolierte Bauern
        int32_t file = SQ2F(sq);
        if(!(whitePawns & neighboringFiles[file]))
            isolatedWhitePawns.setBit(sq);

        // Freibauern
        if(!(sentryMasks[WHITE / COLOR_MASK][sq] & blackPawns))
            whitePassedPawns.setBit(sq);
    }

    tmp = blackPawns;
    while(tmp) {
        int32_t sq = tmp.popFSB();

        // Doppelbauern
        doubleBlackPawns |= blackPawns & fileFacingEnemy[BLACK / COLOR_MASK][sq];

        // Isolierte Bauern
        int32_t file = SQ2F(sq);
        if(!(blackPawns & neighboringFiles[file]))
            isolatedBlackPawns.setBit(sq);

        // Freibauern
        if(!(sentryMasks[BLACK / COLOR_MASK][sq] & whitePawns))
            blackPassedPawns.setBit(sq);
    }

    // Doppelbauern
    while(doubleWhitePawns) {
        int32_t file = SQ2F(doubleWhitePawns.popFSB());
        score.mg += MG_DOUBLED_PAWN_PENALTY[file];
        score.eg += EG_DOUBLED_PAWN_PENALTY[file];
    }

    while(doubleBlackPawns) {
        int32_t file = SQ2F(doubleBlackPawns.popFSB());
        score.mg -= MG_DOUBLED_PAWN_PENALTY[file];
        score.eg -= EG_DOUBLED_PAWN_PENALTY[file];
    }

    // Isolierte Bauern
    while(isolatedWhitePawns) {
        int32_t file = SQ2F(isolatedWhitePawns.popFSB());
        score.mg += MG_ISOLATED_PAWN_PENALTY[file];
        score.eg += EG_ISOLATED_PAWN_PENALTY[file];
    }

    while(isolatedBlackPawns) {
        int32_t file = SQ2F(isolatedBlackPawns.popFSB());
        score.mg -= MG_ISOLATED_PAWN_PENALTY[file];
        score.eg -= EG_ISOLATED_PAWN_PENALTY[file];
    }

    // Freibauern
    while(whitePassedPawns) {
        int32_t rank = SQ2R(whitePassedPawns.popFSB());
        score.mg += MG_PASSED_PAWN_BONUS[rank];
        score.eg += EG_PASSED_PAWN_BONUS[rank];
    }

    while(blackPassedPawns) {
        int32_t rank = SQ2R(blackPassedPawns.popFSB());
        score.mg -= MG_PASSED_PAWN_BONUS[rank];
        score.eg -= EG_PASSED_PAWN_BONUS[rank];
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