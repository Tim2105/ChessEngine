#include "NewEvaluator.h"
#include "core/chess/BoardDefinitions.h"
#include <algorithm>
#include "core/chess/MailboxDefinitions.h"

NewEvaluator::NewEvaluator(NewEvaluator&& other) : Evaluator(other.b) {
    this->evaluationTable = std::move(other.evaluationTable);
    this->pawnStructureTable = std::move(other.pawnStructureTable);
}

NewEvaluator& NewEvaluator::operator=(NewEvaluator&& other) {
    this->b = other.b;
    this->evaluationTable = std::move(other.evaluationTable);
    this->pawnStructureTable = std::move(other.pawnStructureTable);

    return *this;
}

double NewEvaluator::getGamePhase() const {
    double totalWeight = PAWN_WEIGHT * 16 + KNIGHT_WEIGHT * 4 + BISHOP_WEIGHT * 4 + ROOK_WEIGHT * 4 + QUEEN_WEIGHT * 2;
    double phase = totalWeight;

    phase -= b.getPieceBitboard(WHITE_PAWN).getNumberOfSetBits() * PAWN_WEIGHT;
    phase -= b.getPieceBitboard(BLACK_PAWN).getNumberOfSetBits() * PAWN_WEIGHT;
    phase -= b.getPieceBitboard(WHITE_KNIGHT).getNumberOfSetBits() * KNIGHT_WEIGHT;
    phase -= b.getPieceBitboard(BLACK_KNIGHT).getNumberOfSetBits() * KNIGHT_WEIGHT;
    phase -= b.getPieceBitboard(WHITE_BISHOP).getNumberOfSetBits() * BISHOP_WEIGHT;
    phase -= b.getPieceBitboard(BLACK_BISHOP).getNumberOfSetBits() * BISHOP_WEIGHT;
    phase -= b.getPieceBitboard(WHITE_ROOK).getNumberOfSetBits() * ROOK_WEIGHT;
    phase -= b.getPieceBitboard(BLACK_ROOK).getNumberOfSetBits() * ROOK_WEIGHT;
    phase -= b.getPieceBitboard(WHITE_QUEEN).getNumberOfSetBits() * QUEEN_WEIGHT;
    phase -= b.getPieceBitboard(BLACK_QUEEN).getNumberOfSetBits() * QUEEN_WEIGHT;

    phase = phase / totalWeight;
    phase = phase * (MAX_PHASE - MIN_PHASE) + MIN_PHASE; // phase in [MIN_PHASE, MAX_PHASE]
    phase = std::clamp(phase, 0.0, 1.0); // phase auf [0, 1] begrenzen

    return phase;
}

int32_t NewEvaluator::evaluate() {
    if(isDraw() || isLikelyDraw()) // Unentschieden
        return 0;
    
    int32_t storedScore;
    bool evaluationFound = probeEvaluationTable(storedScore);

    if(evaluationFound)
        return storedScore;

    int32_t score = 0;
    int32_t side = b.getSideToMove();

    double phase = getGamePhase();

    double midgameWeight = 1 - phase;
    double endgameWeight = phase;
    
    if(midgameWeight > 0.0)
        score += middlegameEvaluation() * midgameWeight;

    if(endgameWeight > 0.0)
        score += endgameEvaluation() * endgameWeight;

    Score pawnStructureScore = {0, 0};
    bool pawnStructureFound = probePawnStructure(pawnStructureScore);

    if(!pawnStructureFound) {
        Score whitePawnStructureScore = evalPawnStructure(WHITE);
        Score blackPawnStructureScore = evalPawnStructure(BLACK);

        pawnStructureScore = whitePawnStructureScore - blackPawnStructureScore;
        storePawnStructure(pawnStructureScore);
    }

    if(side == BLACK)
        pawnStructureScore *= -1;

    score += pawnStructureScore.mg * midgameWeight;
    score += pawnStructureScore.eg * endgameWeight;

    storeEvaluationTable(score);

    return score;
}

int32_t NewEvaluator::middlegameEvaluation() {
    int32_t score = 0;

    score += evalMGMaterial();
    score += evalMG_PSQT();
    score += evalMGKingSafety();
    score += evalMGMobility();

    return score;
}

