#include "BoardEvaluator.h"
#include "BoardDefinitions.h"
#include "EvaluationDefinitions.h"
#include <algorithm>

int32_t BoardEvaluator::evalMaterial(const Board& b) {
    int32_t score = 0;
    int32_t side = b.side;
    int32_t otherSide = side ^ COLOR_MASK;

    score += b.pieceList[side | PAWN].size() * PIECE_VALUE[PAWN];
    score += b.pieceList[side | KNIGHT].size() * PIECE_VALUE[KNIGHT];
    score += b.pieceList[side | BISHOP].size() * PIECE_VALUE[BISHOP];
    score += b.pieceList[side | ROOK].size() * PIECE_VALUE[ROOK];
    score += b.pieceList[side | QUEEN].size() * PIECE_VALUE[QUEEN];

    score -= b.pieceList[otherSide | PAWN].size() * PIECE_VALUE[PAWN];
    score -= b.pieceList[otherSide | KNIGHT].size() * PIECE_VALUE[KNIGHT];
    score -= b.pieceList[otherSide | BISHOP].size() * PIECE_VALUE[BISHOP];
    score -= b.pieceList[otherSide | ROOK].size() * PIECE_VALUE[ROOK];
    score -= b.pieceList[otherSide | QUEEN].size() * PIECE_VALUE[QUEEN];

    return score;
}

int32_t BoardEvaluator::evalMobility(const Board& b) {
    int32_t score = 0;
    int32_t side = b.side;

    if(side == WHITE) {
        score += b.whiteAttackBitboard.getNumberOfSetBits();
        score -= b.blackAttackBitboard.getNumberOfSetBits();
    } else {
        score += b.blackAttackBitboard.getNumberOfSetBits();
        score -= b.whiteAttackBitboard.getNumberOfSetBits();
    }

    return score;
}

int32_t BoardEvaluator::evalPawnStructure(const Board& b) {
    int32_t score = 0;
    int32_t side = b.side;
    int32_t otherSide = side ^ COLOR_MASK;

    Bitboard ownPawns = b.pieceBitboard[side | PAWN];
    Bitboard otherPawns = b.pieceBitboard[otherSide | PAWN];

    int32_t ownPawnStructureScore = 0;
    int32_t otherPawnStructureScore = 0;

    bool ownPawnStructureFound, otherPawnStructureFound;

    // Überprüfe, ob die Struktur der Bauern in der Hash-Tabelle vorhanden ist
    if(side == WHITE) {
        ownPawnStructureFound = whitePawnStructureTable.get(ownPawns, ownPawnStructureScore);
        otherPawnStructureFound = blackPawnStructureTable.get(otherPawns, otherPawnStructureScore);
    } else {
        ownPawnStructureFound = blackPawnStructureTable.get(ownPawns, ownPawnStructureScore);
        otherPawnStructureFound = whitePawnStructureTable.get(otherPawns, otherPawnStructureScore);
    }
    

    if(ownPawnStructureFound) {
        score += ownPawnStructureScore;
    } else {
        ownPawnStructureScore += evalPawnStructure(
                ownPawns, otherPawns, b.mailbox[b.pieceList[side | KING].front()],
                b.mailbox[b.pieceList[otherSide | KING].front()], side);

        score += ownPawnStructureScore;
    }

    if(otherPawnStructureFound) {
        score -= otherPawnStructureScore;
    } else {
        otherPawnStructureScore += evalPawnStructure(
                otherPawns, ownPawns, b.mailbox[b.pieceList[otherSide | KING].front()],
                b.mailbox[b.pieceList[side | KING].front()], otherSide);

        score -= otherPawnStructureScore;
    }

    // Speichere die Bewertung für die Struktur der Bauern in der Hash-Tabelle
    if(side == WHITE) {
        whitePawnStructureTable.put(ownPawns, ownPawnStructureScore);
        blackPawnStructureTable.put(otherPawns, otherPawnStructureScore);
    } else {
        blackPawnStructureTable.put(ownPawns, ownPawnStructureScore);
        whitePawnStructureTable.put(otherPawns, otherPawnStructureScore);
    }

    return score;
}

