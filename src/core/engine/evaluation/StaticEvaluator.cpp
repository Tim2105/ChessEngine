#include "core/chess/BoardDefinitions.h"
#include "core/engine/evaluation/PSQT.h"
#include "core/engine/evaluation/StaticEvaluator.h"

#include <algorithm>

StaticEvaluator::StaticEvaluator(StaticEvaluator&& other) : Evaluator(other.board) {}

StaticEvaluator& StaticEvaluator::operator=(StaticEvaluator&& other) {
    this->board = other.board;

    return *this;
}

void StaticEvaluator::updateBeforeMove(Move move) {
    int32_t capturedPieceType = TYPEOF(board.pieceAt(move.getDestination()));
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
    int32_t capturedPieceType = TYPEOF(board.pieceAt(move.getDestination()));
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

    materialWeight -= board.getPieceBitboard(WHITE_PAWN).popcount() * PAWN_WEIGHT;
    materialWeight -= board.getPieceBitboard(BLACK_PAWN).popcount() * PAWN_WEIGHT;
    materialWeight -= board.getPieceBitboard(WHITE_KNIGHT).popcount() * KNIGHT_WEIGHT;
    materialWeight -= board.getPieceBitboard(BLACK_KNIGHT).popcount() * KNIGHT_WEIGHT;
    materialWeight -= board.getPieceBitboard(WHITE_BISHOP).popcount() * BISHOP_WEIGHT;
    materialWeight -= board.getPieceBitboard(BLACK_BISHOP).popcount() * BISHOP_WEIGHT;
    materialWeight -= board.getPieceBitboard(WHITE_ROOK).popcount() * ROOK_WEIGHT;
    materialWeight -= board.getPieceBitboard(BLACK_ROOK).popcount() * ROOK_WEIGHT;
    materialWeight -= board.getPieceBitboard(WHITE_QUEEN).popcount() * QUEEN_WEIGHT;
    materialWeight -= board.getPieceBitboard(BLACK_QUEEN).popcount() * QUEEN_WEIGHT;

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
    int32_t side = board.getSideToMove();

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

    int32_t side = board.getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    int32_t ownKingPos = board.getKingSquare(side);
    int32_t otherKingPos = board.getKingSquare(otherSide);

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
    int32_t whitePawns = board.getPieceBitboard(WHITE_PAWN).popcount();
    int32_t whiteKnights = board.getPieceBitboard(WHITE_KNIGHT).popcount();
    int32_t whiteBishops = board.getPieceBitboard(WHITE_BISHOP).popcount();
    int32_t whiteRooks = board.getPieceBitboard(WHITE_ROOK).popcount();
    int32_t whiteQueens = board.getPieceBitboard(WHITE_QUEEN).popcount();

    int32_t blackPawns = board.getPieceBitboard(BLACK_PAWN).popcount();
    int32_t blackKnights = board.getPieceBitboard(BLACK_KNIGHT).popcount();
    int32_t blackBishops = board.getPieceBitboard(BLACK_BISHOP).popcount();
    int32_t blackRooks = board.getPieceBitboard(BLACK_ROOK).popcount();
    int32_t blackQueens = board.getPieceBitboard(BLACK_QUEEN).popcount();

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
    int32_t side = board.getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    score += board.getPieceBitboard(side | PAWN).popcount() * MG_PIECE_VALUE[PAWN];
    score += board.getPieceBitboard(side | KNIGHT).popcount() * MG_PIECE_VALUE[KNIGHT];
    score += board.getPieceBitboard(side | BISHOP).popcount() * MG_PIECE_VALUE[BISHOP];
    score += board.getPieceBitboard(side | ROOK).popcount() * MG_PIECE_VALUE[ROOK];
    score += board.getPieceBitboard(side | QUEEN).popcount() * MG_PIECE_VALUE[QUEEN];

    score -= board.getPieceBitboard(otherSide | PAWN).popcount() * MG_PIECE_VALUE[PAWN];
    score -= board.getPieceBitboard(otherSide | KNIGHT).popcount() * MG_PIECE_VALUE[KNIGHT];
    score -= board.getPieceBitboard(otherSide | BISHOP).popcount() * MG_PIECE_VALUE[BISHOP];
    score -= board.getPieceBitboard(otherSide | ROOK).popcount() * MG_PIECE_VALUE[ROOK];
    score -= board.getPieceBitboard(otherSide | QUEEN).popcount() * MG_PIECE_VALUE[QUEEN];

    return score;
}