int32_t NewEvaluator::endgameEvaluation() {
    int32_t score = 0;

    int32_t side = b.getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    int32_t ownKingPos = b.getKingSquare(side);
    int32_t otherKingPos = b.getKingSquare(otherSide);

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

    return score;
}

bool NewEvaluator::isLikelyDraw() {
    // Unzureichendes Material
    int32_t whitePawns = b.getPieceBitboard(WHITE_PAWN).getNumberOfSetBits();
    int32_t whiteKnights = b.getPieceBitboard(WHITE_KNIGHT).getNumberOfSetBits();
    int32_t whiteBishops = b.getPieceBitboard(WHITE_BISHOP).getNumberOfSetBits();
    int32_t whiteRooks = b.getPieceBitboard(WHITE_ROOK).getNumberOfSetBits();
    int32_t whiteQueens = b.getPieceBitboard(WHITE_QUEEN).getNumberOfSetBits();

    int32_t blackPawns = b.getPieceBitboard(BLACK_PAWN).getNumberOfSetBits();
    int32_t blackKnights = b.getPieceBitboard(BLACK_KNIGHT).getNumberOfSetBits();
    int32_t blackBishops = b.getPieceBitboard(BLACK_BISHOP).getNumberOfSetBits();
    int32_t blackRooks = b.getPieceBitboard(BLACK_ROOK).getNumberOfSetBits();
    int32_t blackQueens = b.getPieceBitboard(BLACK_QUEEN).getNumberOfSetBits();

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

int32_t NewEvaluator::evalMGMaterial() {
    int32_t score = 0;
    int32_t side = b.getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    int32_t numPawns = b.getPieceBitboard(side | PAWN).getNumberOfSetBits() + b.getPieceBitboard(otherSide | PAWN).getNumberOfSetBits();

    score += b.getPieceBitboard(side | PAWN).getNumberOfSetBits() * MG_PIECE_VALUE[PAWN];
    score += b.getPieceBitboard(side | KNIGHT).getNumberOfSetBits() * (MG_PIECE_VALUE[KNIGHT] + KNIGHT_CAPTURED_PAWN_VALUE * (16 - numPawns));
    score += b.getPieceBitboard(side | BISHOP).getNumberOfSetBits() * MG_PIECE_VALUE[BISHOP];
    score += b.getPieceBitboard(side | ROOK).getNumberOfSetBits() * MG_PIECE_VALUE[ROOK];
    score += b.getPieceBitboard(side | QUEEN).getNumberOfSetBits() * MG_PIECE_VALUE[QUEEN];

    score -= b.getPieceBitboard(otherSide | PAWN).getNumberOfSetBits() * MG_PIECE_VALUE[PAWN];
    score -= b.getPieceBitboard(otherSide | KNIGHT).getNumberOfSetBits() * (MG_PIECE_VALUE[KNIGHT] + KNIGHT_CAPTURED_PAWN_VALUE * (16 - numPawns));
    score -= b.getPieceBitboard(otherSide | BISHOP).getNumberOfSetBits() * MG_PIECE_VALUE[BISHOP];
    score -= b.getPieceBitboard(otherSide | ROOK).getNumberOfSetBits() * MG_PIECE_VALUE[ROOK];
    score -= b.getPieceBitboard(otherSide | QUEEN).getNumberOfSetBits() * MG_PIECE_VALUE[QUEEN];

    int32_t numOwnBishops = b.getPieceBitboard(side | BISHOP).getNumberOfSetBits();
    int32_t numOtherBishops = b.getPieceBitboard(otherSide | BISHOP).getNumberOfSetBits();

    if(numOwnBishops > 1)
        score += BISHOP_PAIR_VALUE;

    if(numOtherBishops > 1)
        score -= BISHOP_PAIR_VALUE;

    return score;
}

int32_t NewEvaluator::evalEGMaterial() {
    int32_t score = 0;
    int32_t side = b.getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    int32_t numPawns = b.getPieceBitboard(side | PAWN).getNumberOfSetBits() + b.getPieceBitboard(otherSide | PAWN).getNumberOfSetBits();

    score += b.getPieceBitboard(side | PAWN).getNumberOfSetBits() * EG_PIECE_VALUE[PAWN];
    score += b.getPieceBitboard(side | KNIGHT).getNumberOfSetBits() * (EG_PIECE_VALUE[KNIGHT] + KNIGHT_CAPTURED_PAWN_VALUE * (16 - numPawns));
    score += b.getPieceBitboard(side | BISHOP).getNumberOfSetBits() * EG_PIECE_VALUE[BISHOP];
    score += b.getPieceBitboard(side | ROOK).getNumberOfSetBits() * EG_PIECE_VALUE[ROOK];
    score += b.getPieceBitboard(side | QUEEN).getNumberOfSetBits() * EG_PIECE_VALUE[QUEEN];

    score -= b.getPieceBitboard(otherSide | PAWN).getNumberOfSetBits() * EG_PIECE_VALUE[PAWN];
    score -= b.getPieceBitboard(otherSide | KNIGHT).getNumberOfSetBits() * (EG_PIECE_VALUE[KNIGHT] + KNIGHT_CAPTURED_PAWN_VALUE * (16 - numPawns));
    score -= b.getPieceBitboard(otherSide | BISHOP).getNumberOfSetBits() * EG_PIECE_VALUE[BISHOP];
    score -= b.getPieceBitboard(otherSide | ROOK).getNumberOfSetBits() * EG_PIECE_VALUE[ROOK];
    score -= b.getPieceBitboard(otherSide | QUEEN).getNumberOfSetBits() * EG_PIECE_VALUE[QUEEN];

    int32_t numOwnBishops = b.getPieceBitboard(side | BISHOP).getNumberOfSetBits();
    int32_t numOtherBishops = b.getPieceBitboard(otherSide | BISHOP).getNumberOfSetBits();

    if(numOwnBishops > 1)
        score += BISHOP_PAIR_VALUE;

    if(numOtherBishops > 1)
        score -= BISHOP_PAIR_VALUE;

    return score;
}

inline int32_t NewEvaluator::evalMG_PSQT() {
    int32_t score = 0;
    int32_t whiteScore = 0;
    int32_t blackScore = 0;

    for(int p = PAWN; p <= KING; p++) {
        Bitboard whitePieces = b.getPieceBitboard(WHITE | p);
        Bitboard blackPieces = b.getPieceBitboard(BLACK | p);

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

    Bitboard whiteEnPrise = b.getWhiteOccupiedBitboard() & ~b.getAttackBitboard(WHITE);
    Bitboard blackEnPrise = b.getBlackOccupiedBitboard() & ~b.getAttackBitboard(BLACK);

    if(b.getSideToMove() == WHITE) {
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

inline int32_t NewEvaluator::evalEG_PSQT() {
    int32_t score = 0;
    int32_t whiteScore = 0;
    int32_t blackScore = 0;

    for(int p = PAWN; p <= KING; p++) {
        Bitboard whitePieces = b.getPieceBitboard(WHITE | p);
        Bitboard blackPieces = b.getPieceBitboard(BLACK | p);

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

    Bitboard whiteEnPrise = b.getWhiteOccupiedBitboard() & ~b.getAttackBitboard(WHITE);
    Bitboard blackEnPrise = b.getBlackOccupiedBitboard() & ~b.getAttackBitboard(BLACK);

    if(b.getSideToMove() == WHITE) {
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

bool NewEvaluator::probeEvaluationTable(int32_t& score) {
    uint64_t hash = b.getHashValue();

    return evaluationTable.probe(hash, score);
}

void NewEvaluator::storeEvaluationTable(int32_t score) {
    uint64_t hash = b.getHashValue();

    evaluationTable.put(hash, score);
}

bool NewEvaluator::probePawnStructure(Score& score) {  
    PawnBitboards pawnsBitboards = PawnBitboards {
        b.getPieceBitboard(WHITE | PAWN),
        b.getPieceBitboard(BLACK | PAWN)
    };

    uint64_t hash = std::hash<PawnBitboards>{}(pawnsBitboards);

    return pawnStructureTable.probe(hash, score);
}

void NewEvaluator::storePawnStructure(const Score& score) {
    PawnBitboards pawnsBitboards = PawnBitboards {
        b.getPieceBitboard(WHITE | PAWN),
        b.getPieceBitboard(BLACK | PAWN)
    };

    uint64_t hash = std::hash<PawnBitboards>{}(pawnsBitboards);

    pawnStructureTable.put(hash, score);
}

Score NewEvaluator::evalPawnStructure(int32_t side) {
    Score score = Score{0, 0};

    Bitboard ownPawns = b.getPieceBitboard(side | PAWN);
    Bitboard otherPawns = b.getPieceBitboard((side ^ COLOR_MASK) | PAWN);

    Bitboard doublePawns = findDoublePawns(ownPawns, side);
    Bitboard isolatedPawns = findIsolatedPawns(ownPawns);
    Bitboard passedPawns = findPassedPawns(ownPawns, otherPawns, side);
    Bitboard pawnChains = findPawnChains(ownPawns, side);
    Bitboard connectedPawns = findConnectedPawns(ownPawns);

    score += evalDoublePawns(doublePawns);
    score += evalIsolatedPawns(isolatedPawns);
    score += evalPassedPawns(passedPawns, side);
    score += evalPawnChains(pawnChains);
    score += evalConnectedPawns(connectedPawns);

    return score;
}

inline Score NewEvaluator::evalDoublePawns(Bitboard doublePawns) {
    return Score{
        doublePawns.getNumberOfSetBits() * MG_PAWN_DOUBLED_VALUE,
        doublePawns.getNumberOfSetBits() * EG_PAWN_DOUBLED_VALUE
    };
}

inline Score NewEvaluator::evalIsolatedPawns(Bitboard isolatedPawns) {
    return {
        isolatedPawns.getNumberOfSetBits() * MG_PAWN_ISOLATED_VALUE,
        isolatedPawns.getNumberOfSetBits() * EG_PAWN_ISOLATED_VALUE
    };
}

inline Score NewEvaluator::evalPassedPawns(Bitboard passedPawns, int32_t side) {
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
    }

    return score;
}

inline Score NewEvaluator::evalPawnChains(Bitboard pawnChains) {
    return Score{
        pawnChains.getNumberOfSetBits() * MG_PAWN_CHAIN_VALUE,
        pawnChains.getNumberOfSetBits() * EG_PAWN_CHAIN_VALUE
    };
}

inline Score NewEvaluator::evalConnectedPawns(Bitboard connectedPawns) {
    return {
        connectedPawns.getNumberOfSetBits() * MG_PAWN_CONNECTED_VALUE,
        connectedPawns.getNumberOfSetBits() * EG_PAWN_CONNECTED_VALUE
    };
}

inline int32_t NewEvaluator::evalMGPawnShield(int32_t kingSquare, const Bitboard& ownPawns, int32_t side) {
    return (pawnShieldMask[side / 8][kingSquare] & ownPawns).getNumberOfSetBits() * MG_PAWN_SHIELD_VALUE;
}

inline int32_t NewEvaluator::evalMGPawnStorm(int32_t otherKingSquare, const Bitboard& ownPawns, int32_t side) {
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

int32_t NewEvaluator::evalMGKingAttackZone(int32_t side) {
    int32_t score = 0;

    int32_t otherSide = side ^ COLOR_MASK;
    Bitboard kingAttackZone = kingAttackZoneMask[side / COLOR_MASK][Mailbox::mailbox[b.getKingSquare(side)]];

    int32_t attackCounter = 0;
    
    int32_t knightThreats = (b.getPieceAttackBitboard(otherSide | KNIGHT) & kingAttackZone).getNumberOfSetBits();
    attackCounter += knightThreats * MG_KING_SAFETY_KNIGHT_THREAT_VALUE;

    int32_t bishopThreats = (b.getPieceAttackBitboard(otherSide | BISHOP) & kingAttackZone).getNumberOfSetBits();
    attackCounter += bishopThreats * MG_KING_SAFETY_BISHOP_THREAT_VALUE;

    int32_t rookThreats = (b.getPieceAttackBitboard(otherSide | ROOK) & kingAttackZone).getNumberOfSetBits();
    attackCounter += rookThreats * MG_KING_SAFETY_ROOK_THREAT_VALUE;

    int32_t queenThreats = (b.getPieceAttackBitboard(otherSide | QUEEN) & kingAttackZone).getNumberOfSetBits();
    attackCounter += queenThreats * MG_KING_SAFETY_QUEEN_THREAT_VALUE;
        
    score -= kingSafetyTable[attackCounter];

    return score;
}

Score NewEvaluator::evalPawnStructure(Bitboard doublePawns, Bitboard isolatedPawns, Bitboard passedPawns, Bitboard pawnChains, Bitboard connectedPawns, int32_t side) {
    Score score{0, 0};

    // Doppelte Bauern bewerten
    score += evalDoublePawns(doublePawns);

    // Isolierte Bauern bewerten
    score += evalIsolatedPawns(isolatedPawns);

    // Freibauern(passed pawns) bewerten
    score += evalPassedPawns(passedPawns, side);

    // Bauernketten bewerten
    score += evalPawnChains(pawnChains);

    // Verbundene(nebeneinander stehende) Bauern bewerten
    score += evalConnectedPawns(connectedPawns);

    return score;
}

int32_t NewEvaluator::evalMGKingSafety() {
    int32_t score = 0;
    int32_t side = b.getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    Bitboard ownPawns = b.getPieceBitboard(side | PAWN);
    Bitboard otherPawns = b.getPieceBitboard(otherSide | PAWN);

    int32_t ownKingSafetyScore = 0;
    int32_t otherKingSafetyScore = 0;

    int32_t ownKingSquare = Mailbox::mailbox[b.getKingSquare(side)];
    int32_t otherKingSquare = Mailbox::mailbox[b.getKingSquare(otherSide)];

    ownKingSafetyScore += evalMGPawnShield(ownKingSquare, ownPawns, side);
    ownKingSafetyScore += evalMGPawnStorm(otherKingSquare, ownPawns, side);
    ownKingSafetyScore += evalMGKingAttackZone(side);

    otherKingSafetyScore += evalMGPawnShield(otherKingSquare, otherPawns, otherSide);
    otherKingSafetyScore += evalMGPawnStorm(ownKingSquare, otherPawns, otherSide);
    otherKingSafetyScore += evalMGKingAttackZone(otherSide);

    score += ownKingSafetyScore - otherKingSafetyScore;

    return score;
}

int32_t NewEvaluator::evalMGMobility() {
    int32_t score = 0;
    int32_t side = b.getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    Bitboard ownPieces = b.getWhiteOccupiedBitboard() | b.getPieceBitboard(WHITE_KING);
    Bitboard otherPieces = b.getBlackOccupiedBitboard() | b.getPieceBitboard(BLACK_KING);

    if(side == BLACK) {
        ownPieces = b.getBlackOccupiedBitboard() | b.getPieceBitboard(BLACK_KING);
        otherPieces = b.getWhiteOccupiedBitboard() | b.getPieceBitboard(WHITE_KING);
    }

    Bitboard ownPawnMobility = b.getPieceAttackBitboard(side | PAWN) & ~ownPieces;
    Bitboard otherPawnMobility = b.getPieceAttackBitboard(otherSide | PAWN) & ~otherPieces;
    Bitboard ownKnightMobility = b.getPieceAttackBitboard(side | KNIGHT) & ~ownPieces & ~b.getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherKnightMobility = b.getPieceAttackBitboard(otherSide | KNIGHT) & ~otherPieces & ~b.getPieceAttackBitboard(side | PAWN);
    Bitboard ownBishopMobility = b.getPieceAttackBitboard(side | BISHOP) & ~ownPieces & ~b.getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherBishopMobility = b.getPieceAttackBitboard(otherSide | BISHOP) & ~otherPieces & ~b.getPieceAttackBitboard(side | PAWN);
    Bitboard ownRookMobility = b.getPieceAttackBitboard(side | ROOK) & ~ownPieces & ~b.getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherRookMobility = b.getPieceAttackBitboard(otherSide | ROOK) & ~otherPieces & ~b.getPieceAttackBitboard(side | PAWN);
    Bitboard ownQueenMobility = b.getPieceAttackBitboard(side | QUEEN) & ~ownPieces & ~b.getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherQueenMobility = b.getPieceAttackBitboard(otherSide | QUEEN) & ~otherPieces & ~b.getPieceAttackBitboard(side | PAWN);

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

int32_t NewEvaluator::evalEGMobility() {
    int32_t score = 0;
    int32_t side = b.getSideToMove();
    int32_t otherSide = side ^ COLOR_MASK;

    Bitboard ownPieces = b.getWhiteOccupiedBitboard() | b.getPieceBitboard(WHITE_KING);
    Bitboard otherPieces = b.getBlackOccupiedBitboard() | b.getPieceBitboard(BLACK_KING);

    if(side == BLACK) {
        ownPieces = b.getBlackOccupiedBitboard() | b.getPieceBitboard(BLACK_KING);
        otherPieces = b.getWhiteOccupiedBitboard() | b.getPieceBitboard(WHITE_KING);
    }

    Bitboard ownPawnMobility = b.getPieceAttackBitboard(side | PAWN) & ~ownPieces;
    Bitboard otherPawnMobility = b.getPieceAttackBitboard(otherSide | PAWN) & ~otherPieces;
    Bitboard ownKnightMobility = b.getPieceAttackBitboard(side | KNIGHT) & ~ownPieces & ~b.getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherKnightMobility = b.getPieceAttackBitboard(otherSide | KNIGHT) & ~otherPieces & ~b.getPieceAttackBitboard(side | PAWN);
    Bitboard ownBishopMobility = b.getPieceAttackBitboard(side | BISHOP) & ~ownPieces & ~b.getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherBishopMobility = b.getPieceAttackBitboard(otherSide | BISHOP) & ~otherPieces & ~b.getPieceAttackBitboard(side | PAWN);
    Bitboard ownRookMobility = b.getPieceAttackBitboard(side | ROOK) & ~ownPieces & ~b.getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherRookMobility = b.getPieceAttackBitboard(otherSide | ROOK) & ~otherPieces & ~b.getPieceAttackBitboard(side | PAWN);
    Bitboard ownQueenMobility = b.getPieceAttackBitboard(side | QUEEN) & ~ownPieces & ~b.getPieceAttackBitboard(otherSide | PAWN);
    Bitboard otherQueenMobility = b.getPieceAttackBitboard(otherSide | QUEEN) & ~otherPieces & ~b.getPieceAttackBitboard(side | PAWN);

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

Bitboard NewEvaluator::findDoublePawns(const Bitboard& ownPawns, int32_t side) {
    Bitboard doublePawns;
    Bitboard pawnCopy = ownPawns;

    while(pawnCopy) {
        int i = pawnCopy.getFirstSetBit();

        pawnCopy.clearBit(i);

        doublePawns |= doubledPawnMasks[side / 8][i] & ownPawns;
    }

    return doublePawns;
}

Bitboard NewEvaluator::findIsolatedPawns(const Bitboard& ownPawns) {
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

Bitboard NewEvaluator::findPassedPawns(const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side) {
    Bitboard passedPawns;
    Bitboard pawnCopy = ownPawns;

    while(pawnCopy) {
        int i = pawnCopy.getFirstSetBit();
        pawnCopy.clearBit(i);

        if(!(sentryMasks[side / 8][i] & otherPawns) &&
            !(doubledPawnMasks[side / 8][i] & ownPawns))
            passedPawns.setBit(i);
    }

    return passedPawns;
}

Bitboard NewEvaluator::findPawnChains(const Bitboard& ownPawns, int32_t side) {
    Bitboard pawnChain;
    Bitboard pawnCopy = ownPawns;

    while(pawnCopy) {
        int i = pawnCopy.getFirstSetBit();
        pawnCopy.clearBit(i);

        pawnChain |= pawnChainMasks[side / 8][i] & ownPawns;
    }

    return pawnChain;
}

Bitboard NewEvaluator::findConnectedPawns(const Bitboard& ownPawns) {
    Bitboard connectedPawns;
    Bitboard pawnCopy = ownPawns;

    while(pawnCopy) {
        int i = pawnCopy.getFirstSetBit();
        pawnCopy.clearBit(i);

        connectedPawns |= connectedPawnMasks[i] & ownPawns;
    }

    return connectedPawns;
}