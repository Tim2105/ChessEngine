#include "core/chess/BoardDefinitions.h"
#include "core/engine/evaluation/PSQT.h"
#include "core/engine/evaluation/StaticEvaluator.h"

#include <algorithm>

StaticEvaluator::StaticEvaluator(StaticEvaluator&& other) : Evaluator(*(other.b)) {}

StaticEvaluator& StaticEvaluator::operator=(StaticEvaluator&& other) {
    this->b = other.b;

    return *this;
}

void StaticEvaluator::updateBeforeMove(Move move) {
    int32_t capturedPieceType = TYPEOF(b->pieceAt(move.getDestination()));
    int32_t promotedPieceType = EMPTY;

    if(move.isPromotion()) {
        if(move.isPromotionQueen())
            promotedPieceType = QUEEN;
        else if(move.isPromotionRook())
            promotedPieceType = ROOK;
        else if(move.isPromotionBishop())
            promotedPieceType = BISHOP;
        else
            promotedPieceType = KNIGHT;
    } else if(move.isEnPassant())
        capturedPieceType = PAWN;

    materialWeight += PIECE_WEIGHTS[capturedPieceType];
    materialWeight -= PIECE_WEIGHTS[promotedPieceType] - PAWN_WEIGHT;

    gamePhase = (double)materialWeight / TOTAL_WEIGHT;
    gamePhase = gamePhase * (MAX_PHASE - MIN_PHASE) + MIN_PHASE; // phase in [MIN_PHASE, MAX_PHASE]
    gamePhase = std::clamp(gamePhase, 0.0, 1.0); // phase auf [0, 1] begrenzen
}

void StaticEvaluator::updateAfterUndo(Move move) {
    int32_t capturedPieceType = TYPEOF(b->pieceAt(move.getDestination()));
    int32_t promotedPieceType = EMPTY;

    if(move.isPromotion()) {
        if(move.isPromotionQueen())
            promotedPieceType = QUEEN;
        else if(move.isPromotionRook())
            promotedPieceType = ROOK;
        else if(move.isPromotionBishop())
            promotedPieceType = BISHOP;
        else
            promotedPieceType = KNIGHT;
    } else if(move.isEnPassant())
        capturedPieceType = PAWN;

    materialWeight -= PIECE_WEIGHTS[capturedPieceType];
    materialWeight += PIECE_WEIGHTS[promotedPieceType] - PAWN_WEIGHT;

    gamePhase = (double)materialWeight / TOTAL_WEIGHT;
    gamePhase = gamePhase * (MAX_PHASE - MIN_PHASE) + MIN_PHASE; // phase in [MIN_PHASE, MAX_PHASE]
    gamePhase = std::clamp(gamePhase, 0.0, 1.0); // phase auf [0, 1] begrenzen
}

void StaticEvaluator::initGamePhase() {
    materialWeight = TOTAL_WEIGHT;

    materialWeight -= b->getPieceBitboard(WHITE_PAWN).getNumberOfSetBits() * PAWN_WEIGHT;
    materialWeight -= b->getPieceBitboard(BLACK_PAWN).getNumberOfSetBits() * PAWN_WEIGHT;
    materialWeight -= b->getPieceBitboard(WHITE_KNIGHT).getNumberOfSetBits() * KNIGHT_WEIGHT;
    materialWeight -= b->getPieceBitboard(BLACK_KNIGHT).getNumberOfSetBits() * KNIGHT_WEIGHT;
    materialWeight -= b->getPieceBitboard(WHITE_BISHOP).getNumberOfSetBits() * BISHOP_WEIGHT;
    materialWeight -= b->getPieceBitboard(BLACK_BISHOP).getNumberOfSetBits() * BISHOP_WEIGHT;
    materialWeight -= b->getPieceBitboard(WHITE_ROOK).getNumberOfSetBits() * ROOK_WEIGHT;
    materialWeight -= b->getPieceBitboard(BLACK_ROOK).getNumberOfSetBits() * ROOK_WEIGHT;
    materialWeight -= b->getPieceBitboard(WHITE_QUEEN).getNumberOfSetBits() * QUEEN_WEIGHT;
    materialWeight -= b->getPieceBitboard(BLACK_QUEEN).getNumberOfSetBits() * QUEEN_WEIGHT;

    gamePhase = (double)materialWeight / TOTAL_WEIGHT;
    gamePhase = gamePhase * (MAX_PHASE - MIN_PHASE) + MIN_PHASE; // phase in [MIN_PHASE, MAX_PHASE]
    gamePhase = std::clamp(gamePhase, 0.0, 1.0); // phase auf [0, 1] begrenzen
}

double StaticEvaluator::getGamePhase() const {
    return gamePhase;
}

int32_t StaticEvaluator::evaluate() {
    if(isDraw() || isLikelyDraw()) // Unentschieden
        return 0;

    int32_t score = 0;
    int32_t side = b->getSideToMove();

    double phase = getGamePhase();

    double midgameWeight = 1 - phase;
    double endgameWeight = phase;
    
    if(midgameWeight > 0.0)
        score += middlegameEvaluation() * midgameWeight;

    if(endgameWeight > 0.0)
        score += endgameEvaluation() * endgameWeight;

    Score pawnStructureScore = {0, 0};
    Score whitePawnStructureScore = evalPawnStructure(WHITE);
    Score blackPawnStructureScore = evalPawnStructure(BLACK);

    pawnStructureScore = whitePawnStructureScore - blackPawnStructureScore;

    if(side == BLACK)
        pawnStructureScore *= -1;

    score += pawnStructureScore.mg * midgameWeight;
    score += pawnStructureScore.eg * endgameWeight;

    return score;
}

