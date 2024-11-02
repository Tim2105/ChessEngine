#include "core/engine/evaluation/HandcraftedEvaluator.h"
#include "core/utils/magics/Magics.h"

#include <cmath>

void HandcraftedEvaluator::updateBeforeMove(Move m) {
    evaluationHistory.push_back(evaluationVars);

    Score psqtDelta{0, 0};
    int pieceValueDelta = 0;

    int side = board.getSideToMove();
    int otherSide = side ^ COLOR_MASK;
    int piece = TYPEOF(board.pieceAt(m.getOrigin()));
    int origin = m.getOrigin();
    int destination = m.getDestination();

    // Für schwarze Figuren muss der Rang gespiegelt werden,
    // um die Positionstabelle zu indizieren
    if(side == BLACK) {
        origin = Square::flipY(origin);
        destination = Square::flipY(destination);
    }

    psqtDelta.mg += hceParams.getMGPSQT(piece, destination) - hceParams.getMGPSQT(piece, origin);
    psqtDelta.eg += hceParams.getEGPSQT(piece, destination) - hceParams.getEGPSQT(piece, origin);

    int capturedPieceType = EMPTY;

    // Spezialfall: Schlagzug
    if(m.isCapture()) {
        int capturedPiece = board.pieceAt(m.getDestination());
        int capturedPieceSquare = destination;

        // Spezialfall: En-Passant
        if(m.isEnPassant()) {
            capturedPiece = otherSide | PAWN;
            capturedPieceSquare += SOUTH;
        }

        capturedPieceType = TYPEOF(capturedPiece);

        // Für geschlagene Figuren muss der Rang gespiegelt werden,
        // um die Positionstabelle zu indizieren
        capturedPieceSquare = Square::flipY(capturedPieceSquare);

        // Aktualisiere die linearen und quadratischen Terme
        pieceValueDelta += hceParams.getLinearPieceValue(capturedPieceType);

        int numCapPiece = board.getPieceBitboard(capturedPiece).popcount();
        pieceValueDelta += hceParams.getQuadraticPieceValue(capturedPieceType) * (numCapPiece * numCapPiece - (numCapPiece - 1) * (numCapPiece - 1));

        // Aktualisiere die gemischten Terme
        for(int piece = PAWN; piece <= QUEEN; piece++) {
            if(piece == capturedPieceType)
                continue;

            int crossedPieceValue = hceParams.getCrossedPieceValue(piece, capturedPieceType);
            pieceValueDelta += crossedPieceValue * board.getPieceBitboard(otherSide | piece).popcount();

            int pieceImbalanceValue = hceParams.getPieceImbalanceValue(piece, capturedPieceType);
            if(piece > capturedPieceType)
                pieceImbalanceValue = -pieceImbalanceValue;

            pieceValueDelta += pieceImbalanceValue * board.getPieceBitboard(side | piece).popcount();
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
        int promotedPiece = KNIGHT;

        if(m.isPromotionQueen())
            promotedPiece = QUEEN;
        else if(m.isPromotionRook())
            promotedPiece = ROOK;
        else if(m.isPromotionBishop())
            promotedPiece = BISHOP;

        // Aktualisiere die linearen Terme
        pieceValueDelta += hceParams.getLinearPieceValue(promotedPiece) - hceParams.getLinearPieceValue(PAWN);

        // Aktualisiere die quadratischen Terme
        int numPromotedPiece = board.getPieceBitboard(side | promotedPiece).popcount();
        int numPawns = board.getPieceBitboard(side | PAWN).popcount();
        int quadraticPromotedPieceValue = hceParams.getQuadraticPieceValue(promotedPiece);
        int quadraticPawnValue = hceParams.getQuadraticPieceValue(PAWN);
        pieceValueDelta += quadraticPromotedPieceValue * ((numPromotedPiece + 1) * (numPromotedPiece + 1) - numPromotedPiece * numPromotedPiece) -
                           quadraticPawnValue * (numPawns * numPawns - (numPawns - 1) * (numPawns - 1));

        // Aktualisiere die gemischten Terme für alle Paare von Figuren
        // mit Ausnahme des Bauerns und der aufgewerteten Figur
        for(int piece = KNIGHT; piece <= QUEEN; piece++) {
            int numPieces = board.getPieceBitboard(side | piece).popcount();
            int numOpponentPieces = board.getPieceBitboard(otherSide | piece).popcount() - (piece == capturedPieceType);
            if(piece != promotedPiece) {
                int crossedPieceValue = hceParams.getCrossedPieceValue(piece, promotedPiece);
                pieceValueDelta += crossedPieceValue * numPieces;

                int crossedPawnValue = hceParams.getCrossedPieceValue(piece, PAWN);
                pieceValueDelta -= crossedPawnValue * numPieces;

                int pieceImbalanceValue = hceParams.getPieceImbalanceValue(piece, promotedPiece);
                if(piece > promotedPiece)
                    pieceImbalanceValue = -pieceImbalanceValue;

                pieceValueDelta += pieceImbalanceValue * numOpponentPieces;
            }

            int pawnImbalanceValue = hceParams.getPieceImbalanceValue(PAWN, piece);
            pieceValueDelta += pawnImbalanceValue * numOpponentPieces;
        }

        // Aktualisiere die gemischten Terme für das Paar Bauer und aufgewertete Figur
        int crossedPawnValue = hceParams.getCrossedPieceValue(PAWN, promotedPiece);
        pieceValueDelta -= crossedPawnValue * numPawns * numPromotedPiece;
        pieceValueDelta += crossedPawnValue * (numPawns - 1) * (numPromotedPiece + 1);

        int numOpponentPawns = board.getPieceBitboard(otherSide | PAWN).popcount();
        int pawnImbalanceValue = hceParams.getPieceImbalanceValue(PAWN, promotedPiece);
        pieceValueDelta += pawnImbalanceValue * numOpponentPawns;

        psqtDelta.mg += hceParams.getMGPSQT(promotedPiece, destination) - hceParams.getMGPSQT(PAWN, destination);
        psqtDelta.eg += hceParams.getEGPSQT(promotedPiece, destination) - hceParams.getEGPSQT(PAWN, destination);

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

    int movedPiece = TYPEOF(board.pieceAt(m.getDestination()));
    int capturedPiece = TYPEOF(board.getLastMoveHistoryEntry().capturedPiece);

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
    int pieceScore = 0;

    // Material
    for(int piece = PAWN; piece <= QUEEN; piece++) {
        int numWhitePieces = board.getPieceBitboard(WHITE | piece).popcount();
        int numBlackPieces = board.getPieceBitboard(BLACK | piece).popcount();

        // lineare Terme
        pieceScore += hceParams.getLinearPieceValue(piece) * (numWhitePieces - numBlackPieces);

        // quadratische Terme
        pieceScore += hceParams.getQuadraticPieceValue(piece) * (numWhitePieces * numWhitePieces - numBlackPieces * numBlackPieces);

        // gemischte Terme
        for(int otherPiece = PAWN; otherPiece < piece; otherPiece++) {
            int crossedPieceValue = hceParams.getCrossedPieceValue(piece, otherPiece);
            pieceScore += crossedPieceValue * (numWhitePieces * board.getPieceBitboard(WHITE | otherPiece).popcount() -
                                               numBlackPieces * board.getPieceBitboard(BLACK | otherPiece).popcount());

            // Materialungleichgewicht
            int imbalanceValue = hceParams.getPieceImbalanceValue(piece, otherPiece);
            pieceScore += imbalanceValue * (numWhitePieces * board.getPieceBitboard(BLACK | otherPiece).popcount() -
                                            numBlackPieces * board.getPieceBitboard(WHITE | otherPiece).popcount());
        }
    }

    // Positionstabellen
    for(int piece = PAWN; piece <= KING; piece++) {
        Bitboard pieceBB = board.getPieceBitboard(WHITE | piece);
        while(pieceBB) {
            int square = pieceBB.popFSB();
            psqtScore.mg += hceParams.getMGPSQT(piece, square);
            psqtScore.eg += hceParams.getEGPSQT(piece, square);
        }

        pieceBB = board.getPieceBitboard(BLACK | piece);
        while(pieceBB) {
            int square = pieceBB.popFSB();

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

    // Isolierte Bauern
    Bitboard isolatedWhitePawns = ~whitePawnsWestEast.extrudeVertically() & whitePawns;
    Bitboard isolatedBlackPawns = ~blackPawnsWestEast.extrudeVertically() & blackPawns;
    score.mg += hceParams.getMGIsolatedPawnPenalty() * (isolatedWhitePawns.popcount() - isolatedBlackPawns.popcount());
    score.eg += hceParams.getEGIsolatedPawnPenalty() * (isolatedWhitePawns.popcount() - isolatedBlackPawns.popcount());

    // Rückständige Bauern
    Bitboard backwardWhitePawns = whitePawns & ~whitePawnsWestEast & whitePawnsWestEast.extrudeSouth() & (blackPawns | blackPawnAttacks).shiftSouth() & ~whitePawnAttacks.extrudeNorth();
    Bitboard backwardBlackPawns = blackPawns & ~blackPawnsWestEast & blackPawnsWestEast.extrudeNorth() & (whitePawns | whitePawnAttacks).shiftNorth() & ~blackPawnAttacks.extrudeSouth();
    evaluationVars.whiteBackwardPawns = backwardWhitePawns;
    evaluationVars.blackBackwardPawns = backwardBlackPawns;
    score.mg += hceParams.getMGBackwardPawnPenalty() * (backwardWhitePawns.popcount() - backwardBlackPawns.popcount());
    score.eg += hceParams.getEGBackwardPawnPenalty() * (backwardWhitePawns.popcount() - backwardBlackPawns.popcount());

    // Verbundene Bauern
    Bitboard connectedWhitePawns = (whitePawnsWestEast | whitePawnsWestEast.shiftNorth() | whitePawnsWestEast.shiftSouth()) & whitePawns;
    Bitboard temp = connectedWhitePawns;
    while(temp) {
        int rank = Square::rankOf(temp.popFSB());
        score.mg += hceParams.getMGConnectedPawnBonus(rank);
        score.eg += hceParams.getEGConnectedPawnBonus(rank);
    }

    Bitboard connectedBlackPawns = (blackPawnsWestEast | blackPawnsWestEast.shiftSouth() | blackPawnsWestEast.shiftNorth()) & blackPawns;
    temp = connectedBlackPawns;
    while(temp) {
        int rank = Square::rankOf(Square::flipY(temp.popFSB()));
        score.mg -= hceParams.getMGConnectedPawnBonus(rank);
        score.eg -= hceParams.getEGConnectedPawnBonus(rank);
    }

    // Freibauern
    Bitboard whitePassedPawns = whitePawns & ~doubledWhitePawns & ~((blackPawns | blackPawnAttacks).extrudeSouth());
    evaluationVars.whitePassedPawns = whitePassedPawns;
    while(whitePassedPawns) {
        int rank = Square::rankOf(whitePassedPawns.popFSB());
        score.mg += hceParams.getMGPassedPawnBonus(rank);
        score.eg += hceParams.getEGPassedPawnBonus(rank);
    }

    Bitboard blackPassedPawns = blackPawns & ~doubledBlackPawns & ~((whitePawns | whitePawnAttacks).extrudeNorth());
    evaluationVars.blackPassedPawns = blackPassedPawns;
    while(blackPassedPawns) {
        int rank = Square::rankOf(Square::flipY(blackPassedPawns.popFSB()));
        score.mg -= hceParams.getMGPassedPawnBonus(rank);
        score.eg -= hceParams.getEGPassedPawnBonus(rank);
    }

    // Verbundene Freibauern
    Bitboard connectedWhitePassedPawns = evaluationVars.whitePassedPawns & connectedWhitePawns;
    while(connectedWhitePassedPawns) {
        int rank = Square::rankOf(connectedWhitePassedPawns.popFSB());
        score.mg -= hceParams.getMGConnectedPassedPawnBonus(rank);
        score.eg -= hceParams.getEGConnectedPassedPawnBonus(rank);
    }

    Bitboard connectedBlackPassedPawns = evaluationVars.blackPassedPawns & connectedBlackPawns;
    while(connectedBlackPassedPawns) {
        int rank = Square::rankOf(connectedBlackPassedPawns.popFSB());
        score.mg += hceParams.getMGConnectedPassedPawnBonus(rank);
        score.eg += hceParams.getEGConnectedPassedPawnBonus(rank);
    }

    // Freibauerkandidaten
    evaluationVars.whiteCandidatePassedPawns = 0ULL;

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
            evaluationVars.whiteCandidatePassedPawns |= sq;
            int rank = Square::rankOf(sq.getFSB());
            score += Score{hceParams.getMGCandidatePassedPawnBonus(rank), hceParams.getEGCandidatePassedPawnBonus(rank)};
        }
    }

    evaluationVars.blackCandidatePassedPawns = 0ULL;

    while(blackCandidatePassedPawns) {
        Bitboard sq = 1ULL << blackCandidatePassedPawns.popFSB();
        Bitboard pathToPromotion = sq.extrudeSouth();
        Bitboard whiteOpposition = (pathToPromotion.shiftSouthWest() | pathToPromotion.shiftSouthEast()) & whitePawns;
        Bitboard blackSupport = whiteOpposition.shiftNorth(2).extrudeNorth() & blackSupportCandidates & ~((sq.shiftWest() | sq.shiftEast()).extrudeNorth() & whitePawns).extrudeNorth();

        if(blackSupport.popcount() >= whiteOpposition.popcount()) {
            evaluationVars.blackCandidatePassedPawns |= sq;
            int rank = Square::rankOf(Square::flipY(sq.getFSB()));
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
    int kingSafety = evaluateKingAttackZone();
    int kingMGSafety = evaluateOpenFiles() + evaluatePawnSafety();

    return {kingSafety + kingMGSafety, kingSafety};
}

int HandcraftedEvaluator::evaluateKingAttackZone() {
    int score = 0;

    int whiteKingSquare = board.getKingSquare(WHITE);
    int blackKingSquare = board.getKingSquare(BLACK);

    Bitboard whitePawns = board.getPieceBitboard(WHITE_PAWN);
    Bitboard blackPawns = board.getPieceBitboard(BLACK_PAWN);

    // Bestimme die Anzahl der Angreifer pro Figurentyp auf die Felder um den weißen König
    Bitboard kingZone = kingAttackZone[whiteKingSquare];
    int numBlackAttackers = 0;
    int blackAttackersWeight = 0;

    Bitboard blackKnights = board.getPieceBitboard(BLACK_KNIGHT);
    while(blackKnights) {
        int sq = blackKnights.popFSB();
        Bitboard attacks = knightAttackBitboard(sq) & kingZone;
        if(attacks) {
            numBlackAttackers++;
            blackAttackersWeight += attacks.popcount() * hceParams.getKnightAttackBonus();
        }
    }

    Bitboard blackBishops = board.getPieceBitboard(BLACK_BISHOP);
    // Ignoriere Damen, damit Diagonalangriffe in denen eine Dame von einem Läufer gedeckt wird, doppelt gezählt werden
    Bitboard occupied = (board.getPieceBitboard() | board.getPieceBitboard(BLACK_KING)) & ~board.getPieceBitboard(BLACK_QUEEN);
    while(blackBishops) {
        int sq = blackBishops.popFSB();
        Bitboard attacks = diagonalAttackBitboard(sq, occupied) & kingZone;
        if(attacks) {
            numBlackAttackers++;
            blackAttackersWeight += attacks.popcount() * hceParams.getBishopAttackBonus();
        }
    }

    Bitboard blackRooks = board.getPieceBitboard(BLACK_ROOK);
    // Ignoriere Damen und Türme, damit Horizontalangriffe in denen eine Dame oder ein Turm von einem Turm gedeckt wird, doppelt gezählt werden
    occupied = (board.getPieceBitboard() | board.getPieceBitboard(BLACK_KING)) & ~(board.getPieceBitboard(BLACK_QUEEN) | board.getPieceBitboard(BLACK_ROOK));
    while(blackRooks) {
        int sq = blackRooks.popFSB();
        Bitboard attacks = horizontalAttackBitboard(sq, occupied) & kingZone;
        if(attacks) {
            numBlackAttackers++;
            blackAttackersWeight += attacks.popcount() * hceParams.getRookAttackBonus();
        }
    }

    Bitboard blackQueens = board.getPieceBitboard(BLACK_QUEEN);
    // Ignoriere Damen und Türme (nur in der Horizontalrichtung), damit Horizontalangriffe in denen ein Turm von einer Dame gedeckt wird, doppelt gezählt werden
    occupied = (board.getPieceBitboard() | board.getPieceBitboard(BLACK_KING)) & ~(board.getPieceBitboard(BLACK_QUEEN) | board.getPieceBitboard(BLACK_ROOK));
    Bitboard occupiedDiagonal = (board.getPieceBitboard() | board.getPieceBitboard(BLACK_KING)) & ~board.getPieceBitboard(BLACK_QUEEN);
    while(blackQueens) {
        int sq = blackQueens.popFSB();
        Bitboard attacks = (diagonalAttackBitboard(sq, occupiedDiagonal) |
                            horizontalAttackBitboard(sq, occupied)) & kingZone;

        if(attacks) {
            numBlackAttackers++;
            blackAttackersWeight += attacks.popcount() * hceParams.getQueenAttackBonus();
        }
    }

    numBlackAttackers = std::min(numBlackAttackers, 5);

    // Bestimme die Anzahl der Angreifer pro Figurentyp auf die Felder um den schwarzen König
    kingZone = kingAttackZone[blackKingSquare];
    int numWhiteAttackers = 0;
    int whiteAttackersWeight = 0;

    Bitboard whiteKnights = board.getPieceBitboard(WHITE_KNIGHT);
    while(whiteKnights) {
        int sq = whiteKnights.popFSB();
        Bitboard attacks = knightAttackBitboard(sq) & kingZone;
        if(attacks) {
            numWhiteAttackers++;
            whiteAttackersWeight += attacks.popcount() * hceParams.getKnightAttackBonus();
        }
    }

    Bitboard whiteBishops = board.getPieceBitboard(WHITE_BISHOP);
    // Ignoriere Damen, damit Diagonalangriffe in denen eine Dame von einem Läufer gedeckt wird, doppelt gezählt werden
    occupied = (board.getPieceBitboard() | board.getPieceBitboard(WHITE_KING)) & ~board.getPieceBitboard(WHITE_QUEEN);
    while(whiteBishops) {
        int sq = whiteBishops.popFSB();
        Bitboard attacks = diagonalAttackBitboard(sq, occupied) & kingZone;
        if(attacks) {
            numWhiteAttackers++;
            whiteAttackersWeight += attacks.popcount() * hceParams.getBishopAttackBonus();
        }
    }

    Bitboard whiteRooks = board.getPieceBitboard(WHITE_ROOK);
    // Ignoriere Damen und Türme, damit Horizontalangriffe in denen eine Dame oder ein Turm von einem Turm gedeckt wird, doppelt gezählt werden
    occupied = (board.getPieceBitboard() | board.getPieceBitboard(WHITE_KING)) & ~(board.getPieceBitboard(WHITE_QUEEN) | board.getPieceBitboard(WHITE_ROOK));
    while(whiteRooks) {
        int sq = whiteRooks.popFSB();
        Bitboard attacks = horizontalAttackBitboard(sq, occupied) & kingZone;
        if(attacks) {
            numWhiteAttackers++;
            whiteAttackersWeight += attacks.popcount() * hceParams.getRookAttackBonus();
        }
    }

    Bitboard whiteQueens = board.getPieceBitboard(WHITE_QUEEN);
    // Ignoriere Damen und Türme (nur in der Horizontalrichtung), damit Horizontalangriffe in denen ein Turm von einer Dame gedeckt wird, doppelt gezählt werden
    occupied = (board.getPieceBitboard() | board.getPieceBitboard(WHITE_KING)) & ~(board.getPieceBitboard(WHITE_QUEEN) | board.getPieceBitboard(WHITE_ROOK));
    occupiedDiagonal = (board.getPieceBitboard() | board.getPieceBitboard(WHITE_KING)) & ~board.getPieceBitboard(WHITE_QUEEN);
    while(whiteQueens) {
        int sq = whiteQueens.popFSB();
        Bitboard attacks = (diagonalAttackBitboard(sq, occupiedDiagonal) |
                            horizontalAttackBitboard(sq, occupied)) & kingZone;
                        
        if(attacks) {
            numWhiteAttackers++;
            whiteAttackersWeight += attacks.popcount() * hceParams.getQueenAttackBonus();
        }
    }

    numWhiteAttackers = std::min(numWhiteAttackers, 5);

    // Berechne die Bewertung
    // Typecast zu int32_t um auf 16-Bit-Systemen Überläufe zu vermeiden
    score = whiteAttackersWeight * (int32_t)hceParams.getNumAttackerWeight(numWhiteAttackers) / 100 -
            blackAttackersWeight * (int32_t)hceParams.getNumAttackerWeight(numBlackAttackers) / 100;

    return score;
}

int HandcraftedEvaluator::evaluateOpenFiles() {
    int whiteKingSquare = board.getKingSquare(WHITE);
    int blackKingSquare = board.getKingSquare(BLACK);

    int whiteKingFile = Square::fileOf(whiteKingSquare);
    int blackKingFile = Square::fileOf(blackKingSquare);

    Bitboard whitePawns = board.getPieceBitboard(WHITE_PAWN);
    Bitboard blackPawns = board.getPieceBitboard(BLACK_PAWN);

    int whiteOpenFiles = 0;
    int blackOpenFiles = 0;

    for(int file : nearbyFiles[whiteKingFile])
        if(!(whitePawns & fileMasks[file]))
            whiteOpenFiles++;

    for(int file : nearbyFiles[blackKingFile])
        if(!(blackPawns & fileMasks[file]))
            blackOpenFiles++;

    return hceParams.getMGKingOpenFilePenalty(whiteOpenFiles) - hceParams.getMGKingOpenFilePenalty(blackOpenFiles);
}

int HandcraftedEvaluator::evaluatePawnSafety() {
    int score = 0;

    int whiteKingSquare = board.getKingSquare(WHITE);
    int blackKingSquare = board.getKingSquare(BLACK);

    Bitboard whitePawns = board.getPieceBitboard(WHITE_PAWN);
    Bitboard blackPawns = board.getPieceBitboard(BLACK_PAWN);

    Bitboard whitePawnStorm = pawnStormMask[WHITE / COLOR_MASK][whiteKingSquare] & blackPawns & ~evaluationVars.blackImmobilePawns;
    Bitboard blackPawnStorm = pawnStormMask[BLACK / COLOR_MASK][blackKingSquare] & whitePawns & ~evaluationVars.whiteImmobilePawns;

    while(whitePawnStorm) {
        int rank = Square::rankOf(whitePawnStorm.popFSB());
        score += hceParams.getMGPawnStormPenalty(rank);
    }

    while(blackPawnStorm) {
        int rank = Square::rankOf(Square::flipY(blackPawnStorm.popFSB()));
        score -= hceParams.getMGPawnStormPenalty(rank);
    }

    Bitboard whitePawnShield = pawnShieldMask[WHITE / COLOR_MASK][whiteKingSquare] & whitePawns;
    Bitboard blackPawnShield = pawnShieldMask[BLACK / COLOR_MASK][blackKingSquare] & blackPawns;

    int whitePawnShieldSize = whitePawnShield.popcount();
    int blackPawnShieldSize = blackPawnShield.popcount();

    whitePawnShieldSize = std::min(whitePawnShieldSize, 3);
    blackPawnShieldSize = std::min(blackPawnShieldSize, 3);

    score += hceParams.getMGPawnShieldSizeBonus(whitePawnShieldSize) - hceParams.getMGPawnShieldSizeBonus(blackPawnShieldSize);

    return score;
}

Score HandcraftedEvaluator::calculatePieceScore() {
    return evaluateAttackedPieces() + evaluatePieceMobility() + evaluateMinorPiecesOnStrongSquares() +
           evaluateBadBishops() + evaluateRooksOnOpenFiles() + evaluateRooksBehindPassedPawns() +
           evaluateBlockedPassedPawns() + evaluateKingPawnProximity() + evaluateRuleOfTheSquare();
}

Score HandcraftedEvaluator::evaluateAttackedPieces() {
    Score score{0, 0};

    Bitboard whitePawnAttacks = board.getAttackBitboard(WHITE_PAWN);
    Bitboard blackPawnAttacks = board.getAttackBitboard(BLACK_PAWN);

    Bitboard whiteMinorPieceAttacks = board.getAttackBitboard(WHITE_KNIGHT) | board.getAttackBitboard(WHITE_BISHOP);
    Bitboard blackMinorPieceAttacks = board.getAttackBitboard(BLACK_KNIGHT) | board.getAttackBitboard(BLACK_BISHOP);

    Bitboard whiteRookAttacks = board.getAttackBitboard(WHITE_ROOK);
    Bitboard blackRookAttacks = board.getAttackBitboard(BLACK_ROOK);

    int attackDiff = (board.getPieceBitboard(BLACK_PAWN) & ~blackPawnAttacks & whiteMinorPieceAttacks).popcount() -
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
        int sq = whiteKnights.popFSB();
        Bitboard attacks = knightAttackBitboard(sq) & ~whiteNotReachableSquares;
        int numAttacks = attacks.popcount();

        if(numAttacks == 0)
            score += {hceParams.getMGPieceNoMobilityPenalty(KNIGHT), hceParams.getEGPieceNoMobilityPenalty(KNIGHT)};
        else {
            numAttacks = std::sqrt(numAttacks);
            score += {numAttacks * hceParams.getMGPieceMobilityBonus(KNIGHT), numAttacks * hceParams.getEGPieceMobilityBonus(KNIGHT)};
        }
    }

    Bitboard blackKnights = board.getPieceBitboard(BLACK_KNIGHT);
    while(blackKnights) {
        int sq = blackKnights.popFSB();
        Bitboard attacks = knightAttackBitboard(sq) & ~blackNotReachableSquares;
        int numAttacks = attacks.popcount();

        if(numAttacks == 0)
            score -= {hceParams.getMGPieceNoMobilityPenalty(KNIGHT), hceParams.getEGPieceNoMobilityPenalty(KNIGHT)};
        else {
            numAttacks = std::sqrt(numAttacks);
            score -= {numAttacks * hceParams.getMGPieceMobilityBonus(KNIGHT), numAttacks * hceParams.getEGPieceMobilityBonus(KNIGHT)};
        }
    }

    Bitboard whiteBishops = board.getPieceBitboard(WHITE_BISHOP);
    while(whiteBishops) {
        int sq = whiteBishops.popFSB();
        Bitboard attacks = diagonalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(WHITE_KING)) & ~whiteNotReachableSquares;
        int numAttacks = attacks.popcount();

        if(numAttacks == 0)
            score += {hceParams.getMGPieceNoMobilityPenalty(BISHOP), hceParams.getEGPieceNoMobilityPenalty(BISHOP)};
        else {
            numAttacks = std::sqrt(numAttacks);
            score += {numAttacks * hceParams.getMGPieceMobilityBonus(BISHOP), numAttacks * hceParams.getEGPieceMobilityBonus(BISHOP)};
        }
    }

    Bitboard blackBishops = board.getPieceBitboard(BLACK_BISHOP);
    while(blackBishops) {
        int sq = blackBishops.popFSB();
        Bitboard attacks = diagonalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(BLACK_KING)) & ~blackNotReachableSquares;
        int numAttacks = attacks.popcount();

        if(numAttacks == 0)
            score -= {hceParams.getMGPieceNoMobilityPenalty(BISHOP), hceParams.getEGPieceNoMobilityPenalty(BISHOP)};
        else {
            numAttacks = std::sqrt(numAttacks);
            score -= {numAttacks * hceParams.getMGPieceMobilityBonus(BISHOP), numAttacks * hceParams.getEGPieceMobilityBonus(BISHOP)};
        }
    }

    whiteNotReachableSquares |= board.getAttackBitboard(BLACK_KNIGHT) | board.getAttackBitboard(BLACK_BISHOP);
    blackNotReachableSquares |= board.getAttackBitboard(WHITE_KNIGHT) | board.getAttackBitboard(WHITE_BISHOP);

    Bitboard whiteRooks = board.getPieceBitboard(WHITE_ROOK);
    while(whiteRooks) {
        int sq = whiteRooks.popFSB();
        Bitboard attacks = horizontalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(WHITE_KING)) & ~whiteNotReachableSquares;
        int numAttacks = attacks.popcount();

        if(numAttacks == 0)
            score += {hceParams.getMGPieceNoMobilityPenalty(ROOK), hceParams.getEGPieceNoMobilityPenalty(ROOK)};
        else {
            numAttacks = std::sqrt(numAttacks);
            score += {numAttacks * hceParams.getMGPieceMobilityBonus(ROOK), numAttacks * hceParams.getEGPieceMobilityBonus(ROOK)};
        }
    }

    Bitboard blackRooks = board.getPieceBitboard(BLACK_ROOK);
    while(blackRooks) {
        int sq = blackRooks.popFSB();
        Bitboard attacks = horizontalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(BLACK_KING)) & ~blackNotReachableSquares;
        int numAttacks = attacks.popcount();

        if(numAttacks == 0)
            score -= {hceParams.getMGPieceNoMobilityPenalty(ROOK), hceParams.getEGPieceNoMobilityPenalty(ROOK)};
        else {
            numAttacks = std::sqrt(numAttacks);
            score -= {numAttacks * hceParams.getMGPieceMobilityBonus(ROOK), numAttacks * hceParams.getEGPieceMobilityBonus(ROOK)};
        }
    }

    whiteNotReachableSquares |= board.getAttackBitboard(BLACK_ROOK);
    blackNotReachableSquares |= board.getAttackBitboard(WHITE_ROOK);

    Bitboard whiteQueens = board.getPieceBitboard(WHITE_QUEEN);
    while(whiteQueens) {
        int sq = whiteQueens.popFSB();
        Bitboard attacks = (diagonalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(WHITE_KING)) |
                            horizontalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(WHITE_KING))) & ~whiteNotReachableSquares;
        int numAttacks = attacks.popcount();

        if(numAttacks == 0)
            score += {hceParams.getMGPieceNoMobilityPenalty(QUEEN), hceParams.getEGPieceNoMobilityPenalty(QUEEN)};
        else {
            numAttacks = std::sqrt(numAttacks);
            score += {numAttacks * hceParams.getMGPieceMobilityBonus(QUEEN), numAttacks * hceParams.getEGPieceMobilityBonus(QUEEN)};
        }
    }

    Bitboard blackQueens = board.getPieceBitboard(BLACK_QUEEN);
    while(blackQueens) {
        int sq = blackQueens.popFSB();
        Bitboard attacks = (diagonalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(BLACK_KING)) |
                            horizontalAttackBitboard(sq, board.getPieceBitboard() | board.getPieceBitboard(BLACK_KING))) & ~blackNotReachableSquares;
        int numAttacks = attacks.popcount();

        if(numAttacks == 0)
            score -= {hceParams.getMGPieceNoMobilityPenalty(QUEEN), hceParams.getEGPieceNoMobilityPenalty(QUEEN)};
        else {
            numAttacks = std::sqrt(numAttacks);
            score -= {numAttacks * hceParams.getMGPieceMobilityBonus(QUEEN), numAttacks * hceParams.getEGPieceMobilityBonus(QUEEN)};
        }
    }

    return score;
}

