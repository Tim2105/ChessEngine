#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "Board.h"
#include "BoardDefinitions.h"
#include "Move.h"
#include "Bitboard.h"
#include "Array.h"

class Board;

/**
 * Es gibt 3 Arten legale Züge zu generieren:
 * 
 * 1. Man generiert alle Pseudo-Legalen Züge, führt sie aus und prüft, ob ein Pseudo-Legaler Zug des Gegners den eigenen König schlägt.
 * (Pseudo-Legale Züge sind Züge, die die Regeln der Figurenbewegung einhalten aber eventuell den eigenen König im Schach stehen lassen)
 * Diese Implementierung, wenn zwar einfach, ist sehr ineffizient.  Für jeden Pseudo-Legalen Zug müssen alle Pseudo-Legalen Züge des Gegners generiert werden.
 * 
 * 2. Man generiert alle Pseudo-Legalen Züge, führt sie aus und überprüft ob der eigene König im Schach steht.
 * Diese Variante klingt ähnlich wie die erste Variante, ist aber effienzter.
 * Um zu überprüfen, ob der eigene König im Schach steht müssen nicht unbedingt alle Pseudo-Legalen Züge des Gegners generiert werden.
 * Es reicht vom eigenen König aus die möglichen Figurenbewegungen der gegnerischen Schachfiguren zu simulieren und zu überprüfen, ob man auf einer gegnerischen Figur landet.
 * Z.b. kann man von eigenen König aus die Springerangriffe simulieren und überprüfen, ob auf den Zielfeldern auch ein gegnerischer Springer steht.
 * Wenn ja, wird der eigene König von einem Springer angegriffen, andernfalls nicht. Nachdem man dieses Verfahren für alle 6 Figurentypen angewandt hat, weiß man ob ein König im Schach steht.
 * 
 * Dieses Vorgehen ist weitaus effizienter als Variante 1. Dennoch generiert man Pseudo-Legale Züge, die illegal sind.
 * Am optimalsten wäre es, von Anfang an nur legale Züge zu generieren.
 * 
 * 3. Ein Netz aus Verzweigungen und Sonderfällen wie hier https://peterellisjones.com/posts/generating-legal-chess-moves-efficiently/ beschrieben.
 * 
 * Fall 1: Der Zug ist ein Königszug.
 * 
 * Der eigene König darf sich nicht auf ein Feld bewegen, was von einer gegnerischen Figur angegriffen wird.
 * Um das effizient zu überprüfen wird zu Beginn der Zuggeneration ein Bitboard erstellt, in welchem alle Felder markiert sind, die von einer gegnerischen Figur angegriffen werden.
 * 
 * Fall 2: Der König steht im Schach.
 * 
 * Fall 2.1: Der König wird von 2 Figuren des Gegners angegriffen(double check).
 * 
 * Wenn der eigene König von 2 Figuren gleichzeitig angegriffen wird können nur Königszüge legal sein.
 * 
 * Fall 2.2: Der König wird von einer Figur des Gegners angegriffen.
 * 
 * Hier gibt es 3 Möglichkeiten:
 * - Der König bewegt sich aus dem Schach
 * - Der Angreifer wird geschlagen
 * - Der Angreifer(wenn Läufer, Turm oder Dame) wird blockiert
 * 
 * Zu Beginn der Zuggeneration wird überprüft, wie viele Figuren den König angreifen,
 * auf welchen Feldern die Figuren stehen und bei Läufer, Turm oder Dame welche Felder zwischen dem König und der Figur liegen.
 * 
 * (Gefesselte Figuren haben hier keine legalen Züge)
 * 
 * Fall 3: Die Figur ist gefesselt.
 * 
 * Eine Figur ist gefesselt, wenn sie sich nicht frei bewegen kann, ohne den eigenen König im Schach stehen zu lassen.
 * 
 * Hierfür werden ebenfalls zu Beginn der Zuggeneration die Figuren bestimmt, die gefesselt sind.
 * Zusätzlich wird die Richtung festgehalten, aus der sie gefesselt werden.
 * 
 * Für alle Figuren gibt es dann unterschiedliche Regeln.
 * 
 * Fall 4: Die Figur darf sich frei bewegen.
 * 
 * Hier sind alle Pseudo-Legalen Züge auch gleichzeitig legal(Ausnahme En-Passant!).
 * 
 * --------------------------------------------------------------------------------------------------------------------------------------
 * 
 * Zu jeder guten Zuggeneration gehört auch eine gute makeMove und unmakeMove Funktion.
 * 
 */

/**
 * @brief Die Klasse Movegen ist für die Generierung aller legalen und Pseudo-Legalen Züge für einzelne Figurenklassen zuständig.
 * Die Generierung von legalen Zügen ist sehr performancekritisch, weswegen keine Figuren in Klassen abstrahiert werden.
 */
class Movegen {

