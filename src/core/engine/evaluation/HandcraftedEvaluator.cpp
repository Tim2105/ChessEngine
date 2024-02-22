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

void HandcraftedEvaluator::updateBeforeUndo() {
    evaluationVars = evaluationHistory.back();
    evaluationHistory.pop_back();
}

void HandcraftedEvaluator::calculateScore() {
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