int32_t StaticEvaluator::middlegameEvaluation() {
    int32_t score = 0;

    score += evalMGMaterial();
    score += evalMG_PSQT();
    score += evalMGKingSafety();
    score += evalMGMobility();
    score += evalMGPieces();

    return score;
}

int32_t StaticEvaluator::endgameEvaluation() {
    int32_t score = 0;

    int32_t side = b->getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    int32_t ownKingPos = b->getKingSquare(side);
    int32_t otherKingPos = b->getKingSquare(otherSide);

    int32_t ownKingFile = SQ2F(ownKingPos);
    int32_t ownKingRank = SQ2R(ownKingPos);
    int32_t otherKingFile = SQ2F(otherKingPos);
    int32_t otherKingRank = SQ2R(otherKingPos);

    int32_t distBetweenKings = std::max(abs(ownKingFile - otherKingFile), abs(ownKingRank - otherKingRank));

    int32_t materialScore = evalEGMaterial();

    if(materialScore > EG_WINNING_MATERIAL_ADVANTAGE) // Wenn man einen gewinnenden Materialvorteil hat
        score += (7 - distBetweenKings) * EG_KING_DISTANCE_VALUE;
    else if(-materialScore > EG_WINNING_MATERIAL_ADVANTAGE) // Wenn der Gegner einen gewinnenden Materialvorteil hat
        score -= (7 - distBetweenKings) * EG_KING_DISTANCE_VALUE;

    score += materialScore;
    score += evalEG_PSQT();
    score += evalEGMobility();
    score += evalEGPieces();
    score += evalKingPawnEG();

    return score;
}

bool StaticEvaluator::isLikelyDraw() {
    // Unzureichendes Material
    int32_t whitePawns = b->getPieceBitboard(WHITE_PAWN).getNumberOfSetBits();
    int32_t whiteKnights = b->getPieceBitboard(WHITE_KNIGHT).getNumberOfSetBits();
    int32_t whiteBishops = b->getPieceBitboard(WHITE_BISHOP).getNumberOfSetBits();
    int32_t whiteRooks = b->getPieceBitboard(WHITE_ROOK).getNumberOfSetBits();
    int32_t whiteQueens = b->getPieceBitboard(WHITE_QUEEN).getNumberOfSetBits();

    int32_t blackPawns = b->getPieceBitboard(BLACK_PAWN).getNumberOfSetBits();
    int32_t blackKnights = b->getPieceBitboard(BLACK_KNIGHT).getNumberOfSetBits();
    int32_t blackBishops = b->getPieceBitboard(BLACK_BISHOP).getNumberOfSetBits();
    int32_t blackRooks = b->getPieceBitboard(BLACK_ROOK).getNumberOfSetBits();
    int32_t blackQueens = b->getPieceBitboard(BLACK_QUEEN).getNumberOfSetBits();

    if(whitePawns != 0 || blackPawns != 0)
        return false;
    
    int32_t whitePieceValue = whiteKnights * MG_PIECE_VALUE[KNIGHT] + whiteBishops * MG_PIECE_VALUE[BISHOP] +
        whiteRooks * MG_PIECE_VALUE[ROOK] + whiteQueens * MG_PIECE_VALUE[QUEEN];
    
    int32_t blackPieceValue = blackKnights * MG_PIECE_VALUE[KNIGHT] + blackBishops * MG_PIECE_VALUE[BISHOP] +
        blackRooks * MG_PIECE_VALUE[ROOK] + blackQueens * MG_PIECE_VALUE[QUEEN];
    
    if(abs(whitePieceValue - blackPieceValue) <= std::max(MG_PIECE_VALUE[KNIGHT], MG_PIECE_VALUE[BISHOP]))
        return true;

    return false;
}

int32_t StaticEvaluator::evalMGMaterial() {
    int32_t score = 0;
    int32_t side = b->getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    score += b->getPieceBitboard(side | PAWN).getNumberOfSetBits() * MG_PIECE_VALUE[PAWN];
    score += b->getPieceBitboard(side | KNIGHT).getNumberOfSetBits() * MG_PIECE_VALUE[KNIGHT];
    score += b->getPieceBitboard(side | BISHOP).getNumberOfSetBits() * MG_PIECE_VALUE[BISHOP];
    score += b->getPieceBitboard(side | ROOK).getNumberOfSetBits() * MG_PIECE_VALUE[ROOK];
    score += b->getPieceBitboard(side | QUEEN).getNumberOfSetBits() * MG_PIECE_VALUE[QUEEN];

    score -= b->getPieceBitboard(otherSide | PAWN).getNumberOfSetBits() * MG_PIECE_VALUE[PAWN];
    score -= b->getPieceBitboard(otherSide | KNIGHT).getNumberOfSetBits() * MG_PIECE_VALUE[KNIGHT];
    score -= b->getPieceBitboard(otherSide | BISHOP).getNumberOfSetBits() * MG_PIECE_VALUE[BISHOP];
    score -= b->getPieceBitboard(otherSide | ROOK).getNumberOfSetBits() * MG_PIECE_VALUE[ROOK];
    score -= b->getPieceBitboard(otherSide | QUEEN).getNumberOfSetBits() * MG_PIECE_VALUE[QUEEN];

    return score;
}