int32_t StaticEvaluator::evalEGMaterial() {
    int32_t score = 0;
    int32_t side = board.getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    score += board.getPieceBitboard(side | PAWN).popcount() * EG_PIECE_VALUE[PAWN];
    score += board.getPieceBitboard(side | KNIGHT).popcount() * EG_PIECE_VALUE[KNIGHT];
    score += board.getPieceBitboard(side | BISHOP).popcount() * EG_PIECE_VALUE[BISHOP];
    score += board.getPieceBitboard(side | ROOK).popcount() * EG_PIECE_VALUE[ROOK];
    score += board.getPieceBitboard(side | QUEEN).popcount() * EG_PIECE_VALUE[QUEEN];

    score -= board.getPieceBitboard(otherSide | PAWN).popcount() * EG_PIECE_VALUE[PAWN];
    score -= board.getPieceBitboard(otherSide | KNIGHT).popcount() * EG_PIECE_VALUE[KNIGHT];
    score -= board.getPieceBitboard(otherSide | BISHOP).popcount() * EG_PIECE_VALUE[BISHOP];
    score -= board.getPieceBitboard(otherSide | ROOK).popcount() * EG_PIECE_VALUE[ROOK];
    score -= board.getPieceBitboard(otherSide | QUEEN).popcount() * EG_PIECE_VALUE[QUEEN];

    return score;
}

int32_t StaticEvaluator::evalMGPieces() {
    int32_t score = 0;

    int32_t side = board.getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    Bitboard ownPawns = board.getPieceBitboard(side | PAWN);
    Bitboard otherPawns = board.getPieceBitboard(otherSide | PAWN);
    Bitboard pawns = ownPawns | otherPawns;

    Bitboard ownKnights = board.getPieceBitboard(side | KNIGHT);
    Bitboard otherKnights = board.getPieceBitboard(otherSide | KNIGHT);

    Bitboard ownBishops = board.getPieceBitboard(side | BISHOP);
    Bitboard otherBishops = board.getPieceBitboard(otherSide | BISHOP);

    Bitboard ownRooks = board.getPieceBitboard(side | ROOK);
    Bitboard otherRooks = board.getPieceBitboard(otherSide | ROOK);

    Bitboard ownQueens = board.getPieceBitboard(side | QUEEN);
    Bitboard otherQueens = board.getPieceBitboard(otherSide | QUEEN);

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

    int32_t side = board.getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    Bitboard ownPawns = board.getPieceBitboard(side | PAWN);
    Bitboard otherPawns = board.getPieceBitboard(otherSide | PAWN);

    Bitboard ownPassedPawns = findPassedPawns(ownPawns, otherPawns, side);
    Bitboard otherPassedPawns = findPassedPawns(otherPawns, ownPawns, otherSide);

    Bitboard ownKnights = board.getPieceBitboard(side | KNIGHT);
    Bitboard otherKnights = board.getPieceBitboard(otherSide | KNIGHT);

    Bitboard ownBishops = board.getPieceBitboard(side | BISHOP);
    Bitboard otherBishops = board.getPieceBitboard(otherSide | BISHOP);

    Bitboard ownRooks = board.getPieceBitboard(side | ROOK);
    Bitboard otherRooks = board.getPieceBitboard(otherSide | ROOK);

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

    score += ownKnights.popcount() * KNIGHT_PAWN_VALUE * pawns.popcount();

    return score;
}

inline int32_t StaticEvaluator::evalEGKnights(const Bitboard& ownKnights, const Bitboard& pawns) {
    int32_t score = 0;

    score += ownKnights.popcount() * KNIGHT_PAWN_VALUE * pawns.popcount();

    return score;
}

inline int32_t StaticEvaluator::evalMGBishops(const Bitboard& ownBishops, const Bitboard& otherBishops, const Bitboard& pawns) {
    int32_t score = 0;

    if(ownBishops.popcount() > 1)
        score += MG_BISHOP_PAIR_VALUE;
    
    // Light Square Color Weakness
    if((ownBishops & lightSquares) && !(otherBishops & lightSquares)) {
        if((pawns & lightSquares & extendedCenter).popcount() < 2) {
            score += MG_BISHOP_COLOR_DOMINANCE_VALUE;
        }
    }

    // Dark Square Color Weakness
    if((ownBishops & darkSquares) && !(otherBishops & darkSquares)) {
        if((pawns & darkSquares & extendedCenter).popcount() < 2) {
            score += MG_BISHOP_COLOR_DOMINANCE_VALUE;
        }
    }

    return score;
}

