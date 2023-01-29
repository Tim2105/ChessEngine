#include "BoardEvaluator.h"
#include "BoardDefinitions.h"
#include "EvaluationDefinitions.h"
#include <algorithm>

BoardEvaluator::BoardEvaluator(Board& b) {
    this->b = &b;
}

BoardEvaluator::~BoardEvaluator() {
}

BoardEvaluator::BoardEvaluator(BoardEvaluator&& other) {
    this->b = other.b;
    this->pawnStructureTable = std::move(other.pawnStructureTable);
}

BoardEvaluator& BoardEvaluator::operator=(BoardEvaluator&& other) {
    this->b = other.b;
    this->pawnStructureTable = std::move(other.pawnStructureTable);

    return *this;
}

int32_t BoardEvaluator::evaluate() {
    if(isDraw()) // Unentschieden
        return 0;

    int32_t score = 0;
    int32_t side = b->side;
    int32_t otherSide = side ^ COLOR_MASK;

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

    double midgameWeight = 1 - phase;
    double endgameWeight = phase;
    
    score += middlegameEvaluation() * midgameWeight;
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

    return score;
}

int32_t BoardEvaluator::middlegameEvaluation() {
    int32_t score = 0;

    score += evalMaterial();
    score += evalMG_PSQT() * MG_PSQT_MULTIPLIER;
    score += evalMGKingSafety();
    score += evalMGCenterControl();

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

    int32_t materialScore = evalMaterial();

    if(materialScore > EG_WINNING_MATERIAL_ADVANTAGE) // Wenn man einen gewinnenden Materialvorteil hat
        score += (7 - distBetweenKings) * EG_KING_DISTANCE_VALUE;
    else if(-materialScore > EG_WINNING_MATERIAL_ADVANTAGE) // Wenn der Gegner einen gewinnenden Materialvorteil hat
        score -= (7 - distBetweenKings) * EG_KING_DISTANCE_VALUE;

    score += materialScore;
    score += evalEG_PSQT() * EG_PSQT_MULTIPLIER;
    score += evalEGKingSafety();

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

int32_t BoardEvaluator::evalMaterial() {
    int32_t score = 0;
    int32_t side = b->side;
    int32_t otherSide = side ^ COLOR_MASK;

    score += b->pieceList[side | PAWN].size() * PIECE_VALUE[PAWN];
    score += b->pieceList[side | KNIGHT].size() * PIECE_VALUE[KNIGHT];
    score += b->pieceList[side | BISHOP].size() * PIECE_VALUE[BISHOP];
    score += b->pieceList[side | ROOK].size() * PIECE_VALUE[ROOK];
    score += b->pieceList[side | QUEEN].size() * PIECE_VALUE[QUEEN];

    score -= b->pieceList[otherSide | PAWN].size() * PIECE_VALUE[PAWN];
    score -= b->pieceList[otherSide | KNIGHT].size() * PIECE_VALUE[KNIGHT];
    score -= b->pieceList[otherSide | BISHOP].size() * PIECE_VALUE[BISHOP];
    score -= b->pieceList[otherSide | ROOK].size() * PIECE_VALUE[ROOK];
    score -= b->pieceList[otherSide | QUEEN].size() * PIECE_VALUE[QUEEN];

    return score;
}

inline int32_t BoardEvaluator::evalMG_PSQT() {
    int32_t score = 0;
    int32_t whiteScore = 0;
    int32_t blackScore = 0;

    for(int p = PAWN; p <= KING; p++) {
        for(int sq : b->pieceList[WHITE | p]) {
            whiteScore += MG_PSQT[p][b->mailbox[sq]];
        }
        
        for(int sq : b->pieceList[BLACK | p]) {
            int rank = SQ2R(sq);
            int file = SQ2F(sq);
            blackScore += MG_PSQT[p][(RANK_8 - rank) * 8 + file];
        }
    }

    if(b->side == WHITE) {
        score += whiteScore;
        score -= blackScore;
    } else {
        score += blackScore;
        score -= whiteScore;
    }

    return score;
}

inline int32_t BoardEvaluator::evalEG_PSQT() {
    int32_t score = 0;
    int32_t whiteScore = 0;
    int32_t blackScore = 0;

    for(int p = PAWN; p <= KING; p++) {
        for(int sq : b->pieceList[WHITE | p]) {
            whiteScore += EG_PSQT[p][b->mailbox[sq]];
        }
        
        for(int sq : b->pieceList[BLACK | p]) {
            int rank = SQ2R(sq);
            int file = SQ2F(sq);
            blackScore += EG_PSQT[p][(RANK_8 - rank) * 8 + file];
        }
    }

    if(b->side == WHITE) {
        score += whiteScore;
        score -= blackScore;
    } else {
        score += blackScore;
        score -= whiteScore;
    }

    return score;
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
    Bitboard pawnShieldSquares;

    int rank = kingSquare / 8;
    int file = kingSquare % 8;

    if(file == FILE_D || file == FILE_E)
        return 0;

    if(side == WHITE) {
        if(rank < RANK_5) {
            pawnShieldSquares.setBit(kingSquare + 8);
            pawnShieldSquares.setBit(kingSquare + 16);

            if(file != FILE_A) {
                pawnShieldSquares.setBit(kingSquare - 1);
                pawnShieldSquares.setBit(kingSquare + 7);
                pawnShieldSquares.setBit(kingSquare + 15);
            }

            if(file != FILE_H) {
                pawnShieldSquares.setBit(kingSquare + 1);
                pawnShieldSquares.setBit(kingSquare + 9);
                pawnShieldSquares.setBit(kingSquare + 17);
            }
        }
    } else {
        if(rank > RANK_4) {
            pawnShieldSquares.setBit(kingSquare - 8);
            pawnShieldSquares.setBit(kingSquare - 16);

            if(file != FILE_A) {
                pawnShieldSquares.setBit(kingSquare - 17);
                pawnShieldSquares.setBit(kingSquare - 9);
                pawnShieldSquares.setBit(kingSquare - 1);
            }

            if(file != FILE_H) {
                pawnShieldSquares.setBit(kingSquare - 15);
                pawnShieldSquares.setBit(kingSquare - 7);
                pawnShieldSquares.setBit(kingSquare + 1);
            }
        }
    }

    return (pawnShieldSquares & ownPawns).getNumberOfSetBits() * MG_PAWN_SHIELD_VALUE;
}

inline int32_t BoardEvaluator::evalMGPawnStorm(int32_t otherKingSquare, const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side) {
    int score = 0;

    int destBackw = side == WHITE ? -8 : 8;

    int rank = otherKingSquare / 8;
    int file = otherKingSquare % 8;

    for(int i = otherKingSquare + destBackw; i < 64 && i >= 0; i += destBackw) {
        if(ownPawns.getBit(i)) {
            int advancedRanks = i / 8;
            if(side == BLACK)
                advancedRanks = 7 - advancedRanks;

            advancedRanks--;
            score += MG_PAWN_STORM_BASE_VALUE + advancedRanks * MG_PAWN_STORM_DISTANCE_MULTIPLIER;
            break;
        }
    }
        
    
    if(file != FILE_A) {
        for(int i = otherKingSquare + destBackw - 1; i < 64 && i >= 0; i += destBackw) {
            if(ownPawns.getBit(i)) {
                int advancedRanks = i / 8;
                if(side == BLACK)
                    advancedRanks = 7 - advancedRanks;

                advancedRanks--;
                score += MG_PAWN_STORM_BASE_VALUE + advancedRanks * MG_PAWN_STORM_DISTANCE_MULTIPLIER;
                break;
            }
        }
    }

    if(file != FILE_H) {
        for(int i = otherKingSquare + destBackw + 1; i < 64 && i >= 0; i += destBackw) {
            if(ownPawns.getBit(i)) {
                int advancedRanks = i / 8;
                if(side == BLACK)
                    advancedRanks = 7 - advancedRanks;

                advancedRanks--;
                score += MG_PAWN_STORM_BASE_VALUE + advancedRanks * MG_PAWN_STORM_DISTANCE_MULTIPLIER;
                break;
            }
        }
    }

    return score;
}

inline int32_t BoardEvaluator::evalMGKingMobility(int32_t side) {
    Bitboard enemyAttackedSquares;

    if(side == WHITE)
        enemyAttackedSquares = b->blackAttackBitboard;
    else
        enemyAttackedSquares = b->whiteAttackBitboard;

    int32_t ownKingSquare = b->mailbox[b->pieceList[side | KING].front()];

    Bitboard kingMoves = kingAttackBitboard(ownKingSquare) & ~enemyAttackedSquares;

    return (8 - kingMoves.getNumberOfSetBits()) * MG_KING_SAFETY_VALUE;
}

inline int32_t BoardEvaluator::evalEGKingMobility(int32_t side) {
    Bitboard enemyAttackedSquares;

    if(side == WHITE)
        enemyAttackedSquares = b->blackAttackBitboard;
    else
        enemyAttackedSquares = b->whiteAttackBitboard;

    int32_t ownKingSquare = b->mailbox[b->pieceList[side | KING].front()];

    Bitboard kingMoves = kingAttackBitboard(ownKingSquare) & ~enemyAttackedSquares;

    return (8 - kingMoves.getNumberOfSetBits()) * EG_KING_SAFETY_VALUE;
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
    ownKingSafetyScore -= evalMGPawnStorm(ownKingSquare, otherPawns, ownPawns, otherSide);
    ownKingSafetyScore += evalMGKingMobility(side);

    otherKingSafetyScore += evalMGPawnShield(otherKingSquare, otherPawns, ownPawns, otherSide);
    otherKingSafetyScore -= evalMGPawnStorm(otherKingSquare, ownPawns, otherPawns, side);
    otherKingSafetyScore += evalMGKingMobility(otherSide);

    score += ownKingSafetyScore - otherKingSafetyScore;

    return score;
}

int32_t BoardEvaluator::evalEGKingSafety() {
    int32_t score = 0;
    int32_t side = b->side;
    int32_t otherSide = side ^ COLOR_MASK;

    int32_t ownKingSafetyScore = evalEGKingMobility(side);
    int32_t otherKingSafetyScore = evalEGKingMobility(otherSide);

    score += ownKingSafetyScore - otherKingSafetyScore;

    return score;
}

inline int32_t BoardEvaluator::evalMGCenterControl() {
    int32_t score = 0;
    int32_t side = b->side;
    int32_t otherSide = side ^ COLOR_MASK;

    Bitboard ownPawns = b->pieceBitboard[side | PAWN];
    Bitboard otherPawns = b->pieceBitboard[otherSide | PAWN];

    Bitboard ownCenter = ownPawns & centerSquares;
    Bitboard otherCenter = otherPawns & centerSquares;

    score += ownCenter.getNumberOfSetBits() * MG_CENTER_CONTROL_VALUE;
    score -= otherCenter.getNumberOfSetBits() * MG_CENTER_CONTROL_VALUE;

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
        int32_t capturedPieceValue = PIECE_VALUE[TYPEOF(b->pieces[m.getDestination()])];
        score = std::max(score, capturedPieceValue - see(newMove));
    }

    b->undoMove();

    return score;
}

int32_t BoardEvaluator::evaluateMove(Move& m) {
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
        int32_t capturedPieceValue = PIECE_VALUE[TYPEOF(b->pieces[m.getDestination()])];
        moveScore += capturedPieceValue - see(m);
    }
    
    return moveScore;
}