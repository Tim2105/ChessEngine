#ifndef BOARD_EVALUATOR_H
#define BOARD_EVALUATOR_H

#include "Board.h"
#include "HashTable.h"

class BoardEvaluator {

    private:
    public:
        HashTable<Bitboard, int32_t, 256, 4> whitePawnStructureTable;
        HashTable<Bitboard, int32_t, 256, 4> blackPawnStructureTable;

        int32_t evalMaterial(const Board& b);
        int32_t evalMobility(const Board& b);
        int32_t evalPawnStructure(const Board& b);
        int32_t evalPawnStructure(const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t ownKingSquare, int32_t otherKingSquare, int32_t side);
        inline int32_t evalDoublePawns(const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side);
        inline int32_t evalIsolatedPawns(const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side);
        inline int32_t evalPassedPawns(const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side);
        inline int32_t evalPawnChains(const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side);
        inline int32_t evalConnectedPawns(const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side);
        
        int32_t evalKingSafety(const Board& b);
        inline int32_t evalPawnShield(int32_t kingSquare, const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side);
        inline int32_t evalPawnStorm(int32_t otherKingSquare, const Bitboard& ownPawns, const Bitboard& otherPawns, int32_t side);
        inline int32_t evalKingMobility(const Board& b, int32_t side);

    public:
        int32_t middlegameEvaluation(const Board& b);
        int32_t evaluate(const Board& b);
};

#endif