inline int32_t StaticEvaluator::evalEGBishops(const Bitboard& ownBishops) {
    int32_t score = 0;

    if(ownBishops.popcount() > 1)
        score += EG_BISHOP_PAIR_VALUE;

    return score;
}

inline int32_t StaticEvaluator::evalMGRooks(const Bitboard& ownRooks, const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side) {
    int32_t score = 0;
    
    Bitboard rooks = ownRooks;
    while(rooks) {
        int32_t sq = rooks.getFSB();
        rooks.clearBit(sq);

        Bitboard file = fileFacingEnemy[side / COLOR_MASK][sq];

        if(!(file & ownPawns)) {
            if(file & otherPawns)
                score += MG_ROOK_SEMI_OPEN_FILE_VALUE; // halboffene Linie
            else
                score += MG_ROOK_OPEN_FILE_VALUE; // offene Linie
        }
    }

    score += ownRooks.popcount() * ROOK_CAPTURED_PAWN_VALUE * (16 - (ownPawns | otherPawns).popcount());

    return score;
}

inline int32_t StaticEvaluator::evalEGRooks(const Bitboard& ownRooks, const Bitboard& ownPawns, const Bitboard& otherPawns,
                                         const Bitboard& ownPassedPawns, const Bitboard& otherPassedPawns, int32_t side) {
    int32_t score = 0;
    
    Bitboard rooks = ownRooks;
    while(rooks) {
        int32_t sq = rooks.getFSB();
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

    score += ownRooks.popcount() * ROOK_CAPTURED_PAWN_VALUE * (16 - (ownPawns | otherPawns).popcount());

    return score;
}

inline int32_t StaticEvaluator::evalMGQueens(const Bitboard& ownQueens, const Bitboard& ownKnights, const Bitboard& ownBishops, int32_t side) {
    if(ownQueens.popcount() != 1)
        return 0;
    
    int32_t score = 0;

    int32_t sq = ownQueens.getFSB();
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

    int32_t side = board.getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    Bitboard ownMinorOrMajorPieces = board.getPieceBitboard(side | KNIGHT) | board.getPieceBitboard(side | BISHOP) |
                                     board.getPieceBitboard(side | ROOK) | board.getPieceBitboard(side | QUEEN);

    Bitboard otherMinorOrMajorPieces = board.getPieceBitboard(otherSide | KNIGHT) | board.getPieceBitboard(otherSide | BISHOP) |
                                       board.getPieceBitboard(otherSide | ROOK) | board.getPieceBitboard(otherSide | QUEEN);

    int32_t ownKingSquare = board.getKingSquare(side);
    int32_t otherKingSquare = board.getKingSquare(otherSide);

    Bitboard ownPawns = board.getPieceBitboard(side | PAWN);
    Bitboard otherPawns = board.getPieceBitboard(otherSide | PAWN);

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
        int32_t sq = passedPawns.getFSB();
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
        Bitboard whitePieces = board.getPieceBitboard(WHITE | p);
        Bitboard blackPieces = board.getPieceBitboard(BLACK | p);

        while(whitePieces) {
            int sq = whitePieces.getFSB();
            whiteScore += MG_PSQT[p][sq];
            whitePieces.clearBit(sq);
        }

        while(blackPieces) {
            int sq = blackPieces.getFSB();
            int rank = sq / 8;
            int file = sq % 8;
            blackScore += MG_PSQT[p][(RANK_8 - rank) * 8 + file];
            blackPieces.clearBit(sq);
        }
    }

    Bitboard whiteEnPrise = board.getWhiteOccupiedBitboard() & ~board.getAttackBitboard(WHITE);
    Bitboard blackEnPrise = board.getBlackOccupiedBitboard() & ~board.getAttackBitboard(BLACK);

    if(board.getSideToMove() == WHITE) {
        score += whiteScore;
        score -= whiteEnPrise.popcount() * MG_PIECE_EN_PRISE_VALUE;
        score -= blackScore;
        score += blackEnPrise.popcount() * MG_PIECE_EN_PRISE_VALUE;
    } else {
        score += blackScore;
        score -= blackEnPrise.popcount() * MG_PIECE_EN_PRISE_VALUE;
        score -= whiteScore;
        score += whiteEnPrise.popcount() * MG_PIECE_EN_PRISE_VALUE;
    }

    return score;
}