inline int32_t BoardEvaluator::evalDoublePawns(const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side) {
    int32_t score = 0;

    int32_t pawnForw = side == WHITE ? 8 : -8;

    Bitboard pawnCopy = ownPawns;
    while(pawnCopy) {
        int i;
        if(side == WHITE)
            i = pawnCopy.getFirstSetBit();
        else
            i = pawnCopy.getLastSetBit();

        pawnCopy.clearBit(i);

        for(int j = i + pawnForw; j < 64 && j >= 0; j += pawnForw) {
            if(pawnCopy.getBit(j)) {
                score += PAWN_DOUBLED_VALUE;
                pawnCopy.clearBit(j);
            }
        }
    }

    return score;
}

inline int32_t BoardEvaluator::evalIsolatedPawns(const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side) {
    int32_t score = 0;

    int32_t pawnForw = side == WHITE ? 8 : -8;

    Bitboard pawnCopy = ownPawns;
    while(pawnCopy) {
        int i = pawnCopy.getFirstSetBit();
        pawnCopy.clearBit(i);

        int file = i % 8;

        if(!(ownPawns & neighboringFiles[file])) {
            score += PAWN_ISOLATED_BASE_VALUE;

            int distToOuterFile = std::min(file, 7 - file);
            score += distToOuterFile * PAWN_ISOLATED_INNER_FILE_MULTIPLIER;
        }
    }

    return score;
}

inline int32_t BoardEvaluator::evalPassedPawns(const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side) {
    int32_t score = 0;

    int32_t pawnForw = side == WHITE ? 8 : -8;

    Bitboard pawnCopy = ownPawns;
    while(pawnCopy) {
        int i = pawnCopy.getFirstSetBit();
        pawnCopy.clearBit(i);

        int advancedRanks = i / 8;
        if(side == BLACK)
            advancedRanks = 7 - advancedRanks;
        
        advancedRanks--;

        int file = i % 8;

        int passedPawnValue = PAWN_PASSED_BASE_VALUE + advancedRanks * PAWN_PASSED_RANK_ADVANCED_MULTIPLIER;

        score += passedPawnValue;

        Array<int, 8> fileOffsets = {0};

        if(file != FILE_A)
            fileOffsets.push_back(-1);
        
        if(file != FILE_H)
            fileOffsets.push_back(1);
        
        for(int j = i + pawnForw; j < 64 && j >= 0; j += pawnForw) {
            if(ownPawns.getBit(j)) {
                // Bauer ist kein Freibauer, da er von einem eigenen Bauer blockiert wird
                score -= passedPawnValue;
                break;
            }

            for(int offset : fileOffsets) {
                if(otherPawns.getBit(j + offset)) {
                    // Bauer ist kein Freibauer, da er von einem gegnerischen Bauer blockiert wird
                    score -= passedPawnValue;
                    break;
                }
            }
        }
    }

    return score;
}

inline int32_t BoardEvaluator::evalPawnChains(const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side) {
    Bitboard pawnChain;

    if(side == WHITE) {
        pawnChain = (ownPawns & ownPawns << 7) | (ownPawns & ownPawns << 9);
    } else {
        pawnChain = (ownPawns & ownPawns >> 7) | (ownPawns & ownPawns >> 9);
    }

    return pawnChain.getNumberOfSetBits() * PAWN_CHAIN_VALUE;
}

inline int32_t BoardEvaluator::evalConnectedPawns(const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side) {
    int32_t score = 0;

    int32_t pawnForw = side == WHITE ? 8 : -8;

    Bitboard connectedPawns = (ownPawns & ownPawns << 1) | (ownPawns & ownPawns >> 1);

    while(connectedPawns) {
        int32_t index = connectedPawns.getFirstSetBit();
        connectedPawns.clearBit(index);
        
        if(side == BLACK)
            index = 63 - index;
        
        int32_t ranksAdvanced = index / 8 - 1;

        score += PAWN_CONNECTED_BASE_VALUE + ranksAdvanced * PAWN_CONNECTED_RANK_ADVANCED_MULTIPLIER;
    }

    return score;
}