Score HandcraftedEvaluator::evaluateMinorPiecesOnStrongSquares() {
    Bitboard whiteKnights = board.getPieceBitboard(WHITE_KNIGHT);
    Bitboard blackKnights = board.getPieceBitboard(BLACK_KNIGHT);
    Bitboard whiteBishops = board.getPieceBitboard(WHITE_BISHOP);
    Bitboard blackBishops = board.getPieceBitboard(BLACK_BISHOP);

    int numWhiteKnightsOnStrongSquares = (whiteKnights & evaluationVars.whiteStrongSquares).popcount();
    int numBlackKnightsOnStrongSquares = (blackKnights & evaluationVars.blackStrongSquares).popcount();
    int numWhiteBishopsOnStrongSquares = (whiteBishops & evaluationVars.whiteStrongSquares).popcount();
    int numBlackBishopsOnStrongSquares = (blackBishops & evaluationVars.blackStrongSquares).popcount();

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
        int sq = whiteBishops.popFSB();
        score += Score{hceParams.getMGBadBishopPenalty(), hceParams.getEGBadBishopPenalty()} * (badBishopMask[WHITE / COLOR_MASK][sq] & whiteBlockedSquares).popcount();
    }

    while(blackBishops) {
        int sq = blackBishops.popFSB();
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

    int numWhiteRooksOnOpenFiles = (whiteRooks & openFiles).popcount();
    int numBlackRooksOnOpenFiles = (blackRooks & openFiles).popcount();
    int numWhiteRooksOnSemiOpenFiles = (whiteRooks & whiteSemiOpenFiles).popcount();
    int numBlackRooksOnSemiOpenFiles = (blackRooks & blackSemiOpenFiles).popcount();

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
        int sq = whiteCandidates.popFSB();
        score.eg += !(fileFacingEnemy[WHITE / COLOR_MASK][sq] & whiteLinesBehindPassedPawns & occupancy) * hceParams.getEGRookBehindPassedPawnBonus();
    }

    while(blackCandidates) {
        int sq = blackCandidates.popFSB();
        score.eg -= !(fileFacingEnemy[BLACK / COLOR_MASK][sq] & blackLinesBehindPassedPawns & occupancy) * hceParams.getEGRookBehindPassedPawnBonus();
    }

    return score;
}

