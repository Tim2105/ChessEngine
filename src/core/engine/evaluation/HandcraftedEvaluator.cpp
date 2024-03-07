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

int32_t HandcraftedEvaluator::calculateKingSafetyScore() {
    return evaluateKingAttackZone() + evaluatePawnShield() + evaluateOpenFiles() + evaluatePawnStorm();
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
    while(blackKnights) {
        int32_t sq = blackKnights.popFSB();
        Bitboard attacks = knightAttackBitboard(sq) & kingZone;
        if(attacks) {
            numBlackAttackers++;
            blackAttackersWeight += attacks.popcount() * KNIGHT_ATTACK_WEIGHT;
        }
    }

    Bitboard blackBishops = board.getPieceBitboard(BLACK_BISHOP);
    while(blackBishops) {
        int32_t sq = blackBishops.popFSB();
        Bitboard attacks = diagonalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(BLACK_KING)) & kingZone;
        if(attacks) {
            numBlackAttackers++;
            blackAttackersWeight += attacks.popcount() * BISHOP_ATTACK_WEIGHT;
        }
    }

    Bitboard blackRooks = board.getPieceBitboard(BLACK_ROOK);
    while(blackRooks) {
        int32_t sq = blackRooks.popFSB();
        Bitboard attacks = horizontalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(BLACK_KING)) & kingZone;
        if(attacks) {
            numBlackAttackers++;
            blackAttackersWeight += attacks.popcount() * ROOK_ATTACK_WEIGHT;
        }
    }

    Bitboard blackQueens = board.getPieceBitboard(BLACK_QUEEN);
    while(blackQueens) {
        int32_t sq = blackQueens.popFSB();
        Bitboard attacks = (diagonalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(BLACK_KING)) |
                            horizontalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(BLACK_KING))) & kingZone;

        if(attacks) {
            numBlackAttackers++;
            blackAttackersWeight += attacks.popcount() * QUEEN_ATTACK_WEIGHT;
        }
    }

    numBlackAttackers = std::min(numBlackAttackers, (int32_t)(NUM_ATTACKER_WEIGHT_SIZE - 1));

    // Bestimme die Anzahl der Angreifer pro Figurentyp auf die Felder um den schwarzen König
    kingZone = kingAttackZone[blackKingSquare];
    int32_t numWhiteAttackers = 0;
    int32_t whiteAttackersWeight = 0;

    Bitboard whiteKnights = board.getPieceBitboard(WHITE_KNIGHT);
    while(whiteKnights) {
        int32_t sq = whiteKnights.popFSB();
        Bitboard attacks = knightAttackBitboard(sq) & kingZone;
        if(attacks) {
            numWhiteAttackers++;
            whiteAttackersWeight += attacks.popcount() * KNIGHT_ATTACK_WEIGHT;
        }
    }

    Bitboard whiteBishops = board.getPieceBitboard(WHITE_BISHOP);
    while(whiteBishops) {
        int32_t sq = whiteBishops.popFSB();
        Bitboard attacks = diagonalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(WHITE_KING)) & kingZone;
        if(attacks) {
            numWhiteAttackers++;
            whiteAttackersWeight += attacks.popcount() * BISHOP_ATTACK_WEIGHT;
        }
    }

    Bitboard whiteRooks = board.getPieceBitboard(WHITE_ROOK);
    while(whiteRooks) {
        int32_t sq = whiteRooks.popFSB();
        Bitboard attacks = horizontalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(WHITE_KING)) & kingZone;
        if(attacks) {
            numWhiteAttackers++;
            whiteAttackersWeight += attacks.popcount() * ROOK_ATTACK_WEIGHT;
        }
    }

    Bitboard whiteQueens = board.getPieceBitboard(WHITE_QUEEN);
    while(whiteQueens) {
        int32_t sq = whiteQueens.popFSB();
        Bitboard attacks = (diagonalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(WHITE_KING)) |
                            horizontalAttackBitboard(sq, board.getOccupiedBitboard() | board.getPieceBitboard(WHITE_KING))) & kingZone;
                        
        if(attacks) {
            numWhiteAttackers++;
            whiteAttackersWeight += attacks.popcount() * QUEEN_ATTACK_WEIGHT;
        }
    }

    numWhiteAttackers = std::min(numWhiteAttackers, (int32_t)(NUM_ATTACKER_WEIGHT_SIZE - 1));

    // Berechne die Bewertung
    score += whiteAttackersWeight * NUM_ATTACKER_WEIGHT[numWhiteAttackers] / 100;
    score -= blackAttackersWeight * NUM_ATTACKER_WEIGHT[numBlackAttackers] / 100;

    return score * (1 - evaluationVars.phase);
}

