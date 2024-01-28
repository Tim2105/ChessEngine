#include "core/engine/evaluation/SimpleUpdatedEvaluator.h"

void SimpleUpdatedEvaluator::updateBeforeMove(Move m) {
    evaluationHistory.push_back({currentScore, currentGamePhase, currentPhaseWeight});

    Score delta{0, 0};

    int32_t side = board.getSideToMove();
    int32_t piece = TYPEOF(board.pieceAt(m.getOrigin()));
    int32_t originPSQT = m.getOrigin();
    int32_t destinationPSQT = m.getDestination();

    // Für schwarze Figuren muss der Rang gespiegelt werden,
    // um die Positionstabelle zu indizieren
    if(side == BLACK) {
        int32_t rank = SQ2R(originPSQT);
        int32_t file = SQ2F(originPSQT);
        originPSQT = FR2SQ(file, 7 - rank);

        rank = SQ2R(destinationPSQT);
        file = SQ2F(destinationPSQT);
        destinationPSQT = FR2SQ(file, 7 - rank);
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
        int32_t rank = SQ2R(capturedPieceSquare);
        int32_t file = SQ2F(capturedPieceSquare);
        capturedPieceSquare = FR2SQ(file, 7 - rank);

        delta.mg += MG_PIECE_VALUE[TYPEOF(capturedPiece)];
        delta.eg += EG_PIECE_VALUE[TYPEOF(capturedPiece)];

        delta.mg += MG_PSQT[TYPEOF(capturedPiece)][capturedPieceSquare];
        delta.eg += EG_PSQT[TYPEOF(capturedPiece)][capturedPieceSquare];

        // Aktualisiere die Phasengewichte
        currentPhaseWeight += PIECE_WEIGHT[TYPEOF(capturedPiece)];
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
        currentPhaseWeight -= PIECE_WEIGHT[promotedPiece] - PIECE_WEIGHT[PAWN];
    }

    currentScore += delta * (side == WHITE ? 1 : -1);

    currentGamePhase = currentPhaseWeight / (double)TOTAL_WEIGHT;
    currentGamePhase = currentGamePhase * (MAX_PHASE - MIN_PHASE) + MIN_PHASE; // phase in [MIN_PHASE, MAX_PHASE]
    currentGamePhase = std::clamp(currentGamePhase, 0.0, 1.0); // phase auf [0, 1] begrenzen
}

void SimpleUpdatedEvaluator::updateBeforeUndo() {
    EvaluationVariables scorePhasePair = evaluationHistory.pop_back();
    currentScore = scorePhasePair.score;
    currentGamePhase = scorePhasePair.phase;
    currentPhaseWeight = scorePhasePair.phaseWeight;
}

void SimpleUpdatedEvaluator::calculateScore() {
    Score score{0, 0};

    // Material

    for(int32_t piece = PAWN; piece <= KING; piece++) {
        score.mg += board.getPieceBitboard(WHITE | piece).getNumberOfSetBits() * MG_PIECE_VALUE[piece];
        score.eg += board.getPieceBitboard(WHITE | piece).getNumberOfSetBits() * EG_PIECE_VALUE[piece];
        score.mg -= board.getPieceBitboard(BLACK | piece).getNumberOfSetBits() * MG_PIECE_VALUE[piece];
        score.eg -= board.getPieceBitboard(BLACK | piece).getNumberOfSetBits() * EG_PIECE_VALUE[piece];
    }

    // Positionstabellen
    for(int32_t piece = PAWN; piece <= KING; piece++) {
        Bitboard pieceBB = board.getPieceBitboard(WHITE | piece);
        while(pieceBB) {
            int32_t square = pieceBB.popFirstSetBit();
            score.mg += MG_PSQT[piece][square];
            score.eg += EG_PSQT[piece][square];
        }

        pieceBB = board.getPieceBitboard(BLACK | piece);
        while(pieceBB) {
            int32_t square = pieceBB.popFirstSetBit();

            // Für schwarze Figuren muss der Rang gespiegelt werden
            int32_t rank = SQ2R(square);
            int32_t file = SQ2F(square);
            square = FR2SQ(file, 7 - rank);

            score.mg -= MG_PSQT[piece][square];
            score.eg -= EG_PSQT[piece][square];
        }
    }

    currentScore = score;
}

void SimpleUpdatedEvaluator::calculateGamePhase() {
    currentPhaseWeight = TOTAL_WEIGHT;

    currentPhaseWeight -= board.getPieceBitboard(WHITE_PAWN).getNumberOfSetBits() * PAWN_WEIGHT;
    currentPhaseWeight -= board.getPieceBitboard(BLACK_PAWN).getNumberOfSetBits() * PAWN_WEIGHT;
    currentPhaseWeight -= board.getPieceBitboard(WHITE_KNIGHT).getNumberOfSetBits() * KNIGHT_WEIGHT;
    currentPhaseWeight -= board.getPieceBitboard(BLACK_KNIGHT).getNumberOfSetBits() * KNIGHT_WEIGHT;
    currentPhaseWeight -= board.getPieceBitboard(WHITE_BISHOP).getNumberOfSetBits() * BISHOP_WEIGHT;
    currentPhaseWeight -= board.getPieceBitboard(BLACK_BISHOP).getNumberOfSetBits() * BISHOP_WEIGHT;
    currentPhaseWeight -= board.getPieceBitboard(WHITE_ROOK).getNumberOfSetBits() * ROOK_WEIGHT;
    currentPhaseWeight -= board.getPieceBitboard(BLACK_ROOK).getNumberOfSetBits() * ROOK_WEIGHT;
    currentPhaseWeight -= board.getPieceBitboard(WHITE_QUEEN).getNumberOfSetBits() * QUEEN_WEIGHT;
    currentPhaseWeight -= board.getPieceBitboard(BLACK_QUEEN).getNumberOfSetBits() * QUEEN_WEIGHT;

    currentGamePhase = currentPhaseWeight / (double)TOTAL_WEIGHT;
    currentGamePhase = currentGamePhase * (MAX_PHASE - MIN_PHASE) + MIN_PHASE; // phase in [MIN_PHASE, MAX_PHASE]
    currentGamePhase = std::clamp(currentGamePhase, 0.0, 1.0); // phase auf [0, 1] begrenzen
}