int32_t StaticEvaluator::evalEGMaterial() {
    int32_t score = 0;
    int32_t side = b->getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    score += b->getPieceBitboard(side | PAWN).getNumberOfSetBits() * EG_PIECE_VALUE[PAWN];
    score += b->getPieceBitboard(side | KNIGHT).getNumberOfSetBits() * EG_PIECE_VALUE[KNIGHT];
    score += b->getPieceBitboard(side | BISHOP).getNumberOfSetBits() * EG_PIECE_VALUE[BISHOP];
    score += b->getPieceBitboard(side | ROOK).getNumberOfSetBits() * EG_PIECE_VALUE[ROOK];
    score += b->getPieceBitboard(side | QUEEN).getNumberOfSetBits() * EG_PIECE_VALUE[QUEEN];

    score -= b->getPieceBitboard(otherSide | PAWN).getNumberOfSetBits() * EG_PIECE_VALUE[PAWN];
    score -= b->getPieceBitboard(otherSide | KNIGHT).getNumberOfSetBits() * EG_PIECE_VALUE[KNIGHT];
    score -= b->getPieceBitboard(otherSide | BISHOP).getNumberOfSetBits() * EG_PIECE_VALUE[BISHOP];
    score -= b->getPieceBitboard(otherSide | ROOK).getNumberOfSetBits() * EG_PIECE_VALUE[ROOK];
    score -= b->getPieceBitboard(otherSide | QUEEN).getNumberOfSetBits() * EG_PIECE_VALUE[QUEEN];

    return score;
}

int32_t StaticEvaluator::evalMGPieces() {
    int32_t score = 0;

    int32_t side = b->getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    Bitboard ownPawns = b->getPieceBitboard(side | PAWN);
    Bitboard otherPawns = b->getPieceBitboard(otherSide | PAWN);
    Bitboard pawns = ownPawns | otherPawns;

    Bitboard ownKnights = b->getPieceBitboard(side | KNIGHT);
    Bitboard otherKnights = b->getPieceBitboard(otherSide | KNIGHT);

    Bitboard ownBishops = b->getPieceBitboard(side | BISHOP);
    Bitboard otherBishops = b->getPieceBitboard(otherSide | BISHOP);

    Bitboard ownRooks = b->getPieceBitboard(side | ROOK);
    Bitboard otherRooks = b->getPieceBitboard(otherSide | ROOK);

    Bitboard ownQueens = b->getPieceBitboard(side | QUEEN);
    Bitboard otherQueens = b->getPieceBitboard(otherSide | QUEEN);

    score += evalMGKnights(ownKnights, pawns);
    score -= evalMGKnights(otherKnights, pawns);

    score += evalMGBishops(ownBishops, otherBishops, pawns);
    score -= evalMGBishops(otherBishops, ownBishops, pawns);

    score += evalMGRooks(ownRooks, ownPawns, otherPawns, side);
    score -= evalMGRooks(otherRooks, otherPawns, ownPawns, otherSide);

    score += evalMGQueens(ownQueens, ownKnights, ownBishops, side);
    score -= evalMGQueens(otherQueens, otherKnights, otherBishops, otherSide);

    return score;
}

int32_t StaticEvaluator::evalEGPieces() {
    int32_t score = 0;

    int32_t side = b->getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    Bitboard ownPawns = b->getPieceBitboard(side | PAWN);
    Bitboard otherPawns = b->getPieceBitboard(otherSide | PAWN);

    Bitboard ownPassedPawns = findPassedPawns(ownPawns, otherPawns, side);
    Bitboard otherPassedPawns = findPassedPawns(otherPawns, ownPawns, otherSide);

    Bitboard ownKnights = b->getPieceBitboard(side | KNIGHT);
    Bitboard otherKnights = b->getPieceBitboard(otherSide | KNIGHT);

    Bitboard ownBishops = b->getPieceBitboard(side | BISHOP);
    Bitboard otherBishops = b->getPieceBitboard(otherSide | BISHOP);

    Bitboard ownRooks = b->getPieceBitboard(side | ROOK);
    Bitboard otherRooks = b->getPieceBitboard(otherSide | ROOK);

    score += evalEGKnights(ownKnights, ownPawns | otherPawns);
    score -= evalEGKnights(otherKnights, ownPawns | otherPawns);

    score += evalEGBishops(ownBishops);
    score -= evalEGBishops(otherBishops);

    score += evalEGRooks(ownRooks, ownPawns, otherPawns, ownPassedPawns, otherPassedPawns, side);
    score -= evalEGRooks(otherRooks, otherPawns, ownPawns, otherPassedPawns, ownPassedPawns, otherSide);

    return score;
}

inline int32_t StaticEvaluator::evalMGKnights(const Bitboard& ownKnights, const Bitboard& pawns) {
    int32_t score = 0;

    score += ownKnights.getNumberOfSetBits() * KNIGHT_PAWN_VALUE * pawns.getNumberOfSetBits();

    return score;
}

inline int32_t StaticEvaluator::evalEGKnights(const Bitboard& ownKnights, const Bitboard& pawns) {
    int32_t score = 0;

    score += ownKnights.getNumberOfSetBits() * KNIGHT_PAWN_VALUE * pawns.getNumberOfSetBits();

    return score;
}