Score HandcraftedEvaluator::evaluateBlockedPassedPawns() {
    Bitboard whitePassedPawns = evaluationVars.whitePassedPawns;
    Bitboard blackPassedPawns = evaluationVars.blackPassedPawns;

    Bitboard whitePieces = board.getPieceBitboard(WHITE) | board.getPieceBitboard(WHITE_KING);
    Bitboard blackPieces = board.getPieceBitboard(BLACK) | board.getPieceBitboard(BLACK_KING);

    Bitboard whiteRooks = board.getPieceBitboard(WHITE_ROOK);
    Bitboard blackRooks = board.getPieceBitboard(BLACK_ROOK);

    Score score = {0, 0};

    while(whitePassedPawns) {
        int sq = whitePassedPawns.popFSB();
        Bitboard pathToPromotion = fileFacingEnemy[WHITE / COLOR_MASK][sq];

        if(blackPieces & pathToPromotion)
            score.eg -= hceParams.getEGBlockedEnemyPassedPawnBonus();
    }

    while(blackPassedPawns) {
        int sq = blackPassedPawns.popFSB();
        Bitboard pathToPromotion = fileFacingEnemy[BLACK / COLOR_MASK][sq];

        if(whitePieces & pathToPromotion)
            score.eg += hceParams.getEGBlockedEnemyPassedPawnBonus();
    }

    return score;
}

