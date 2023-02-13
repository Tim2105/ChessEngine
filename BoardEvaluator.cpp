#include "BoardEvaluator.h"
#include "BoardDefinitions.h"
#include "EvaluationDefinitions.h"
#include <algorithm>

BoardEvaluator::BoardEvaluator(Board& b) {
    this->b = &b;
}

BoardEvaluator::BoardEvaluator(BoardEvaluator&& other) {
    this->b = other.b;
    this->evaluationTable = std::move(other.evaluationTable);
    this->pawnStructureTable = std::move(other.pawnStructureTable);
}

BoardEvaluator& BoardEvaluator::operator=(BoardEvaluator&& other) {
    this->b = other.b;
    this->evaluationTable = std::move(other.evaluationTable);
    this->pawnStructureTable = std::move(other.pawnStructureTable);

    return *this;
}

double BoardEvaluator::getGamePhase() const {
    double totalWeight = PAWN_WEIGHT * 16 + KNIGHT_WEIGHT * 4 + BISHOP_WEIGHT * 4 + ROOK_WEIGHT * 4 + QUEEN_WEIGHT * 2;
    double phase = totalWeight;

    phase -= b->pieceList[WHITE_PAWN].size() * PAWN_WEIGHT;
    phase -= b->pieceList[BLACK_PAWN].size() * PAWN_WEIGHT;
    phase -= b->pieceList[WHITE_KNIGHT].size() * KNIGHT_WEIGHT;
    phase -= b->pieceList[BLACK_KNIGHT].size() * KNIGHT_WEIGHT;
    phase -= b->pieceList[WHITE_BISHOP].size() * BISHOP_WEIGHT;
    phase -= b->pieceList[BLACK_BISHOP].size() * BISHOP_WEIGHT;
    phase -= b->pieceList[WHITE_ROOK].size() * ROOK_WEIGHT;
    phase -= b->pieceList[BLACK_ROOK].size() * ROOK_WEIGHT;
    phase -= b->pieceList[WHITE_QUEEN].size() * QUEEN_WEIGHT;
    phase -= b->pieceList[BLACK_QUEEN].size() * QUEEN_WEIGHT;

    phase = phase / totalWeight;
    phase = phase * (MAX_PHASE - MIN_PHASE) + MIN_PHASE; // phase in [MIN_PHASE, MAX_PHASE]
    phase = std::clamp(phase, 0.0, 1.0); // phase auf [0, 1] begrenzen

    return phase;
}