inline int32_t StaticEvaluator::evalMGBishops(const Bitboard& ownBishops, const Bitboard& otherBishops, const Bitboard& pawns) {
    int32_t score = 0;

    if(ownBishops.getNumberOfSetBits() > 1)
        score += MG_BISHOP_PAIR_VALUE;
    
    // Light Square Color Weakness
    if((ownBishops & lightSquares) && !(otherBishops & lightSquares)) {
        if((pawns & lightSquares & extendedCenter).getNumberOfSetBits() < 2) {
            score += MG_BISHOP_COLOR_DOMINANCE_VALUE;
        }
    }

    // Dark Square Color Weakness
    if((ownBishops & darkSquares) && !(otherBishops & darkSquares)) {
        if((pawns & darkSquares & extendedCenter).getNumberOfSetBits() < 2) {
            score += MG_BISHOP_COLOR_DOMINANCE_VALUE;
        }
    }

    return score;
}

inline int32_t StaticEvaluator::evalEGBishops(const Bitboard& ownBishops) {
    int32_t score = 0;

    if(ownBishops.getNumberOfSetBits() > 1)
        score += EG_BISHOP_PAIR_VALUE;

    return score;
}

inline int32_t StaticEvaluator::evalMGRooks(const Bitboard& ownRooks, const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side) {
    int32_t score = 0;
    
    Bitboard rooks = ownRooks;
    while(rooks) {
        int32_t sq = rooks.getFirstSetBit();
        rooks.clearBit(sq);

        Bitboard file = fileFacingEnemy[side / COLOR_MASK][sq];

        if(!(file & ownPawns)) {
            if(file & otherPawns)
                score += MG_ROOK_SEMI_OPEN_FILE_VALUE; // halboffene Linie
            else
                score += MG_ROOK_OPEN_FILE_VALUE; // offene Linie
        }
    }

    score += ownRooks.getNumberOfSetBits() * ROOK_CAPTURED_PAWN_VALUE * (16 - (ownPawns | otherPawns).getNumberOfSetBits());

    return score;
}

inline int32_t StaticEvaluator::evalEGRooks(const Bitboard& ownRooks, const Bitboard& ownPawns, const Bitboard& otherPawns,
                                         const Bitboard& ownPassedPawns, const Bitboard& otherPassedPawns, int32_t side) {
    int32_t score = 0;
    
    Bitboard rooks = ownRooks;
    while(rooks) {
        int32_t sq = rooks.getFirstSetBit();
        rooks.clearBit(sq);

        Bitboard fileTowardEnemy = fileFacingEnemy[side / COLOR_MASK][sq];

        if(!(fileTowardEnemy & ownPawns)) {
            if(fileTowardEnemy & otherPawns)
                score += EG_ROOK_SEMI_OPEN_FILE_VALUE; // halboffene Linie
            else
                score += EG_ROOK_OPEN_FILE_VALUE; // offene Linie
        } else {
            if(fileTowardEnemy & ownPassedPawns)
                score += EG_ROOK_SUPPORTING_PASSED_PAWN_VALUE; // Turm deckt einen eigenen Freibauern
        }

        Bitboard fileTowardSelf = fileFacingEnemy[(side ^ COLOR_MASK) / COLOR_MASK][sq];

        if(!(fileTowardSelf & ownPawns) && (fileTowardSelf & otherPassedPawns))
            score += EG_ROOK_BLOCKING_PASSED_PAWN_VALUE; // Turm blockiert einen gegnerischen Freibauern
    }

    score += ownRooks.getNumberOfSetBits() * ROOK_CAPTURED_PAWN_VALUE * (16 - (ownPawns | otherPawns).getNumberOfSetBits());

    return score;
}

inline int32_t StaticEvaluator::evalMGQueens(const Bitboard& ownQueens, const Bitboard& ownKnights, const Bitboard& ownBishops, int32_t side) {
    if(ownQueens.getNumberOfSetBits() != 1)
        return 0;
    
    int32_t score = 0;

    int32_t sq = ownQueens.getFirstSetBit();
    int32_t rank = sq / 8;

    if(side == WHITE && rank == RANK_1)
        return 0;
    
    if(side == BLACK && rank == RANK_8)
        return 0;

    int32_t knightSq1 = B1;
    int32_t knightSq2 = G1;
    int32_t bishopSq1 = C1;
    int32_t bishopSq2 = F1;

    if(side == BLACK) {
        knightSq1 = B8;
        knightSq2 = G8;
        bishopSq1 = C8;
        bishopSq2 = F8;
    }

    if(ownKnights.getBit(knightSq1))
        score += MG_DEVELOPED_QUEEN_VALUE;
    
    if(ownKnights.getBit(knightSq2))
        score += MG_DEVELOPED_QUEEN_VALUE;

    if(ownBishops.getBit(bishopSq1))
        score += MG_DEVELOPED_QUEEN_VALUE;

    if(ownBishops.getBit(bishopSq2))
        score += MG_DEVELOPED_QUEEN_VALUE;

    return score;
}