inline int32_t BoardEvaluator::evalPawnShield(int32_t kingSquare, const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side) {
    Bitboard pawnShieldSquares;

    int rank = kingSquare / 8;
    int file = kingSquare % 8;

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

    return (pawnShieldSquares & ownPawns).getNumberOfSetBits() * PAWN_SHIELD_VALUE;
}

inline int32_t BoardEvaluator::evalPawnStorm(int32_t otherKingSquare, const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side) {
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
            score += PAWN_STORM_BASE_VALUE + advancedRanks * PAWN_STORM_DISTANCE_MULTIPLIER;
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
                score += PAWN_STORM_BASE_VALUE + advancedRanks * PAWN_STORM_DISTANCE_MULTIPLIER;
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
                score += PAWN_STORM_BASE_VALUE + advancedRanks * PAWN_STORM_DISTANCE_MULTIPLIER;
                break;
            }
        }
    }

    return score;
}

inline int32_t BoardEvaluator::evalKingMobility(const Board& b, int32_t side) {
    Bitboard enemyAttackedSquares;

    if(side == WHITE)
        enemyAttackedSquares = b.blackAttackBitboard;
    else
        enemyAttackedSquares = b.whiteAttackBitboard;

    int32_t ownKingSquare = b.mailbox[b.pieceList[side | KING].front()];

    Bitboard kingMoves = kingAttackBitboard(ownKingSquare) & ~enemyAttackedSquares;

    return (8 - kingMoves.getNumberOfSetBits()) * KING_SAFETY_VALUE;
}

int32_t BoardEvaluator::evalPawnStructure(const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t ownKingSquare, int32_t otherKingSquare, int32_t side) {
    int32_t score = 0;

    // Doppelte Bauern bewerten
    score += evalDoublePawns(ownPawns, otherPawns, side);

    // Isolierte Bauern bewerten
    score += evalIsolatedPawns(ownPawns, otherPawns, side);

    // Freibauern(passed pawns) bewerten
    score += evalPassedPawns(ownPawns, otherPawns, side);

    // Bauernketten bewerten
    score += evalPawnChains(ownPawns, otherPawns, side);

    // Verbundene(nebeneinander stehende) Bauern bewerten
    score += evalConnectedPawns(ownPawns, otherPawns, side);

    return score;
}

int32_t BoardEvaluator::evalKingSafety(const Board& b) {
    int32_t score = 0;
    int32_t side = b.side;
    int32_t otherSide = side ^ COLOR_MASK;

    Bitboard ownPawns = b.pieceBitboard[side | PAWN];
    Bitboard otherPawns = b.pieceBitboard[otherSide | PAWN];

    int32_t ownKingSafetyScore = 0;
    int32_t otherKingSafetyScore = 0;

    int32_t ownKingSquare = b.mailbox[b.pieceList[side | KING].front()];
    int32_t otherKingSquare = b.mailbox[b.pieceList[otherSide | KING].front()];

    ownKingSafetyScore += evalPawnShield(ownKingSquare, ownPawns, otherPawns, side);
    ownKingSafetyScore -= evalPawnStorm(ownKingSquare, otherPawns, ownPawns, otherSide);
    ownKingSafetyScore += evalKingMobility(b, side);

    otherKingSafetyScore += evalPawnShield(otherKingSquare, otherPawns, ownPawns, otherSide);
    otherKingSafetyScore -= evalPawnStorm(otherKingSquare, ownPawns, otherPawns, side);
    otherKingSafetyScore += evalKingMobility(b, otherSide);

    score += ownKingSafetyScore - otherKingSafetyScore;

    return score;
}

int32_t BoardEvaluator::middlegameEvaluation(const Board& b) {
    int32_t score = 0;

    score += evalMaterial(b);
    score += evalMobility(b) * MOBILITY_VALUE;
    score += evalPawnStructure(b);
    score += evalKingSafety(b);

    return score;
}

int32_t BoardEvaluator::evaluate(const Board& b) {
    int32_t score = 0;
    
    score += middlegameEvaluation(b);

    return score;
}