inline int32_t StaticEvaluator::evalEG_PSQT() {
    int32_t score = 0;
    int32_t whiteScore = 0;
    int32_t blackScore = 0;

    for(int p = PAWN; p <= KING; p++) {
        Bitboard whitePieces = board.getPieceBitboard(WHITE | p);
        Bitboard blackPieces = board.getPieceBitboard(BLACK | p);

        while(whitePieces) {
            int sq = whitePieces.getFSB();
            whiteScore += EG_PSQT[p][sq];
            whitePieces.clearBit(sq);
        }

        while(blackPieces) {
            int sq = blackPieces.getFSB();
            int rank = sq / 8;
            int file = sq % 8;
            blackScore += EG_PSQT[p][(RANK_8 - rank) * 8 + file];
            blackPieces.clearBit(sq);
        }
    }

    Bitboard whiteEnPrise = board.getWhiteOccupiedBitboard() & ~board.getAttackBitboard(WHITE);
    Bitboard blackEnPrise = board.getBlackOccupiedBitboard() & ~board.getAttackBitboard(BLACK);

    if(board.getSideToMove() == WHITE) {
        score += whiteScore;
        score -= whiteEnPrise.popcount() * EG_PIECE_EN_PRISE_VALUE;
        score -= blackScore;
        score += blackEnPrise.popcount() * EG_PIECE_EN_PRISE_VALUE;
    } else {
        score += blackScore;
        score -= blackEnPrise.popcount() * EG_PIECE_EN_PRISE_VALUE;
        score -= whiteScore;
        score += whiteEnPrise.popcount() * EG_PIECE_EN_PRISE_VALUE;
    }

    return score;
}

Score StaticEvaluator::evalPawnStructure(int32_t side) {
    Score score = Score{0, 0};

    Bitboard ownPawns = board.getPieceBitboard(side | PAWN);
    Bitboard otherPawns = board.getPieceBitboard((side ^ COLOR_MASK) | PAWN);
    Bitboard ownPawnAttacks = board.getPieceAttackBitboard(side | PAWN);

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
        doublePawns.popcount() * MG_PAWN_DOUBLED_VALUE,
        doublePawns.popcount() * EG_PAWN_DOUBLED_VALUE
    };
}

inline Score StaticEvaluator::evalIsolatedPawns(Bitboard isolatedPawns) {
    return {
        isolatedPawns.popcount() * MG_PAWN_ISOLATED_VALUE,
        isolatedPawns.popcount() * EG_PAWN_ISOLATED_VALUE
    };
}

inline Score StaticEvaluator::evalPassedPawns(Bitboard passedPawns, const Bitboard& ownPawnAttacks, int32_t side) {
    Score score{0, 0};

    while(passedPawns) {
        int sq = passedPawns.getFSB();
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
        pawnChains.popcount() * MG_PAWN_CHAIN_VALUE,
        pawnChains.popcount() * EG_PAWN_CHAIN_VALUE
    };
}

inline Score StaticEvaluator::evalConnectedPawns(Bitboard connectedPawns) {
    return {
        connectedPawns.popcount() * MG_PAWN_CONNECTED_VALUE,
        connectedPawns.popcount() * EG_PAWN_CONNECTED_VALUE
    };
}

inline int32_t StaticEvaluator::evalMGPawnShield(int32_t kingSquare, const Bitboard& ownPawns, int32_t side) {
    return (pawnShieldMask[side / 8][kingSquare] & ownPawns).popcount() * MG_PAWN_SHIELD_VALUE;
}