    public:
        /**
         * @brief Generiert alle Pseudo-Legalen Züge für die weißen Bauern.
         */
        static void generatePseudoLegalWhitePawnMoves(Array<Move, 256>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für die schwarzen Bauern.
         */
        static void generatePseudoLegalBlackPawnMoves(Array<Move, 256>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für die weißen Springer.
         */
        static void generatePseudoLegalWhiteKnightMoves(Array<Move, 256>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für die schwarzen Springer.
         */
        static void generatePseudoLegalBlackKnightMoves(Array<Move, 256>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für die weißen Läufer.
         */
        static void generatePseudoLegalWhiteBishopMoves(Array<Move, 256>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für die schwarzen Läufer.
         */
        static void generatePseudoLegalBlackBishopMoves(Array<Move, 256>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für die weißen Türme.
         */
        static void generatePseudoLegalWhiteRookMoves(Array<Move, 256>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für die schwarzen Türme.
         */
        static void generatePseudoLegalBlackRookMoves(Array<Move, 256>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für die weißen Damen.
         */
        static void generatePseudoLegalWhiteQueenMoves(Array<Move, 256>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für die schwarzen Damen.
         */
        static void generatePseudoLegalBlackQueenMoves(Array<Move, 256>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für den weißen König.
         */
        static void generatePseudoLegalWhiteKingMoves(Array<Move, 256>& moves, Board& board);

        /**
         * @brief Generiert alle Pseudo-Legalen Züge für den schwarzen König.
         */
        static void generatePseudoLegalBlackKingMoves(Array<Move, 256>& moves, Board& board);

        /**
         * @brief Generiert alle legalen Züge für die weißen Bauern.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         * @param pinnedDirections Array in 64er Notation, das für jedes Feld die Richtung angibt, in die die Figur darauf sie gefesselt ist(wenn existent).
         */
        static void generateWhitePawnMoves(Array<Move, 256>& moves, Board& board,
                                            int32_t numAttackers, Bitboard attackingRays,
                                            Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für die schwarzen Bauern.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         * @param pinnedDirections Array in 64er Notation, das für jedes Feld die Richtung angibt, in die die Figur darauf sie gefesselt ist(wenn existent).
         */
        static void generateBlackPawnMoves(Array<Move, 256>& moves, Board& board,
                                            int32_t numAttackers, Bitboard attackingRays,
                                            Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für die weißen Springer.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         */
        static void generateWhiteKnightMoves(Array<Move, 256>& moves, Board& board,
                                                int32_t numAttackers, Bitboard attackingRays,
                                                Bitboard pinnedPiecesBitboard);

        /**
         * @brief Generiert alle legalen Züge für die schwarzen Springer.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         */
        static void generateBlackKnightMoves(Array<Move, 256>& moves, Board& board,
                                                int32_t numAttackers, Bitboard attackingRays,
                                                Bitboard pinnedPiecesBitboard);

        /**
         * @brief Generiert alle legalen Züge für die weißen Läufer.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         * @param pinnedDirections Array in 64er Notation, das für jedes Feld die Richtung angibt, in die die Figur darauf sie gefesselt ist(wenn existent).
         */
        static void generateWhiteBishopMoves(Array<Move, 256>& moves, Board& board,
                                                int32_t numAttackers, Bitboard attackingRays,
                                                Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für die schwarzen Läufer.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         * @param pinnedDirections Array in 64er Notation, das für jedes Feld die Richtung angibt, in die die Figur darauf sie gefesselt ist(wenn existent).
         */
        static void generateBlackBishopMoves(Array<Move, 256>& moves, Board& board,
                                                int32_t numAttackers, Bitboard attackingRays,
                                                Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für die weißen Türme.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         * @param pinnedDirections Array in 64er Notation, das für jedes Feld die Richtung angibt, in die die Figur darauf sie gefesselt ist(wenn existent).
         */
        static void generateWhiteRookMoves(Array<Move, 256>& moves, Board& board,
                                            int32_t numAttackers, Bitboard attackingRays,
                                            Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für die schwarzen Türme.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         * @param pinnedDirections Array in 64er Notation, das für jedes Feld die Richtung angibt, in die die Figur darauf sie gefesselt ist(wenn existent).
         */
        static void generateBlackRookMoves(Array<Move, 256>& moves, Board& board,
                                            int32_t numAttackers, Bitboard attackingRays,
                                            Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für die weißen Damen.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         * @param pinnedDirections Array in 64er Notation, das für jedes Feld die Richtung angibt, in die die Figur darauf sie gefesselt ist(wenn existent).
         */
        static void generateWhiteQueenMoves(Array<Move, 256>& moves, Board& board,
                                            int32_t numAttackers, Bitboard attackingRays,
                                            Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für die schwarzen Damen.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         * @param pinnedDirections Array in 64er Notation, das für jedes Feld die Richtung angibt, in die die Figur darauf sie gefesselt ist(wenn existent).
         */
        static void generateBlackQueenMoves(Array<Move, 256>& moves, Board& board,
                                            int32_t numAttackers, Bitboard attackingRays,
                                            Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für den weißen König.
         * @param attackedSquares Bitboard, das alle Felder enthält, die die gegnerischen Figuren angreifen.
         */
        static void generateWhiteKingMoves(Array<Move, 256>& moves, Board& board, Bitboard attackedSquares);

        /**
         * @brief Generiert alle legalen Züge für den schwarzen König.
         * @param attackedSquares Bitboard, das alle Felder enthält, die die gegnerischen Figuren angreifen.
         */
        static void generateBlackKingMoves(Array<Move, 256>& moves, Board& board, Bitboard attackedSquares);
};

#endif