int32_t StaticEvaluator::evalKingPawnEG() {
    int32_t score = 0;

    int32_t side = b->getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    Bitboard ownMinorOrMajorPieces = b->getPieceBitboard(side | KNIGHT) | b->getPieceBitboard(side | BISHOP) |
                                     b->getPieceBitboard(side | ROOK) | b->getPieceBitboard(side | QUEEN);

    Bitboard otherMinorOrMajorPieces = b->getPieceBitboard(otherSide | KNIGHT) | b->getPieceBitboard(otherSide | BISHOP) |
                                       b->getPieceBitboard(otherSide | ROOK) | b->getPieceBitboard(otherSide | QUEEN);

    int32_t ownKingSquare = b->getKingSquare(side);
    int32_t otherKingSquare = b->getKingSquare(otherSide);

    Bitboard ownPawns = b->getPieceBitboard(side | PAWN);
    Bitboard otherPawns = b->getPieceBitboard(otherSide | PAWN);

    Bitboard ownPassedPawns = findPassedPawns(ownPawns, otherPawns, side);
    Bitboard otherPassedPawns = findPassedPawns(otherPawns, ownPawns, otherSide);

    if(!otherMinorOrMajorPieces)
        score += evalEGRuleOfTheSquare(ownPassedPawns, otherKingSquare, side, true);

    if(!ownMinorOrMajorPieces)
        score -= evalEGRuleOfTheSquare(otherPassedPawns, ownKingSquare, otherSide, false);

    return score;
}

inline int32_t StaticEvaluator::evalEGRuleOfTheSquare(const Bitboard& ownPassedPawns, int32_t otherKingSquare, int32_t side, bool canMoveNext) {
    int32_t score = 0;

    Bitboard passedPawns = ownPassedPawns;
    while(passedPawns) {
        int32_t sq = passedPawns.getFirstSetBit();
        passedPawns.clearBit(sq);

        int32_t pawnFile = sq % 8;
        int32_t pawnRank = sq / 8;
        int32_t otherKingFile = otherKingSquare % 8;
        int32_t otherKingRank = otherKingSquare / 8;

        int32_t distToPromotion = 0;
        if(side == WHITE)
            distToPromotion = RANK_8 - pawnRank;
        else
            distToPromotion = pawnRank - RANK_1;
        
        int32_t promotionRank = side == WHITE ? RANK_8 : RANK_1;
        
        int32_t kingDistToPromotionSquare = std::max(std::abs(otherKingFile - pawnFile),
                                                     std::abs(otherKingRank - promotionRank));

        if(canMoveNext)
            distToPromotion--;

        if(distToPromotion < kingDistToPromotionSquare - 1) {
            score += EG_UNSTOPPABLE_PAWN_VALUE;
            break;
        }
    }

    return score;
}

inline int32_t StaticEvaluator::evalMG_PSQT() {
    int32_t score = 0;
    int32_t whiteScore = 0;
    int32_t blackScore = 0;

    for(int p = PAWN; p <= KING; p++) {
        Bitboard whitePieces = b->getPieceBitboard(WHITE | p);
        Bitboard blackPieces = b->getPieceBitboard(BLACK | p);

        while(whitePieces) {
            int sq = whitePieces.getFirstSetBit();
            whiteScore += MG_PSQT[p][sq];
            whitePieces.clearBit(sq);
        }

        while(blackPieces) {
            int sq = blackPieces.getFirstSetBit();
            int rank = sq / 8;
            int file = sq % 8;
            blackScore += MG_PSQT[p][(RANK_8 - rank) * 8 + file];
            blackPieces.clearBit(sq);
        }
    }

    Bitboard whiteEnPrise = b->getWhiteOccupiedBitboard() & ~b->getAttackBitboard(WHITE);
    Bitboard blackEnPrise = b->getBlackOccupiedBitboard() & ~b->getAttackBitboard(BLACK);

    if(b->getSideToMove() == WHITE) {
        score += whiteScore;
        score -= whiteEnPrise.getNumberOfSetBits() * MG_PIECE_EN_PRISE_VALUE;
        score -= blackScore;
        score += blackEnPrise.getNumberOfSetBits() * MG_PIECE_EN_PRISE_VALUE;
    } else {
        score += blackScore;
        score -= blackEnPrise.getNumberOfSetBits() * MG_PIECE_EN_PRISE_VALUE;
        score -= whiteScore;
        score += whiteEnPrise.getNumberOfSetBits() * MG_PIECE_EN_PRISE_VALUE;
    }

    return score;
}

inline int32_t StaticEvaluator::evalEG_PSQT() {
    int32_t score = 0;
    int32_t whiteScore = 0;
    int32_t blackScore = 0;

    for(int p = PAWN; p <= KING; p++) {
        Bitboard whitePieces = b->getPieceBitboard(WHITE | p);
        Bitboard blackPieces = b->getPieceBitboard(BLACK | p);

        while(whitePieces) {
            int sq = whitePieces.getFirstSetBit();
            whiteScore += EG_PSQT[p][sq];
            whitePieces.clearBit(sq);
        }

        while(blackPieces) {
            int sq = blackPieces.getFirstSetBit();
            int rank = sq / 8;
            int file = sq % 8;
            blackScore += EG_PSQT[p][(RANK_8 - rank) * 8 + file];
            blackPieces.clearBit(sq);
        }
    }

    Bitboard whiteEnPrise = b->getWhiteOccupiedBitboard() & ~b->getAttackBitboard(WHITE);
    Bitboard blackEnPrise = b->getBlackOccupiedBitboard() & ~b->getAttackBitboard(BLACK);

    if(b->getSideToMove() == WHITE) {
        score += whiteScore;
        score -= whiteEnPrise.getNumberOfSetBits() * EG_PIECE_EN_PRISE_VALUE;
        score -= blackScore;
        score += blackEnPrise.getNumberOfSetBits() * EG_PIECE_EN_PRISE_VALUE;
    } else {
        score += blackScore;
        score -= blackEnPrise.getNumberOfSetBits() * EG_PIECE_EN_PRISE_VALUE;
        score -= whiteScore;
        score += whiteEnPrise.getNumberOfSetBits() * EG_PIECE_EN_PRISE_VALUE;
    }

    return score;
}