inline int32_t StaticEvaluator::evalMGPawnStorm(int32_t otherKingSquare, const Bitboard& ownPawns, int32_t side) {
    int score = 0;

    Bitboard stormingPawns = pawnStormMask[side / 8][otherKingSquare] & ownPawns;

    score += stormingPawns.popcount() * MG_PAWN_STORM_BASE_VALUE;

    while(stormingPawns) {
        int i = stormingPawns.getFSB();
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
    Bitboard kingAttackZone = kingAttackZoneMask[side / COLOR_MASK][board.getKingSquare(side)];

    int32_t attackCounter = 0;
    
    int32_t knightThreats = (board.getPieceAttackBitboard(otherSide | KNIGHT) & kingAttackZone).popcount();
    attackCounter += knightThreats * MG_KING_SAFETY_KNIGHT_THREAT_VALUE;

    int32_t bishopThreats = (board.getPieceAttackBitboard(otherSide | BISHOP) & kingAttackZone).popcount();
    attackCounter += bishopThreats * MG_KING_SAFETY_BISHOP_THREAT_VALUE;

    int32_t rookThreats = (board.getPieceAttackBitboard(otherSide | ROOK) & kingAttackZone).popcount();
    attackCounter += rookThreats * MG_KING_SAFETY_ROOK_THREAT_VALUE;

    int32_t queenThreats = (board.getPieceAttackBitboard(otherSide | QUEEN) & kingAttackZone).popcount();
    attackCounter += queenThreats * MG_KING_SAFETY_QUEEN_THREAT_VALUE;
        
    if(attackCounter >= kingSafetyTableSize)
        attackCounter = kingSafetyTableSize - 1;

    score -= kingSafetyTable[attackCounter];

    return score;
}

int32_t StaticEvaluator::evalMGKingSafety() {
    int32_t score = 0;
    int32_t side = board.getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    Bitboard ownPawns = board.getPieceBitboard(side | PAWN);
    Bitboard otherPawns = board.getPieceBitboard(otherSide | PAWN);

    int32_t ownKingSafetyScore = 0;
    int32_t otherKingSafetyScore = 0;

    int32_t ownKingSquare = board.getKingSquare(side);
    int32_t otherKingSquare = board.getKingSquare(otherSide);

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
    int32_t side = board.getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    Bitboard ownPieces = board.getWhiteOccupiedBitboard() | board.getPieceBitboard(WHITE_KING);
    Bitboard otherPieces = board.getBlackOccupiedBitboard() | board.getPieceBitboard(BLACK_KING);

    if(side == BLACK) {
        ownPieces = board.getBlackOccupiedBitboard() | board.getPieceBitboard(BLACK_KING);
        otherPieces = board.getWhiteOccupiedBitboard() | board.getPieceBitboard(WHITE_KING);
    }

    Bitboard ownPawnMobility = board.getPieceAttackBitboard(side | PAWN);
    Bitboard otherPawnMobility = board.getPieceAttackBitboard(otherSide | PAWN);
    Bitboard ownKnightMobility = board.getPieceAttackBitboard(side | KNIGHT) & ~board.getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherKnightMobility = board.getPieceAttackBitboard(otherSide | KNIGHT) & ~board.getPieceAttackBitboard(side | PAWN);
    Bitboard ownBishopMobility = board.getPieceAttackBitboard(side | BISHOP) & ~board.getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherBishopMobility = board.getPieceAttackBitboard(otherSide | BISHOP) & ~board.getPieceAttackBitboard(side | PAWN);
    Bitboard ownRookMobility = board.getPieceAttackBitboard(side | ROOK) & ~board.getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherRookMobility = board.getPieceAttackBitboard(otherSide | ROOK) & ~board.getPieceAttackBitboard(side | PAWN);
    Bitboard ownQueenMobility = board.getPieceAttackBitboard(side | QUEEN) & ~board.getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherQueenMobility = board.getPieceAttackBitboard(otherSide | QUEEN) & ~board.getPieceAttackBitboard(side | PAWN);

    score += ownPawnMobility.popcount() * MG_PAWN_MOBILITY_VALUE;
    score -= otherPawnMobility.popcount() * MG_PAWN_MOBILITY_VALUE;
    score += ownKnightMobility.popcount() * MG_KNIGHT_MOBILITY_VALUE;
    score -= otherKnightMobility.popcount() * MG_KNIGHT_MOBILITY_VALUE;
    score += ownBishopMobility.popcount() * MG_BISHOP_MOBILITY_VALUE;
    score -= otherBishopMobility.popcount() * MG_BISHOP_MOBILITY_VALUE;
    score += ownRookMobility.popcount() * MG_ROOK_MOBILITY_VALUE;
    score -= otherRookMobility.popcount() * MG_ROOK_MOBILITY_VALUE;
    score += ownQueenMobility.popcount() * MG_QUEEN_MOBILITY_VALUE;
    score -= otherQueenMobility.popcount() * MG_QUEEN_MOBILITY_VALUE;

    return score;
}

int32_t StaticEvaluator::evalEGMobility() {
    int32_t score = 0;
    int32_t side = board.getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    Bitboard ownPieces = board.getWhiteOccupiedBitboard() | board.getPieceBitboard(WHITE_KING);
    Bitboard otherPieces = board.getBlackOccupiedBitboard() | board.getPieceBitboard(BLACK_KING);

    if(side == BLACK) {
        ownPieces = board.getBlackOccupiedBitboard() | board.getPieceBitboard(BLACK_KING);
        otherPieces = board.getWhiteOccupiedBitboard() | board.getPieceBitboard(WHITE_KING);
    }

    Bitboard ownPawnMobility = board.getPieceAttackBitboard(side | PAWN);
    Bitboard otherPawnMobility = board.getPieceAttackBitboard(otherSide | PAWN);
    Bitboard ownKnightMobility = board.getPieceAttackBitboard(side | KNIGHT) & ~board.getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherKnightMobility = board.getPieceAttackBitboard(otherSide | KNIGHT) & ~board.getPieceAttackBitboard(side | PAWN);
    Bitboard ownBishopMobility = board.getPieceAttackBitboard(side | BISHOP) & ~board.getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherBishopMobility = board.getPieceAttackBitboard(otherSide | BISHOP) & ~board.getPieceAttackBitboard(side | PAWN);
    Bitboard ownRookMobility = board.getPieceAttackBitboard(side | ROOK) & ~board.getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherRookMobility = board.getPieceAttackBitboard(otherSide | ROOK) & ~board.getPieceAttackBitboard(side | PAWN);
    Bitboard ownQueenMobility = board.getPieceAttackBitboard(side | QUEEN) & ~board.getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherQueenMobility = board.getPieceAttackBitboard(otherSide | QUEEN) & ~board.getPieceAttackBitboard(side | PAWN);

    score += ownPawnMobility.popcount() * EG_PAWN_MOBILITY_VALUE;
    score -= otherPawnMobility.popcount() * EG_PAWN_MOBILITY_VALUE;
    score += ownKnightMobility.popcount() * EG_KNIGHT_MOBILITY_VALUE;
    score -= otherKnightMobility.popcount() * EG_KNIGHT_MOBILITY_VALUE;
    score += ownBishopMobility.popcount() * EG_BISHOP_MOBILITY_VALUE;
    score -= otherBishopMobility.popcount() * EG_BISHOP_MOBILITY_VALUE;
    score += ownRookMobility.popcount() * EG_ROOK_MOBILITY_VALUE;
    score -= otherRookMobility.popcount() * EG_ROOK_MOBILITY_VALUE;
    score += ownQueenMobility.popcount() * EG_QUEEN_MOBILITY_VALUE;
    score -= otherQueenMobility.popcount() * EG_QUEEN_MOBILITY_VALUE;

    return score;
}

Bitboard StaticEvaluator::findDoublePawns(const Bitboard& ownPawns, int32_t side) {
    Bitboard doublePawns;
    Bitboard pawnCopy = ownPawns;

    while(pawnCopy) {
        int i = pawnCopy.getFSB();

        pawnCopy.clearBit(i);

        doublePawns |= fileFacingEnemy[side / COLOR_MASK][i] & ownPawns;
    }

    return doublePawns;
}

Bitboard StaticEvaluator::findIsolatedPawns(const Bitboard& ownPawns) {
    Bitboard isolatedPawns; 
    Bitboard pawnCopy = ownPawns;

    while(pawnCopy) {
        int i = pawnCopy.getFSB();
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
        int i = pawnCopy.getFSB();
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
        int i = pawnCopy.getFSB();
        pawnCopy.clearBit(i);

        pawnChain |= pawnChainMasks[side / 8][i] & ownPawns;
    }

    return pawnChain;
}

Bitboard StaticEvaluator::findConnectedPawns(const Bitboard& ownPawns) {
    Bitboard connectedPawns;
    Bitboard pawnCopy = ownPawns;

    while(pawnCopy) {
        int i = pawnCopy.getFSB();
        pawnCopy.clearBit(i);

        connectedPawns |= connectedPawnMasks[i] & ownPawns;
    }

    return connectedPawns;
}