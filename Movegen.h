#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "Board.h"
#include "Definitions.h"
#include "Move.h"
#include "Bitboard.h"
#include <vector>

class Board;

/**
 * @brief Die Klasse Movegen ist für die Generierung aller legalen und pseudo-legalen Züge für einzelne Figurenklassen zuständig.
 * Die Generierung von legalen Zügen ist sehr performancekritisch, deshalb werden abstrahierende objektorientierte Prinzipien missachtet um höchstmöglich Performance zu erzielen.
 */
class Movegen {

    public:
        /**
         * @brief Generiert alle Pseudo-Legalen Züge für die weißen Bauern.
         */
        static void generatePseudoLegalWhitePawnMoves(std::vector<Move>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für die schwarzen Bauern.
         */
        static void generatePseudoLegalBlackPawnMoves(std::vector<Move>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für die weißen Springer.
         */
        static void generatePseudoLegalWhiteKnightMoves(std::vector<Move>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für die schwarzen Springer.
         */
        static void generatePseudoLegalBlackKnightMoves(std::vector<Move>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für die weißen Läufer.
         */
        static void generatePseudoLegalWhiteBishopMoves(std::vector<Move>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für die schwarzen Läufer.
         */
        static void generatePseudoLegalBlackBishopMoves(std::vector<Move>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für die weißen Türme.
         */
        static void generatePseudoLegalWhiteRookMoves(std::vector<Move>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für die schwarzen Türme.
         */
        static void generatePseudoLegalBlackRookMoves(std::vector<Move>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für die weißen Damen.
         */
        static void generatePseudoLegalWhiteQueenMoves(std::vector<Move>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für die schwarzen Damen.
         */
        static void generatePseudoLegalBlackQueenMoves(std::vector<Move>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für den weißen König.
         */
        static void generatePseudoLegalWhiteKingMoves(std::vector<Move>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für den schwarzen König.
         */
        static void generatePseudoLegalBlackKingMoves(std::vector<Move>& moves, Board& board);

        /**
         * @brief Generiert alle legalen Züge für die weißen Bauern.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         * @param pinnedDirections Array in 64er Notation, das für jedes Feld die Richtung angibt, in die die Figur darauf sie gefesselt ist(wenn existent).
         */
        static void generateWhitePawnMoves(std::vector<Move>& moves, Board& board,
                                            int32_t numAttackers, Bitboard attackingRays,
                                            Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für die schwarzen Bauern.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         * @param pinnedDirections Array in 64er Notation, das für jedes Feld die Richtung angibt, in die die Figur darauf sie gefesselt ist(wenn existent).
         */
        static void generateBlackPawnMoves(std::vector<Move>& moves, Board& board,
                                            int32_t numAttackers, Bitboard attackingRays,
                                            Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für die weißen Springer.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         */
        static void generateWhiteKnightMoves(std::vector<Move>& moves, Board& board,
                                                int32_t numAttackers, Bitboard attackingRays,
                                                Bitboard pinnedPiecesBitboard);

        /**
         * @brief Generiert alle legalen Züge für die schwarzen Springer.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         */
        static void generateBlackKnightMoves(std::vector<Move>& moves, Board& board,
                                                int32_t numAttackers, Bitboard attackingRays,
                                                Bitboard pinnedPiecesBitboard);

        /**
         * @brief Generiert alle legalen Züge für die weißen Läufer.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         * @param pinnedDirections Array in 64er Notation, das für jedes Feld die Richtung angibt, in die die Figur darauf sie gefesselt ist(wenn existent).
         */
        static void generateWhiteBishopMoves(std::vector<Move>& moves, Board& board,
                                                int32_t numAttackers, Bitboard attackingRays,
                                                Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für die schwarzen Läufer.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         * @param pinnedDirections Array in 64er Notation, das für jedes Feld die Richtung angibt, in die die Figur darauf sie gefesselt ist(wenn existent).
         */
        static void generateBlackBishopMoves(std::vector<Move>& moves, Board& board,
                                                int32_t numAttackers, Bitboard attackingRays,
                                                Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für die weißen Türme.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         * @param pinnedDirections Array in 64er Notation, das für jedes Feld die Richtung angibt, in die die Figur darauf sie gefesselt ist(wenn existent).
         */
        static void generateWhiteRookMoves(std::vector<Move>& moves, Board& board,
                                            int32_t numAttackers, Bitboard attackingRays,
                                            Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für die schwarzen Türme.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         * @param pinnedDirections Array in 64er Notation, das für jedes Feld die Richtung angibt, in die die Figur darauf sie gefesselt ist(wenn existent).
         */
        static void generateBlackRookMoves(std::vector<Move>& moves, Board& board,
                                            int32_t numAttackers, Bitboard attackingRays,
                                            Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für die weißen Damen.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         * @param pinnedDirections Array in 64er Notation, das für jedes Feld die Richtung angibt, in die die Figur darauf sie gefesselt ist(wenn existent).
         */
        static void generateWhiteQueenMoves(std::vector<Move>& moves, Board& board,
                                            int32_t numAttackers, Bitboard attackingRays,
                                            Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für die schwarzen Damen.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         * @param pinnedDirections Array in 64er Notation, das für jedes Feld die Richtung angibt, in die die Figur darauf sie gefesselt ist(wenn existent).
         */
        static void generateBlackQueenMoves(std::vector<Move>& moves, Board& board,
                                            int32_t numAttackers, Bitboard attackingRays,
                                            Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für den weißen König.
         * @param attackedSquares Bitboard, das alle Felder enthält, die die gegnerischen Figuren angreifen.
         */
        static void generateWhiteKingMoves(std::vector<Move>& moves, Board& board, Bitboard attackedSquares);

        /**
         * @brief Generiert alle legalen Züge für den schwarzen König.
         * @param attackedSquares Bitboard, das alle Felder enthält, die die gegnerischen Figuren angreifen.
         */
        static void generateBlackKingMoves(std::vector<Move>& moves, Board& board, Bitboard attackedSquares);
};

#endif