Score StaticEvaluator::evalPawnStructure(int32_t side) {
    Score score = Score{0, 0};

    Bitboard ownPawns = b->getPieceBitboard(side | PAWN);
    Bitboard otherPawns = b->getPieceBitboard((side ^ COLOR_MASK) | PAWN);
    Bitboard ownPawnAttacks = b->getPieceAttackBitboard(side | PAWN);

    Bitboard doublePawns = findDoublePawns(ownPawns, side);
    Bitboard isolatedPawns = findIsolatedPawns(ownPawns);
    Bitboard passedPawns = findPassedPawns(ownPawns, otherPawns, side);
    Bitboard pawnChains = findPawnChains(ownPawns, side);
    Bitboard connectedPawns = findConnectedPawns(ownPawns);

    score += evalDoublePawns(doublePawns);
    score += evalIsolatedPawns(isolatedPawns);
    score += evalPassedPawns(passedPawns, ownPawnAttacks, side);
    score += evalPawnChains(pawnChains);
    score += evalConnectedPawns(connectedPawns);

    return score;
}

inline Score StaticEvaluator::evalDoublePawns(Bitboard doublePawns) {
    return Score{
        doublePawns.getNumberOfSetBits() * MG_PAWN_DOUBLED_VALUE,
        doublePawns.getNumberOfSetBits() * EG_PAWN_DOUBLED_VALUE
    };
}

inline Score StaticEvaluator::evalIsolatedPawns(Bitboard isolatedPawns) {
    return {
        isolatedPawns.getNumberOfSetBits() * MG_PAWN_ISOLATED_VALUE,
        isolatedPawns.getNumberOfSetBits() * EG_PAWN_ISOLATED_VALUE
    };
}

inline Score StaticEvaluator::evalPassedPawns(Bitboard passedPawns, const Bitboard& ownPawnAttacks, int32_t side) {
    Score score{0, 0};

    while(passedPawns) {
        int sq = passedPawns.getFirstSetBit();
        passedPawns.clearBit(sq);

        int rank = sq / 8;
        int ranksAdvanced = (side == WHITE) ? (rank - RANK_2) : (RANK_7 - rank);

        score.mg += MG_PAWN_PASSED_BASE_VALUE;
        score.mg += ranksAdvanced * MG_PAWN_PASSED_RANK_ADVANCED_MULTIPLIER;
        score.eg += EG_PAWN_PASSED_BASE_VALUE;
        score.eg += ranksAdvanced * EG_PAWN_PASSED_RANK_ADVANCED_MULTIPLIER;

        if(ownPawnAttacks.getBit(sq)) {
            score.mg += MG_PASSED_PAWN_PROTECTION_VALUE;
            score.eg += EG_PASSED_PAWN_PROTECTION_VALUE;
        }
    }

    return score;
}

inline Score StaticEvaluator::evalPawnChains(Bitboard pawnChains) {
    return Score{
        pawnChains.getNumberOfSetBits() * MG_PAWN_CHAIN_VALUE,
        pawnChains.getNumberOfSetBits() * EG_PAWN_CHAIN_VALUE
    };
}

inline Score StaticEvaluator::evalConnectedPawns(Bitboard connectedPawns) {
    return {
        connectedPawns.getNumberOfSetBits() * MG_PAWN_CONNECTED_VALUE,
        connectedPawns.getNumberOfSetBits() * EG_PAWN_CONNECTED_VALUE
    };
}

inline int32_t StaticEvaluator::evalMGPawnShield(int32_t kingSquare, const Bitboard& ownPawns, int32_t side) {
    return (pawnShieldMask[side / 8][kingSquare] & ownPawns).getNumberOfSetBits() * MG_PAWN_SHIELD_VALUE;
}

inline int32_t StaticEvaluator::evalMGPawnStorm(int32_t otherKingSquare, const Bitboard& ownPawns, int32_t side) {
    int score = 0;

    Bitboard stormingPawns = pawnStormMask[side / 8][otherKingSquare] & ownPawns;

    score += stormingPawns.getNumberOfSetBits() * MG_PAWN_STORM_BASE_VALUE;

    while(stormingPawns) {
        int i = stormingPawns.getFirstSetBit();
        stormingPawns.clearBit(i);

        int pawnRank = i / 8;
        int ranksAdvanced;
        if(side == WHITE)
            ranksAdvanced = pawnRank - RANK_2;
        else
            ranksAdvanced = RANK_7 - pawnRank;
        
        score += ranksAdvanced * MG_PAWN_STORM_DISTANCE_MULTIPLIER;
    }

    return score;
}