int32_t HandcraftedEvaluator::evaluatePawnShield() {
    int32_t score = 0;

    int32_t whiteKingSquare = board.getKingSquare(WHITE);
    int32_t blackKingSquare = board.getKingSquare(BLACK);

    Bitboard whitePawns = board.getPieceBitboard(WHITE_PAWN);
    Bitboard blackPawns = board.getPieceBitboard(BLACK_PAWN);

    Bitboard whitePawnShieldMask = pawnShieldMask[WHITE / COLOR_MASK][whiteKingSquare];
    Bitboard blackPawnShieldMask = pawnShieldMask[BLACK / COLOR_MASK][blackKingSquare];

    Bitboard whitePawnShield = whitePawns & whitePawnShieldMask;
    Bitboard blackPawnShield = blackPawns & blackPawnShieldMask;

    int32_t whitePawnShieldSize = whitePawnShield.popcount();
    int32_t blackPawnShieldSize = blackPawnShield.popcount();

    whitePawnShieldSize = std::min(whitePawnShieldSize, (int32_t)(PAWN_SHIELD_SIZE_BONUS_SIZE - 1));
    blackPawnShieldSize = std::min(blackPawnShieldSize, (int32_t)(PAWN_SHIELD_SIZE_BONUS_SIZE - 1));

    score += PAWN_SHIELD_SIZE_BONUS[whitePawnShieldSize];
    score -= PAWN_SHIELD_SIZE_BONUS[blackPawnShieldSize];

    return score * (1 - evaluationVars.phase);
}

int32_t HandcraftedEvaluator::evaluateOpenFiles() {
    int32_t score = 0;

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

    score += KING_OPEN_FILE_BONUS[whiteOpenFiles];
    score -= KING_OPEN_FILE_BONUS[blackOpenFiles];

    return score * (1 - evaluationVars.phase);
}

int32_t HandcraftedEvaluator::evaluatePawnStorm() {
    int32_t score = 0;

    int32_t whiteKingSquare = board.getKingSquare(WHITE);
    int32_t blackKingSquare = board.getKingSquare(BLACK);

    Bitboard whitePawns = board.getPieceBitboard(WHITE_PAWN);
    Bitboard blackPawns = board.getPieceBitboard(BLACK_PAWN);

    Bitboard whitePawnStorm = pawnStormMask[WHITE / COLOR_MASK][whiteKingSquare] & blackPawns;
    Bitboard blackPawnStorm = pawnStormMask[BLACK / COLOR_MASK][blackKingSquare] & whitePawns;

    while(whitePawnStorm) {
        int32_t sq = whitePawnStorm.popFSB();
        score += PAWN_STORM_BONUS[Square::rankOf(sq)];
    }

    while(blackPawnStorm) {
        int32_t sq = blackPawnStorm.popFSB();
        score -= PAWN_STORM_BONUS[Square::rankOf(Square::flipY(sq))];
    }

    return score * (1 - evaluationVars.phase);
}

int32_t HandcraftedEvaluator::evaluateKNBKEndgame(int32_t b, int32_t k) {
    int32_t evaluation = std::abs(evaluationVars.materialScore.eg);

    b = -1879048192 * b >> 31;
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