int32_t BoardEvaluator::evaluate() {
    if(isDraw() || isLikelyDraw()) // Unentschieden
        return 0;
    
    int32_t storedScore;
    bool evaluationFound = probeEvaluationTable(storedScore);

    if(evaluationFound)
        return storedScore;

    int32_t score = 0;
    int32_t side = b->side;
    int32_t otherSide = side ^ COLOR_MASK;

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

int32_t BoardEvaluator::middlegameEvaluation() {
    int32_t score = 0;

    score += evalMGMaterial();
    score += evalMG_PSQT() * MG_PSQT_MULTIPLIER;
    score += evalMGKingSafety();
    score += evalMGMobility();

    return score;
}

int32_t BoardEvaluator::endgameEvaluation() {
    int32_t score = 0;

    int32_t side = b->side;
    int32_t otherSide = side ^ COLOR_MASK;

    int32_t ownKingPos = b->pieceList[side | KING].front();
    int32_t otherKingPos = b->pieceList[otherSide | KING].front();

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
    score += evalEG_PSQT() * EG_PSQT_MULTIPLIER;
    score += evalEGMobility();

    return score;
}

bool BoardEvaluator::isDraw() {
    // Fifty-move rule
    if(b->fiftyMoveRule >= 100)
        return true;
    
    if(b->repetitionCount() >= 3)
        return true;

    // Unzureichendes Material
    int32_t whitePawns = b->pieceList[WHITE_PAWN].size();
    int32_t whiteKnights = b->pieceList[WHITE_KNIGHT].size();
    int32_t whiteBishops = b->pieceList[WHITE_BISHOP].size();
    int32_t whiteRooks = b->pieceList[WHITE_ROOK].size();
    int32_t whiteQueens = b->pieceList[WHITE_QUEEN].size();

    int32_t blackPawns = b->pieceList[BLACK_PAWN].size();
    int32_t blackKnights = b->pieceList[BLACK_KNIGHT].size();
    int32_t blackBishops = b->pieceList[BLACK_BISHOP].size();
    int32_t blackRooks = b->pieceList[BLACK_ROOK].size();
    int32_t blackQueens = b->pieceList[BLACK_QUEEN].size();

    // Wenn Bauern, Türme oder Damen auf dem Spielfeld sind, ist noch genug Material vorhanden
    if(!(whitePawns > 0 || blackPawns > 0 || whiteRooks > 0 ||
       blackRooks > 0 || whiteQueens > 0 || blackQueens > 0)) {
        
        // König gegen König
        if(whiteKnights == 0 && whiteBishops == 0 &&
        blackKnights == 0 && blackBishops == 0)
            return true;
        
        // König und Springer gegen König
        if(whiteBishops == 0 && blackBishops == 0 &&
        (whiteKnights == 1 && blackKnights == 0 || whiteKnights == 0 && blackKnights == 1))
            return true;
        
        // König und Läufer gegen König
        if(whiteKnights == 0 && blackKnights == 0 &&
        (whiteBishops == 1 && blackBishops == 0 || whiteBishops == 0 && blackBishops == 1))
            return true;
        
        // König und Läufer gegen König und Läufer mit gleicher Farbe
        if(whiteKnights == 0 && blackKnights == 0 &&
        whiteBishops == 1 && blackBishops == 1) {
            
            int whiteBishopSq = b->pieceList[WHITE_BISHOP].front();
            int blackBishopSq = b->pieceList[BLACK_BISHOP].front();

            int whiteBishopColor = whiteBishopSq % 20 < 10 ? whiteBishopSq % 2 : 1 - whiteBishopSq % 2;
            int blackBishopColor = blackBishopSq % 20 < 10 ? blackBishopSq % 2 : 1 - blackBishopSq % 2;

            if(whiteBishopColor == blackBishopColor)
                return true;
        }
    }

    return false;
}

bool BoardEvaluator::isLikelyDraw() {
    // Unzureichendes Material
    int32_t whitePawns = b->pieceList[WHITE_PAWN].size();
    int32_t whiteKnights = b->pieceList[WHITE_KNIGHT].size();
    int32_t whiteBishops = b->pieceList[WHITE_BISHOP].size();
    int32_t whiteRooks = b->pieceList[WHITE_ROOK].size();
    int32_t whiteQueens = b->pieceList[WHITE_QUEEN].size();

    int32_t blackPawns = b->pieceList[BLACK_PAWN].size();
    int32_t blackKnights = b->pieceList[BLACK_KNIGHT].size();
    int32_t blackBishops = b->pieceList[BLACK_BISHOP].size();
    int32_t blackRooks = b->pieceList[BLACK_ROOK].size();
    int32_t blackQueens = b->pieceList[BLACK_QUEEN].size();

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

int32_t BoardEvaluator::evalMGMaterial() {
    int32_t score = 0;
    int32_t side = b->side;
    int32_t otherSide = side ^ COLOR_MASK;

    int32_t numPawns = b->pieceList[side | PAWN].size() + b->pieceList[otherSide | PAWN].size();

    score += b->pieceList[side | PAWN].size() * MG_PIECE_VALUE[PAWN];
    score += b->pieceList[side | KNIGHT].size() * (MG_PIECE_VALUE[KNIGHT] + KNIGHT_CAPTURED_PAWN_VALUE * (16 - numPawns));
    score += b->pieceList[side | BISHOP].size() * MG_PIECE_VALUE[BISHOP];
    score += b->pieceList[side | ROOK].size() * MG_PIECE_VALUE[ROOK];
    score += b->pieceList[side | QUEEN].size() * MG_PIECE_VALUE[QUEEN];

    score -= b->pieceList[otherSide | PAWN].size() * MG_PIECE_VALUE[PAWN];
    score -= b->pieceList[otherSide | KNIGHT].size() * (MG_PIECE_VALUE[KNIGHT] + KNIGHT_CAPTURED_PAWN_VALUE * (16 - numPawns));
    score -= b->pieceList[otherSide | BISHOP].size() * MG_PIECE_VALUE[BISHOP];
    score -= b->pieceList[otherSide | ROOK].size() * MG_PIECE_VALUE[ROOK];
    score -= b->pieceList[otherSide | QUEEN].size() * MG_PIECE_VALUE[QUEEN];

    int32_t numOwnBishops = b->pieceList[side | BISHOP].size();
    int32_t numOtherBishops = b->pieceList[otherSide | BISHOP].size();

    if(numOwnBishops > 1)
        score += BISHOP_PAIR_VALUE;

    if(numOtherBishops > 1)
        score -= BISHOP_PAIR_VALUE;

    return score;
}

int32_t BoardEvaluator::evalEGMaterial() {
    int32_t score = 0;
    int32_t side = b->side;
    int32_t otherSide = side ^ COLOR_MASK;

    int32_t numPawns = b->pieceList[side | PAWN].size() + b->pieceList[otherSide | PAWN].size();

    score += b->pieceList[side | PAWN].size() * EG_PIECE_VALUE[PAWN];
    score += b->pieceList[side | KNIGHT].size() * (EG_PIECE_VALUE[KNIGHT] + KNIGHT_CAPTURED_PAWN_VALUE * (16 - numPawns));
    score += b->pieceList[side | BISHOP].size() * EG_PIECE_VALUE[BISHOP];
    score += b->pieceList[side | ROOK].size() * EG_PIECE_VALUE[ROOK];
    score += b->pieceList[side | QUEEN].size() * EG_PIECE_VALUE[QUEEN];

    score -= b->pieceList[otherSide | PAWN].size() * EG_PIECE_VALUE[PAWN];
    score -= b->pieceList[otherSide | KNIGHT].size() * (EG_PIECE_VALUE[KNIGHT] + KNIGHT_CAPTURED_PAWN_VALUE * (16 - numPawns));
    score -= b->pieceList[otherSide | BISHOP].size() * EG_PIECE_VALUE[BISHOP];
    score -= b->pieceList[otherSide | ROOK].size() * EG_PIECE_VALUE[ROOK];
    score -= b->pieceList[otherSide | QUEEN].size() * EG_PIECE_VALUE[QUEEN];

    int32_t numOwnBishops = b->pieceList[side | BISHOP].size();
    int32_t numOtherBishops = b->pieceList[otherSide | BISHOP].size();

    if(numOwnBishops > 1)
        score += BISHOP_PAIR_VALUE;

    if(numOtherBishops > 1)
        score -= BISHOP_PAIR_VALUE;

    return score;
}

inline int32_t BoardEvaluator::evalMG_PSQT() {
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

    Bitboard whiteEnPrise = b->whitePiecesBitboard & ~b->getAttackBitboard(WHITE);
    Bitboard blackEnPrise = b->blackPiecesBitboard & ~b->getAttackBitboard(BLACK);

    if(b->side == WHITE) {
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

inline int32_t BoardEvaluator::evalEG_PSQT() {
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

    Bitboard whiteEnPrise = b->whitePiecesBitboard & ~b->getAttackBitboard(WHITE);
    Bitboard blackEnPrise = b->blackPiecesBitboard & ~b->getAttackBitboard(BLACK);

    if(b->side == WHITE) {
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

bool BoardEvaluator::probeEvaluationTable(int32_t& score) {
    uint64_t hash = b->getHashValue();

    return evaluationTable.probe(hash, score);
}

void BoardEvaluator::storeEvaluationTable(int32_t score) {
    uint64_t hash = b->getHashValue();

    evaluationTable.put(hash, score);
}

bool BoardEvaluator::probePawnStructure(Score& score) {  
    PawnBitboards pawnsBitboards = PawnBitboards {
        b->pieceBitboard[WHITE | PAWN],
        b->pieceBitboard[BLACK | PAWN]
    };

    uint64_t hash = std::hash<PawnBitboards>{}(pawnsBitboards);

    return pawnStructureTable.probe(hash, score);
}

void BoardEvaluator::storePawnStructure(const Score& score) {
    PawnBitboards pawnsBitboards = PawnBitboards {
        b->pieceBitboard[WHITE | PAWN],
        b->pieceBitboard[BLACK | PAWN]
    };

    uint64_t hash = std::hash<PawnBitboards>{}(pawnsBitboards);

    pawnStructureTable.put(hash, score);
}

Score BoardEvaluator::evalPawnStructure(int32_t side) {
    Score score = Score{0, 0};

    Bitboard ownPawns = b->pieceBitboard[side | PAWN];
    Bitboard otherPawns = b->pieceBitboard[side ^ COLOR_MASK | PAWN];

    Bitboard doublePawns = findDoublePawns(ownPawns, side);
    Bitboard isolatedPawns = findIsolatedPawns(ownPawns, side);
    Bitboard passedPawns = findPassedPawns(ownPawns, otherPawns, side);
    Bitboard pawnChains = findPawnChains(ownPawns, side);
    Bitboard connectedPawns = findConnectedPawns(ownPawns);

    score += evalDoublePawns(doublePawns, side);
    score += evalIsolatedPawns(isolatedPawns, side);
    score += evalPassedPawns(passedPawns, side);
    score += evalPawnChains(pawnChains, side);
    score += evalConnectedPawns(connectedPawns, side);

    return score;
}

inline Score BoardEvaluator::evalDoublePawns(Bitboard doublePawns, int32_t side) {
    return Score{
        doublePawns.getNumberOfSetBits() * MG_PAWN_DOUBLED_VALUE,
        doublePawns.getNumberOfSetBits() * EG_PAWN_DOUBLED_VALUE
    };
}

inline Score BoardEvaluator::evalIsolatedPawns(Bitboard isolatedPawns, int32_t side) {
    return {
        isolatedPawns.getNumberOfSetBits() * MG_PAWN_ISOLATED_VALUE,
        isolatedPawns.getNumberOfSetBits() * EG_PAWN_ISOLATED_VALUE
    };
}

inline Score BoardEvaluator::evalPassedPawns(Bitboard passedPawns, int32_t side) {
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

inline Score BoardEvaluator::evalPawnChains(Bitboard pawnChains, int32_t side) {
    return Score{
        pawnChains.getNumberOfSetBits() * MG_PAWN_CHAIN_VALUE,
        pawnChains.getNumberOfSetBits() * EG_PAWN_CHAIN_VALUE
    };
}

inline Score BoardEvaluator::evalConnectedPawns(Bitboard connectedPawns, int32_t side) {
    return {
        connectedPawns.getNumberOfSetBits() * MG_PAWN_CONNECTED_VALUE,
        connectedPawns.getNumberOfSetBits() * EG_PAWN_CONNECTED_VALUE
    };
}

inline int32_t BoardEvaluator::evalMGPawnShield(int32_t kingSquare, const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side) {
    return (pawnShieldMask[side / 8][kingSquare] & ownPawns).getNumberOfSetBits() * MG_PAWN_SHIELD_VALUE;
}

inline int32_t BoardEvaluator::evalMGPawnStorm(int32_t otherKingSquare, const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side) {
    int score = 0;

    int rank = otherKingSquare / 8;
    int file = otherKingSquare % 8;

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

int32_t BoardEvaluator::evalMGKingAttackZone(int32_t side) {
    int32_t score = 0;

    int32_t otherSide = side ^ COLOR_MASK;
    Bitboard kingAttackZone = kingAttackZoneMask[side / COLOR_MASK][b->mailbox[b->pieceList[side | KING].front()]];

    int32_t numAttackers = 0, attackCounter = 0;
    
    int32_t knightThreats = (b->pieceAttackBitboard[otherSide | KNIGHT] & kingAttackZone).getNumberOfSetBits();
    if(knightThreats > 0) {
        attackCounter += knightThreats * MG_KING_SAFETY_KNIGHT_THREAT_VALUE;
        numAttackers++;
    }

    int32_t bishopThreats = (b->pieceAttackBitboard[otherSide | BISHOP] & kingAttackZone).getNumberOfSetBits();
    if(bishopThreats > 0) {
        attackCounter += bishopThreats * MG_KING_SAFETY_BISHOP_THREAT_VALUE;
        numAttackers++;
    }

    int32_t rookThreats = (b->pieceAttackBitboard[otherSide | ROOK] & kingAttackZone).getNumberOfSetBits();
    if(rookThreats > 0) {
        attackCounter += rookThreats * MG_KING_SAFETY_ROOK_THREAT_VALUE;
        numAttackers++;
    }

    int32_t queenThreats = (b->pieceAttackBitboard[otherSide | QUEEN] & kingAttackZone).getNumberOfSetBits();
    if(queenThreats > 0) {
        attackCounter += queenThreats * MG_KING_SAFETY_QUEEN_THREAT_VALUE;
        numAttackers++;
    }

    if(numAttackers > 1)
        score -= kingSafetyTable[std::min(attackCounter, KING_SAFETY_TABLE_SIZE - 1)];

    return score;
}

Score BoardEvaluator::evalPawnStructure(Bitboard doublePawns, Bitboard isolatedPawns, Bitboard passedPawns, Bitboard pawnChains, Bitboard connectedPawns, int32_t side) {
    Score score{0, 0};

    // Doppelte Bauern bewerten
    score += evalDoublePawns(doublePawns, side);

    // Isolierte Bauern bewerten
    score += evalIsolatedPawns(isolatedPawns, side);

    // Freibauern(passed pawns) bewerten
    score += evalPassedPawns(passedPawns, side);

    // Bauernketten bewerten
    score += evalPawnChains(pawnChains, side);

    // Verbundene(nebeneinander stehende) Bauern bewerten
    score += evalConnectedPawns(connectedPawns, side);

    return score;
}

int32_t BoardEvaluator::evalMGKingSafety() {
    int32_t score = 0;
    int32_t side = b->side;
    int32_t otherSide = side ^ COLOR_MASK;

    Bitboard ownPawns = b->pieceBitboard[side | PAWN];
    Bitboard otherPawns = b->pieceBitboard[otherSide | PAWN];

    int32_t ownKingSafetyScore = 0;
    int32_t otherKingSafetyScore = 0;

    int32_t ownKingSquare = b->mailbox[b->pieceList[side | KING].front()];
    int32_t otherKingSquare = b->mailbox[b->pieceList[otherSide | KING].front()];

    ownKingSafetyScore += evalMGPawnShield(ownKingSquare, ownPawns, otherPawns, side);
    ownKingSafetyScore += evalMGPawnStorm(otherKingSquare, ownPawns, otherPawns, side);
    ownKingSafetyScore += evalMGKingAttackZone(side);

    otherKingSafetyScore += evalMGPawnShield(otherKingSquare, otherPawns, ownPawns, otherSide);
    otherKingSafetyScore += evalMGPawnStorm(ownKingSquare, otherPawns, ownPawns, otherSide);
    otherKingSafetyScore += evalMGKingAttackZone(otherSide);

    score += ownKingSafetyScore - otherKingSafetyScore;

    return score;
}

int32_t BoardEvaluator::evalMGMobility() {
    int32_t score = 0;
    int32_t side = b->side;
    int32_t otherSide = side ^ COLOR_MASK;

    Bitboard ownPieces = b->whitePiecesBitboard | b->pieceBitboard[WHITE_KING];
    Bitboard otherPieces = b->blackPiecesBitboard | b->pieceBitboard[BLACK_KING];

    if(side == BLACK) {
        ownPieces = b->blackPiecesBitboard | b->pieceBitboard[BLACK_KING];
        otherPieces = b->whitePiecesBitboard | b->pieceBitboard[WHITE_KING];
    }

    Bitboard ownPawnMobility = b->pieceAttackBitboard[side | PAWN] & ~ownPieces;
    Bitboard otherPawnMobility = b->pieceAttackBitboard[otherSide | PAWN] & ~otherPieces;
    Bitboard ownKnightMobility = b->pieceAttackBitboard[side | KNIGHT] & ~ownPieces & ~b->pieceAttackBitboard[otherSide | PAWN];
    Bitboard otherKnightMobility = b->pieceAttackBitboard[otherSide | KNIGHT] & ~otherPieces & ~b->pieceAttackBitboard[side | PAWN];
    Bitboard ownBishopMobility = b->pieceAttackBitboard[side | BISHOP] & ~ownPieces & ~b->pieceAttackBitboard[otherSide | PAWN];
    Bitboard otherBishopMobility = b->pieceAttackBitboard[otherSide | BISHOP] & ~otherPieces & ~b->pieceAttackBitboard[side | PAWN];
    Bitboard ownRookMobility = b->pieceAttackBitboard[side | ROOK] & ~ownPieces & ~b->pieceAttackBitboard[otherSide | PAWN];
    Bitboard otherRookMobility = b->pieceAttackBitboard[otherSide | ROOK] & ~otherPieces & ~b->pieceAttackBitboard[side | PAWN];
    Bitboard ownQueenMobility = b->pieceAttackBitboard[side | QUEEN] & ~ownPieces & ~b->pieceAttackBitboard[otherSide | PAWN];
    Bitboard otherQueenMobility = b->pieceAttackBitboard[otherSide | QUEEN] & ~otherPieces & ~b->pieceAttackBitboard[side | PAWN];

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

int32_t BoardEvaluator::evalEGMobility() {
    int32_t score = 0;
    int32_t side = b->side;
    int32_t otherSide = side ^ COLOR_MASK;

    Bitboard ownPieces = b->whitePiecesBitboard | b->pieceBitboard[WHITE_KING];
    Bitboard otherPieces = b->blackPiecesBitboard | b->pieceBitboard[BLACK_KING];

    if(side == BLACK) {
        ownPieces = b->blackPiecesBitboard | b->pieceBitboard[BLACK_KING];
        otherPieces = b->whitePiecesBitboard | b->pieceBitboard[WHITE_KING];
    }

    Bitboard ownPawnMobility = b->pieceAttackBitboard[side | PAWN] & ~ownPieces;
    Bitboard otherPawnMobility = b->pieceAttackBitboard[otherSide | PAWN] & ~otherPieces;
    Bitboard ownKnightMobility = b->pieceAttackBitboard[side | KNIGHT] & ~ownPieces & ~b->pieceAttackBitboard[otherSide | PAWN];
    Bitboard otherKnightMobility = b->pieceAttackBitboard[otherSide | KNIGHT] & ~otherPieces & ~b->pieceAttackBitboard[side | PAWN];
    Bitboard ownBishopMobility = b->pieceAttackBitboard[side | BISHOP] & ~ownPieces & ~b->pieceAttackBitboard[otherSide | PAWN];
    Bitboard otherBishopMobility = b->pieceAttackBitboard[otherSide | BISHOP] & ~otherPieces & ~b->pieceAttackBitboard[side | PAWN];
    Bitboard ownRookMobility = b->pieceAttackBitboard[side | ROOK] & ~ownPieces & ~b->pieceAttackBitboard[otherSide | PAWN];
    Bitboard otherRookMobility = b->pieceAttackBitboard[otherSide | ROOK] & ~otherPieces & ~b->pieceAttackBitboard[side | PAWN];
    Bitboard ownQueenMobility = b->pieceAttackBitboard[side | QUEEN] & ~ownPieces & ~b->pieceAttackBitboard[otherSide | PAWN];
    Bitboard otherQueenMobility = b->pieceAttackBitboard[otherSide | QUEEN] & ~otherPieces & ~b->pieceAttackBitboard[side | PAWN];

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

Bitboard BoardEvaluator::findDoublePawns(const Bitboard& ownPawns, int32_t side) {
    Bitboard doublePawns;
    Bitboard pawnCopy = ownPawns;

    while(pawnCopy) {
        int i = pawnCopy.getFirstSetBit();

        pawnCopy.clearBit(i);

        doublePawns |= doubledPawnMasks[side / 8][i] & ownPawns;
    }

    return doublePawns;
}

Bitboard BoardEvaluator::findIsolatedPawns(const Bitboard& ownPawns, int32_t side) {
    Bitboard isolatedPawns; 
    int32_t pawnForw = side == WHITE ? 8 : -8;
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

Bitboard BoardEvaluator::findPassedPawns(const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side) {
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

Bitboard BoardEvaluator::findPawnChains(const Bitboard& ownPawns, int32_t side) {
    Bitboard pawnChain;
    Bitboard pawnCopy = ownPawns;

    while(pawnCopy) {
        int i = pawnCopy.getFirstSetBit();
        pawnCopy.clearBit(i);

        pawnChain |= pawnChainMasks[side / 8][i] & ownPawns;
    }

    return pawnChain;
}

Bitboard BoardEvaluator::findConnectedPawns(const Bitboard& ownPawns) {
    Bitboard connectedPawns;
    Bitboard pawnCopy = ownPawns;

    while(pawnCopy) {
        int i = pawnCopy.getFirstSetBit();
        pawnCopy.clearBit(i);

        connectedPawns |= connectedPawnMasks[i] & ownPawns;
    }

    return connectedPawns;
}

int32_t BoardEvaluator::getSmallestAttacker(int32_t to, int32_t side) {
    int32_t smallestAttacker = EMPTY;
    int32_t otherSide = side ^ COLOR_MASK;
    int32_t to64 = b->mailbox[to];

    if(side == WHITE && !b->whiteAttackBitboard.getBit(to64))
        return NO_SQ;
    else if (side == BLACK && !b->blackAttackBitboard.getBit(to64))
        return NO_SQ;
    
    Bitboard pawnAttackers = pawnAttackBitboard(to64, otherSide) & b->pieceBitboard[side | PAWN];
    if(pawnAttackers)
        return b->mailbox64[pawnAttackers.getFirstSetBit()];

    Bitboard knightAttackers = knightAttackBitboard(to64) & b->pieceBitboard[side | KNIGHT];
    if(knightAttackers)
        return b->mailbox64[knightAttackers.getFirstSetBit()];

    Bitboard bishopAttackers = diagonalAttackBitboard(to64, b->allPiecesBitboard | b->pieceBitboard[side | KING])
                                                        & b->pieceBitboard[side | BISHOP];
    if(bishopAttackers)
        return b->mailbox64[bishopAttackers.getFirstSetBit()];

    Bitboard rookAttackers = straightAttackBitboard(to64, b->allPiecesBitboard | b->pieceBitboard[side | KING])
                                                        & b->pieceBitboard[side | ROOK];
    if(rookAttackers)
        return b->mailbox64[rookAttackers.getFirstSetBit()];

    Bitboard queenAttackers = (diagonalAttackBitboard(to64, b->allPiecesBitboard | b->pieceBitboard[side | KING])
                                | straightAttackBitboard(to64, b->allPiecesBitboard | b->pieceBitboard[side | KING]))
                                & b->pieceBitboard[side | QUEEN];
    if(queenAttackers)
        return b->mailbox64[queenAttackers.getFirstSetBit()];
    
    Bitboard kingAttackers = kingAttackBitboard(to64) & b->pieceBitboard[side | KING];
    if(kingAttackers) {
        // Der König darf nur schlagen, wenn die Figur nicht verteidigt wird
        if(side == WHITE && b->blackAttackBitboard.getBit(to64))
            return NO_SQ;
        else if(side == BLACK && b->whiteAttackBitboard.getBit(to64))
            return NO_SQ;

        return b->mailbox64[kingAttackers.getFirstSetBit()];
    }

    return NO_SQ;
}

int32_t BoardEvaluator::see(Move& m) {
    int32_t score = 0;
    int32_t side = b->side ^ COLOR_MASK;
    int32_t otherSide = b->side;

    b->makeMove(m);

    int32_t attackerSq = getSmallestAttacker(m.getDestination(), side);
    if(attackerSq != NO_SQ) {
        Move newMove(attackerSq, m.getDestination(), MOVE_CAPTURE);
        int32_t capturedPieceValue = MG_PIECE_VALUE[TYPEOF(b->pieces[m.getDestination()])];
        score = std::max(score, capturedPieceValue - see(newMove));
    }

    b->undoMove();

    return score;
}

int32_t BoardEvaluator::evaluateMoveSEE(Move& m) {
    int32_t moveScore = 0;

    if(m.isPromotion()) {
        if(m.isPromotionQueen())
            moveScore += PROMOTION_QUEEN_SCORE;
        else if(m.isPromotionRook())
            moveScore += PROMOTION_ROOK_SCORE;
        else if(m.isPromotionBishop())
            moveScore += PROMOTION_BISHOP_SCORE;
        else if(m.isPromotionKnight())
            moveScore += PROMOTION_KNIGHT_SCORE;
    }

    if(m.isCapture()) {
        // SEE-Heuristik
        int32_t capturedPieceValue = MG_PIECE_VALUE[TYPEOF(b->pieces[m.getDestination()])];
        moveScore += capturedPieceValue - see(m);
    }
    
    return moveScore;
}

int32_t BoardEvaluator::evaluateMoveMVVLVA(Move& m) {
    int32_t moveScore = 0;

    if(m.isPromotion()) {
        if(m.isPromotionQueen())
            moveScore += PROMOTION_QUEEN_SCORE;
        else if(m.isPromotionRook())
            moveScore += PROMOTION_ROOK_SCORE;
        else if(m.isPromotionBishop())
            moveScore += PROMOTION_BISHOP_SCORE;
        else if(m.isPromotionKnight())
            moveScore += PROMOTION_KNIGHT_SCORE;
    }

    if(m.isCapture()) {
        // MVVLVA-Heuristik
        int32_t movedPieceValue = MG_PIECE_VALUE[TYPEOF(b->pieces[m.getOrigin()])];
        int32_t capturedPieceValue = MG_PIECE_VALUE[TYPEOF(b->pieces[m.getDestination()])];

        moveScore += capturedPieceValue - movedPieceValue;
    }
    
    return moveScore;
}