int32_t StaticEvaluator::evalMGKingAttackZone(int32_t side) {
    int32_t score = 0;

    int32_t otherSide = side ^ COLOR_MASK;
    Bitboard kingAttackZone = kingAttackZoneMask[side / COLOR_MASK][b->getKingSquare(side)];

    int32_t attackCounter = 0;
    
    int32_t knightThreats = (b->getPieceAttackBitboard(otherSide | KNIGHT) & kingAttackZone).getNumberOfSetBits();
    attackCounter += knightThreats * MG_KING_SAFETY_KNIGHT_THREAT_VALUE;

    int32_t bishopThreats = (b->getPieceAttackBitboard(otherSide | BISHOP) & kingAttackZone).getNumberOfSetBits();
    attackCounter += bishopThreats * MG_KING_SAFETY_BISHOP_THREAT_VALUE;

    int32_t rookThreats = (b->getPieceAttackBitboard(otherSide | ROOK) & kingAttackZone).getNumberOfSetBits();
    attackCounter += rookThreats * MG_KING_SAFETY_ROOK_THREAT_VALUE;

    int32_t queenThreats = (b->getPieceAttackBitboard(otherSide | QUEEN) & kingAttackZone).getNumberOfSetBits();
    attackCounter += queenThreats * MG_KING_SAFETY_QUEEN_THREAT_VALUE;
        
    if(attackCounter >= kingSafetyTableSize)
        attackCounter = kingSafetyTableSize - 1;

    score -= kingSafetyTable[attackCounter];

    return score;
}

int32_t StaticEvaluator::evalMGKingSafety() {
    int32_t score = 0;
    int32_t side = b->getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    Bitboard ownPawns = b->getPieceBitboard(side | PAWN);
    Bitboard otherPawns = b->getPieceBitboard(otherSide | PAWN);

    int32_t ownKingSafetyScore = 0;
    int32_t otherKingSafetyScore = 0;

    int32_t ownKingSquare = b->getKingSquare(side);
    int32_t otherKingSquare = b->getKingSquare(otherSide);

    ownKingSafetyScore += evalMGPawnShield(ownKingSquare, ownPawns, side);
    ownKingSafetyScore += evalMGPawnStorm(otherKingSquare, ownPawns, side);
    ownKingSafetyScore += evalMGKingAttackZone(side);

    otherKingSafetyScore += evalMGPawnShield(otherKingSquare, otherPawns, otherSide);
    otherKingSafetyScore += evalMGPawnStorm(ownKingSquare, otherPawns, otherSide);
    otherKingSafetyScore += evalMGKingAttackZone(otherSide);

    score += ownKingSafetyScore - otherKingSafetyScore;

    return score;
}

int32_t StaticEvaluator::evalMGMobility() {
    int32_t score = 0;
    int32_t side = b->getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    Bitboard ownPieces = b->getWhiteOccupiedBitboard() | b->getPieceBitboard(WHITE_KING);
    Bitboard otherPieces = b->getBlackOccupiedBitboard() | b->getPieceBitboard(BLACK_KING);

    if(side == BLACK) {
        ownPieces = b->getBlackOccupiedBitboard() | b->getPieceBitboard(BLACK_KING);
        otherPieces = b->getWhiteOccupiedBitboard() | b->getPieceBitboard(WHITE_KING);
    }

    Bitboard ownPawnMobility = b->getPieceAttackBitboard(side | PAWN);
    Bitboard otherPawnMobility = b->getPieceAttackBitboard(otherSide | PAWN);
    Bitboard ownKnightMobility = b->getPieceAttackBitboard(side | KNIGHT) & ~b->getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherKnightMobility = b->getPieceAttackBitboard(otherSide | KNIGHT) & ~b->getPieceAttackBitboard(side | PAWN);
    Bitboard ownBishopMobility = b->getPieceAttackBitboard(side | BISHOP) & ~b->getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherBishopMobility = b->getPieceAttackBitboard(otherSide | BISHOP) & ~b->getPieceAttackBitboard(side | PAWN);
    Bitboard ownRookMobility = b->getPieceAttackBitboard(side | ROOK) & ~b->getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherRookMobility = b->getPieceAttackBitboard(otherSide | ROOK) & ~b->getPieceAttackBitboard(side | PAWN);
    Bitboard ownQueenMobility = b->getPieceAttackBitboard(side | QUEEN) & ~b->getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherQueenMobility = b->getPieceAttackBitboard(otherSide | QUEEN) & ~b->getPieceAttackBitboard(side | PAWN);

    score += ownPawnMobility.getNumberOfSetBits() * MG_PAWN_MOBILITY_VALUE;
    score -= otherPawnMobility.getNumberOfSetBits() * MG_PAWN_MOBILITY_VALUE;
    score += ownKnightMobility.getNumberOfSetBits() * MG_KNIGHT_MOBILITY_VALUE;
    score -= otherKnightMobility.getNumberOfSetBits() * MG_KNIGHT_MOBILITY_VALUE;
    score += ownBishopMobility.getNumberOfSetBits() * MG_BISHOP_MOBILITY_VALUE;
    score -= otherBishopMobility.getNumberOfSetBits() * MG_BISHOP_MOBILITY_VALUE;
    score += ownRookMobility.getNumberOfSetBits() * MG_ROOK_MOBILITY_VALUE;
    score -= otherRookMobility.getNumberOfSetBits() * MG_ROOK_MOBILITY_VALUE;
    score += ownQueenMobility.getNumberOfSetBits() * MG_QUEEN_MOBILITY_VALUE;
    score -= otherQueenMobility.getNumberOfSetBits() * MG_QUEEN_MOBILITY_VALUE;

    return score;
}