int kingDistance(int sq1, int sq2) {
    int file1 = Square::fileOf(sq1);
    int rank1 = Square::rankOf(sq1);
    int file2 = Square::fileOf(sq2);
    int rank2 = Square::rankOf(sq2);

    return std::max(std::abs(file1 - file2), std::abs(rank1 - rank2));
}

Score HandcraftedEvaluator::evaluateKingPawnProximity() {
    Score score = {0, 0};

    int whiteKingSquare = board.getKingSquare(WHITE);
    int blackKingSquare = board.getKingSquare(BLACK);

    Bitboard pawns = board.getPieceBitboard(WHITE_PAWN) | board.getPieceBitboard(BLACK_PAWN);
    Bitboard backwardPawns = evaluationVars.whiteBackwardPawns | evaluationVars.blackBackwardPawns;
    Bitboard passedPawns = evaluationVars.whitePassedPawns | evaluationVars.blackPassedPawns;
    Bitboard candidatePassedPawns = evaluationVars.whiteCandidatePassedPawns | evaluationVars.blackCandidatePassedPawns;

    while(pawns) {
        int sq = pawns.popFSB();
        int whiteKingDist = kingDistance(whiteKingSquare, sq);
        int blackKingDist = kingDistance(blackKingSquare, sq);

        if(passedPawns.getBit(sq)) {
            score.eg += (7 - whiteKingDist) * hceParams.getEGKingProximityPassedPawnWeight();
            score.eg -= (7 - blackKingDist) * hceParams.getEGKingProximityPassedPawnWeight();
        } else if(candidatePassedPawns.getBit(sq)) {
            score.eg += (7 - whiteKingDist) * hceParams.getEGKingProximityCandidatePassedPawnWeight();
            score.eg -= (7 - blackKingDist) * hceParams.getEGKingProximityCandidatePassedPawnWeight();
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

        int sideToMove = board.getSideToMove();

        Bitboard whitePassedPawns = evaluationVars.whitePassedPawns;

        int blackKingSquare = board.getKingSquare(BLACK);
        int blackKingRank = Square::rankOf(blackKingSquare);
        int blackKingFile = Square::fileOf(blackKingSquare);

        while(whitePassedPawns) {
            int sq = whitePassedPawns.popFSB();
            int rank = Square::rankOf(sq);
            rank = std::max(rank, (int)RANK_3); // Bauern auf der 2. Reihe können zwei Felder ziehen
            int file = Square::fileOf(sq);

            if(std::max(std::abs(blackKingRank - rank), std::abs(blackKingFile - file)) - (sideToMove == BLACK) > RANK_8 - rank) {
                score += Score{hceParams.getRuleOfTheSquareBonus(), hceParams.getRuleOfTheSquareBonus()};
                break;
            }
        }

        Bitboard blackPassedPawns = evaluationVars.blackPassedPawns;

        int whiteKingSquare = board.getKingSquare(WHITE);
        int whiteKingRank = Square::rankOf(whiteKingSquare);
        int whiteKingFile = Square::fileOf(whiteKingSquare);

        while(blackPassedPawns) {
            int sq = blackPassedPawns.popFSB();
            int rank = Square::rankOf(sq);
            rank = std::min(rank, (int)RANK_6); // Bauern auf der 7. Reihe können zwei Felder ziehen
            int file = Square::fileOf(sq);

            if(std::max(std::abs(whiteKingRank - rank), std::abs(whiteKingFile - file)) - (sideToMove == WHITE) > rank - RANK_1) {
                score -= Score{hceParams.getRuleOfTheSquareBonus(), hceParams.getRuleOfTheSquareBonus()};
                break;
            }
        }

        return score;
    }

    return {0, 0};
}

bool HandcraftedEvaluator::isWinnable(int side) {
    if(board.getPieceBitboard(side | PAWN))
        return true;

    Bitboard minorPieces = board.getPieceBitboard(side | KNIGHT) | board.getPieceBitboard(side | BISHOP);
    Bitboard heavyPieces = board.getPieceBitboard(side | ROOK) | board.getPieceBitboard(side | QUEEN);

    if(minorPieces.popcount() >= 2 || heavyPieces) {
        int ownPieceValue = board.getPieceBitboard(side | KNIGHT).popcount() * SIMPLE_PIECE_VALUE[KNIGHT] +
                            board.getPieceBitboard(side | BISHOP).popcount() * SIMPLE_PIECE_VALUE[BISHOP] +
                            board.getPieceBitboard(side | ROOK).popcount() * SIMPLE_PIECE_VALUE[ROOK] +
                            board.getPieceBitboard(side | QUEEN).popcount() * SIMPLE_PIECE_VALUE[QUEEN];

        int otherSide = side ^ COLOR_MASK;

        int enemyPieceValue = board.getPieceBitboard(otherSide | KNIGHT).popcount() * SIMPLE_PIECE_VALUE[KNIGHT] +
                              board.getPieceBitboard(otherSide | BISHOP).popcount() * SIMPLE_PIECE_VALUE[BISHOP] +
                              board.getPieceBitboard(otherSide | ROOK).popcount() * SIMPLE_PIECE_VALUE[ROOK] +
                              board.getPieceBitboard(otherSide | QUEEN).popcount() * SIMPLE_PIECE_VALUE[QUEEN];

        return ownPieceValue - enemyPieceValue >= SIMPLE_PIECE_VALUE[ROOK];
    }

    return false;
}

bool HandcraftedEvaluator::isOppositeColorBishopEndgame() {
    Bitboard pawns = board.getPieceBitboard(WHITE_PAWN) | board.getPieceBitboard(BLACK_PAWN);
    Bitboard whiteBishops = board.getPieceBitboard(WHITE_BISHOP);
    Bitboard blackBishops = board.getPieceBitboard(BLACK_BISHOP);

    return (pawns | whiteBishops | blackBishops) == board.getPieceBitboard() &&
            whiteBishops.popcount() == 1 && blackBishops.popcount() == 1 &&
           (bool)(whiteBishops & lightSquares) != (bool)(blackBishops & lightSquares);
}

bool HandcraftedEvaluator::isDrawnKRPKREndgame() {
    Bitboard whitePawns = board.getPieceBitboard(WHITE_PAWN);
    Bitboard blackPawns = board.getPieceBitboard(BLACK_PAWN);
    Bitboard whiteKings = board.getPieceBitboard(WHITE_KING);
    Bitboard blackKings = board.getPieceBitboard(BLACK_KING);

    // Wenn sich der verteidigende König vor dem angreifenden Bauern befindet, ist das Endspiel Remis
    if(whitePawns)
        return whitePawns.shiftNorth().extrudeNorth() & blackKings;
    else
        return blackPawns.shiftSouth().extrudeSouth() & whiteKings;
}

int HandcraftedEvaluator::evaluateKNBKEndgame(int b, int k) {
    int evaluation = std::abs(evaluationVars.materialScore.eg);

    b = b < 0 ? -1 : 0;
    k = (k >> 3) + ((k ^ b) & 7);
    int manhattanDistToClosestCornerOfBishopSqColor = (15 * (k >> 3) ^ k) - (k >> 3);
    evaluation += (7 - manhattanDistToClosestCornerOfBishopSqColor) * EG_SPECIAL_MATE_PROGRESS_BONUS;

    return evaluation + EG_WINNING_BONUS;
}

int HandcraftedEvaluator::evaluateWinningNoPawnsEndgame(int k) {
    int evaluation = std::abs(evaluationVars.materialScore.eg);

    int file = SQ2F(k);
    int rank = SQ2R(k);

    file ^= (file - 4) >> 8;
    rank ^= (rank - 4) >> 8;

    int manhattanDistToCenter = (file + rank) & 7;
    evaluation += manhattanDistToCenter * EG_SPECIAL_MATE_PROGRESS_BONUS;

    return evaluation + EG_WINNING_BONUS;
}