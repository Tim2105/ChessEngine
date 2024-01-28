#include "core/chess/BoardDefinitions.h"
#include "core/engine/evaluation/PSQT.h"
#include "core/engine/evaluation/EndgameEvaluator.h"

#include <algorithm>

EndgameEvaluator::EndgameEvaluator(EndgameEvaluator&& other) : Evaluator(other.board) {}

EndgameEvaluator& EndgameEvaluator::operator=(EndgameEvaluator&& other) {
    this->board = other.board;

    return *this;
}

int32_t EndgameEvaluator::evaluate() {
    int32_t side = board.getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;
    int32_t score = endgameEvaluation();
    score += evalPawnStructure(side);
    score -= evalPawnStructure(otherSide);

    return score;
}

int32_t EndgameEvaluator::endgameEvaluation() {
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

int32_t EndgameEvaluator::evalEGMaterial() {
    int32_t score = 0;
    int32_t side = board.getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    score += board.getPieceBitboard(side | PAWN).getNumberOfSetBits() * EG_PIECE_VALUE[PAWN];
    score += board.getPieceBitboard(side | KNIGHT).getNumberOfSetBits() * EG_PIECE_VALUE[KNIGHT];
    score += board.getPieceBitboard(side | BISHOP).getNumberOfSetBits() * EG_PIECE_VALUE[BISHOP];
    score += board.getPieceBitboard(side | ROOK).getNumberOfSetBits() * EG_PIECE_VALUE[ROOK];
    score += board.getPieceBitboard(side | QUEEN).getNumberOfSetBits() * EG_PIECE_VALUE[QUEEN];

    score -= board.getPieceBitboard(otherSide | PAWN).getNumberOfSetBits() * EG_PIECE_VALUE[PAWN];
    score -= board.getPieceBitboard(otherSide | KNIGHT).getNumberOfSetBits() * EG_PIECE_VALUE[KNIGHT];
    score -= board.getPieceBitboard(otherSide | BISHOP).getNumberOfSetBits() * EG_PIECE_VALUE[BISHOP];
    score -= board.getPieceBitboard(otherSide | ROOK).getNumberOfSetBits() * EG_PIECE_VALUE[ROOK];
    score -= board.getPieceBitboard(otherSide | QUEEN).getNumberOfSetBits() * EG_PIECE_VALUE[QUEEN];

    return score;
}

int32_t EndgameEvaluator::evalEGPieces() {
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

inline int32_t EndgameEvaluator::evalEGKnights(const Bitboard& ownKnights, const Bitboard& pawns) {
    int32_t score = 0;

    score += ownKnights.getNumberOfSetBits() * KNIGHT_PAWN_VALUE * pawns.getNumberOfSetBits();

    return score;
}

inline int32_t EndgameEvaluator::evalEGBishops(const Bitboard& ownBishops) {
    int32_t score = 0;

    if(ownBishops.getNumberOfSetBits() > 1)
        score += EG_BISHOP_PAIR_VALUE;

    return score;
}

inline int32_t EndgameEvaluator::evalEGRooks(const Bitboard& ownRooks, const Bitboard& ownPawns, const Bitboard& otherPawns,
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

int32_t EndgameEvaluator::evalKingPawnEG() {
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

inline int32_t EndgameEvaluator::evalEGRuleOfTheSquare(const Bitboard& ownPassedPawns, int32_t otherKingSquare, int32_t side, bool canMoveNext) {
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

inline int32_t EndgameEvaluator::evalEG_PSQT() {
    int32_t score = 0;
    int32_t whiteScore = 0;
    int32_t blackScore = 0;

    for(int p = PAWN; p <= KING; p++) {
        Bitboard whitePieces = board.getPieceBitboard(WHITE | p);
        Bitboard blackPieces = board.getPieceBitboard(BLACK | p);

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

    Bitboard whiteEnPrise = board.getWhiteOccupiedBitboard() & ~board.getAttackBitboard(WHITE);
    Bitboard blackEnPrise = board.getBlackOccupiedBitboard() & ~board.getAttackBitboard(BLACK);

    if(board.getSideToMove() == WHITE) {
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

int32_t EndgameEvaluator::evalPawnStructure(int32_t side) {
    int32_t score = 0;

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

inline int32_t EndgameEvaluator::evalDoublePawns(Bitboard doublePawns) {
    return doublePawns.getNumberOfSetBits() * EG_PAWN_DOUBLED_VALUE;
}

inline int32_t EndgameEvaluator::evalIsolatedPawns(Bitboard isolatedPawns) {
    return isolatedPawns.getNumberOfSetBits() * EG_PAWN_ISOLATED_VALUE;
}

inline int32_t EndgameEvaluator::evalPassedPawns(Bitboard passedPawns, const Bitboard& ownPawnAttacks, int32_t side) {
    int32_t score = 0;

    while(passedPawns) {
        int sq = passedPawns.getFirstSetBit();
        passedPawns.clearBit(sq);

        int rank = sq / 8;
        int ranksAdvanced = (side == WHITE) ? (rank - RANK_2) : (RANK_7 - rank);

        score += EG_PAWN_PASSED_BASE_VALUE;
        score += ranksAdvanced * EG_PAWN_PASSED_RANK_ADVANCED_MULTIPLIER;

        if(ownPawnAttacks.getBit(sq))
            score += EG_PASSED_PAWN_PROTECTION_VALUE;
    }

    return score;
}

inline int32_t EndgameEvaluator::evalPawnChains(Bitboard pawnChains) {
    return pawnChains.getNumberOfSetBits() * EG_PAWN_CHAIN_VALUE;
}

inline int32_t EndgameEvaluator::evalConnectedPawns(Bitboard connectedPawns) {
    return connectedPawns.getNumberOfSetBits() * EG_PAWN_CONNECTED_VALUE;
}

int32_t EndgameEvaluator::evalEGMobility() {
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

Bitboard EndgameEvaluator::findDoublePawns(const Bitboard& ownPawns, int32_t side) {
    Bitboard doublePawns;
    Bitboard pawnCopy = ownPawns;

    while(pawnCopy) {
        int i = pawnCopy.getFirstSetBit();

        pawnCopy.clearBit(i);

        doublePawns |= fileFacingEnemy[side / COLOR_MASK][i] & ownPawns;
    }

    return doublePawns;
}

Bitboard EndgameEvaluator::findIsolatedPawns(const Bitboard& ownPawns) {
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

Bitboard EndgameEvaluator::findPassedPawns(const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side) {
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

Bitboard EndgameEvaluator::findPawnChains(const Bitboard& ownPawns, int32_t side) {
    Bitboard pawnChain;
    Bitboard pawnCopy = ownPawns;

    while(pawnCopy) {
        int i = pawnCopy.getFirstSetBit();
        pawnCopy.clearBit(i);

        pawnChain |= pawnChainMasks[side / 8][i] & ownPawns;
    }

    return pawnChain;
}

Bitboard EndgameEvaluator::findConnectedPawns(const Bitboard& ownPawns) {
    Bitboard connectedPawns;
    Bitboard pawnCopy = ownPawns;

    while(pawnCopy) {
        int i = pawnCopy.getFirstSetBit();
        pawnCopy.clearBit(i);

        connectedPawns |= connectedPawnMasks[i] & ownPawns;
    }

    return connectedPawns;
}