int32_t StaticEvaluator::evalEGMobility() {
    int32_t score = 0;
    int32_t side = b->getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    Bitboard ownPieces = b->getWhiteOccupiedBitboard() | b->getPieceBitboard(WHITE_KING);
    Bitboard otherPieces = b->getBlackOccupiedBitboard() | b->getPieceBitboard(BLACK_KING);

    if(side == BLACK) {
        ownPieces = b->getBlackOccupiedBitboard() | b->getPieceBitboard(BLACK_KING);
        otherPieces = b->getWhiteOccupiedBitboard() | b->getPieceBitboard(WHITE_KING);
    }

    Bitboard ownPawnMobility = b->getPieceAttackBitboard(side | PAWN);
    Bitboard otherPawnMobility = b->getPieceAttackBitboard(otherSide | PAWN);
    Bitboard ownKnightMobility = b->getPieceAttackBitboard(side | KNIGHT) & ~b->getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherKnightMobility = b->getPieceAttackBitboard(otherSide | KNIGHT) & ~b->getPieceAttackBitboard(side | PAWN);
    Bitboard ownBishopMobility = b->getPieceAttackBitboard(side | BISHOP) & ~b->getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherBishopMobility = b->getPieceAttackBitboard(otherSide | BISHOP) & ~b->getPieceAttackBitboard(side | PAWN);
    Bitboard ownRookMobility = b->getPieceAttackBitboard(side | ROOK) & ~b->getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherRookMobility = b->getPieceAttackBitboard(otherSide | ROOK) & ~b->getPieceAttackBitboard(side | PAWN);
    Bitboard ownQueenMobility = b->getPieceAttackBitboard(side | QUEEN) & ~b->getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherQueenMobility = b->getPieceAttackBitboard(otherSide | QUEEN) & ~b->getPieceAttackBitboard(side | PAWN);

    score += ownPawnMobility.getNumberOfSetBits() * EG_PAWN_MOBILITY_VALUE;
    score -= otherPawnMobility.getNumberOfSetBits() * EG_PAWN_MOBILITY_VALUE;
    score += ownKnightMobility.getNumberOfSetBits() * EG_KNIGHT_MOBILITY_VALUE;
    score -= otherKnightMobility.getNumberOfSetBits() * EG_KNIGHT_MOBILITY_VALUE;
    score += ownBishopMobility.getNumberOfSetBits() * EG_BISHOP_MOBILITY_VALUE;
    score -= otherBishopMobility.getNumberOfSetBits() * EG_BISHOP_MOBILITY_VALUE;
    score += ownRookMobility.getNumberOfSetBits() * EG_ROOK_MOBILITY_VALUE;
    score -= otherRookMobility.getNumberOfSetBits() * EG_ROOK_MOBILITY_VALUE;
    score += ownQueenMobility.getNumberOfSetBits() * EG_QUEEN_MOBILITY_VALUE;
    score -= otherQueenMobility.getNumberOfSetBits() * EG_QUEEN_MOBILITY_VALUE;

    return score;
}

Bitboard StaticEvaluator::findDoublePawns(const Bitboard& ownPawns, int32_t side) {
    Bitboard doublePawns;
    Bitboard pawnCopy = ownPawns;

    while(pawnCopy) {
        int i = pawnCopy.getFirstSetBit();

        pawnCopy.clearBit(i);

        doublePawns |= fileFacingEnemy[side / COLOR_MASK][i] & ownPawns;
    }

    return doublePawns;
}

Bitboard StaticEvaluator::findIsolatedPawns(const Bitboard& ownPawns) {
    Bitboard isolatedPawns; 
    Bitboard pawnCopy = ownPawns;

    while(pawnCopy) {
        int i = pawnCopy.getFirstSetBit();
        pawnCopy.clearBit(i);

        int file = i % 8;
        if(!(ownPawns & neighboringFiles[file])) {
            isolatedPawns.setBit(i);
        }
    }

    return isolatedPawns;
}

Bitboard StaticEvaluator::findPassedPawns(const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side) {
    Bitboard passedPawns;
    Bitboard pawnCopy = ownPawns;

    while(pawnCopy) {
        int i = pawnCopy.getFirstSetBit();
        pawnCopy.clearBit(i);

        if(!(sentryMasks[side / COLOR_MASK][i] & otherPawns) &&
            !(fileFacingEnemy[side / COLOR_MASK][i] & ownPawns))
            passedPawns.setBit(i);
    }

    return passedPawns;
}

Bitboard StaticEvaluator::findPawnChains(const Bitboard& ownPawns, int32_t side) {
    Bitboard pawnChain;
    Bitboard pawnCopy = ownPawns;

    while(pawnCopy) {
        int i = pawnCopy.getFirstSetBit();
        pawnCopy.clearBit(i);

        pawnChain |= pawnChainMasks[side / 8][i] & ownPawns;
    }

    return pawnChain;
}

Bitboard StaticEvaluator::findConnectedPawns(const Bitboard& ownPawns) {
    Bitboard connectedPawns;
    Bitboard pawnCopy = ownPawns;

    while(pawnCopy) {
        int i = pawnCopy.getFirstSetBit();
        pawnCopy.clearBit(i);

        connectedPawns |= connectedPawnMasks[i] & ownPawns;
    }

    return connectedPawns;
}