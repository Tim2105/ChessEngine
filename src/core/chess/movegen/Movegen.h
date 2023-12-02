#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "core/chess/Board.h"
#include "core/chess/BoardDefinitions.h"
#include "core/chess/Move.h"
#include "core/utils/Array.h"
#include "core/utils/Bitboard.h"

/**
 * Es gibt 3 Arten legale Züge zu generieren:
 * 
 * 1. Man generiert alle Pseudo-Legalen Züge, führt sie aus und prüft, ob ein Pseudo-Legaler Zug des Gegners den eigenen König schlägt.
 * (Pseudo-Legale Züge sind Züge, die die Regeln der Figurenbewegung einhalten aber eventuell den eigenen König im Schach stehen lassen)
 * Diese Implementierung, wenn zwar einfach, ist sehr ineffizient. Für jeden Pseudo-Legalen Zug müssen alle Pseudo-Legalen Züge des Gegners generiert werden.
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
 * 
 * Zur Generierung von Turm-, Läufer- und Damenzügen werden Magic-Bitboards verwendet.
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
        template <Array<uint8_t, 10> moveTypes = Array<uint8_t, 10>{ALL_MOVES}>
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
        template <Array<uint8_t, 10> moveTypes = Array<uint8_t, 10>{ALL_MOVES}>
        static void generateBlackPawnMoves(Array<Move, 256>& moves, Board& board,
                                            int32_t numAttackers, Bitboard attackingRays,
                                            Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für die weißen Springer.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         */
        template <Array<uint8_t, 10> moveTypes = Array<uint8_t, 10>{ALL_MOVES}>
        static void generateWhiteKnightMoves(Array<Move, 256>& moves, Board& board,
                                                int32_t numAttackers, Bitboard attackingRays,
                                                Bitboard pinnedPiecesBitboard);

        /**
         * @brief Generiert alle legalen Züge für die schwarzen Springer.
         * @param numAttackers Gibt an, wie viele gegnerische Figuren den eigenen König angreifen.
         * @param attackingRays Bitboard, das alle Diagonalen und Geraden enthält, auf denen der eigene König angegriffen wird.
         * @param pinnedPiecesBitboard Bitboard, das alle gefesselten Figuren enthält.
         */
        template <Array<uint8_t, 10> moveTypes = Array<uint8_t, 10>{ALL_MOVES}>
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
        template <Array<uint8_t, 10> moveTypes = Array<uint8_t, 10>{ALL_MOVES}>
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
        template <Array<uint8_t, 10> moveTypes = Array<uint8_t, 10>{ALL_MOVES}>
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
        template <Array<uint8_t, 10> moveTypes = Array<uint8_t, 10>{ALL_MOVES}>
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
        template <Array<uint8_t, 10> moveTypes = Array<uint8_t, 10>{ALL_MOVES}>
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
        template <Array<uint8_t, 10> moveTypes = Array<uint8_t, 10>{ALL_MOVES}>
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
        template <Array<uint8_t, 10> moveTypes = Array<uint8_t, 10>{ALL_MOVES}>
        static void generateBlackQueenMoves(Array<Move, 256>& moves, Board& board,
                                            int32_t numAttackers, Bitboard attackingRays,
                                            Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für den weißen König.
         * @param attackedSquares Bitboard, das alle Felder enthält, die die gegnerischen Figuren angreifen.
         */
        template <Array<uint8_t, 10> moveTypes = Array<uint8_t, 10>{ALL_MOVES}>
        static void generateWhiteKingMoves(Array<Move, 256>& moves, Board& board, Bitboard attackedSquares);

        /**
         * @brief Generiert alle legalen Züge für den schwarzen König.
         * @param attackedSquares Bitboard, das alle Felder enthält, die die gegnerischen Figuren angreifen.
         */
        template <Array<uint8_t, 10> moveTypes = Array<uint8_t, 10>{ALL_MOVES}>
        static void generateBlackKingMoves(Array<Move, 256>& moves, Board& board, Bitboard attackedSquares);

        /**
         * @brief Generiert alle legalen Schlagzüge für die weißen Bauern.
         */
        static void generateWhitePawnCaptures(Array<Move, 256>& moves, Board& board,
                                            int32_t numAttackers, Bitboard attackingRays,
                                            Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Schlagzüge für die schwarzen Bauern.
         */
        static void generateBlackPawnCaptures(Array<Move, 256>& moves, Board& board,
                                            int32_t numAttackers, Bitboard attackingRays,
                                            Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für die weißen Springer.
         */
        static void generateWhiteKnightCaptures(Array<Move, 256>& moves, Board& board,
                                                int32_t numAttackers, Bitboard attackingRays,
                                                Bitboard pinnedPiecesBitboard);

        /**
         * @brief Generiert alle legalen Züge für die schwarzen Springer.
         */
        static void generateBlackKnightCaptures(Array<Move, 256>& moves, Board& board,
                                                int32_t numAttackers, Bitboard attackingRays,
                                                Bitboard pinnedPiecesBitboard);

        /**
         * @brief Generiert alle legalen Züge für die weißen Läufer.
         */
        static void generateWhiteBishopCaptures(Array<Move, 256>& moves, Board& board,
                                                int32_t numAttackers, Bitboard attackingRays,
                                                Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für die schwarzen Läufer.
         */
        static void generateBlackBishopCaptures(Array<Move, 256>& moves, Board& board,
                                                int32_t numAttackers, Bitboard attackingRays,
                                                Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für die weißen Türme.
         */
        static void generateWhiteRookCaptures(Array<Move, 256>& moves, Board& board,
                                                int32_t numAttackers, Bitboard attackingRays,
                                                Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für die schwarzen Türme.
         */
        static void generateBlackRookCaptures(Array<Move, 256>& moves, Board& board,
                                                int32_t numAttackers, Bitboard attackingRays,
                                                Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für die weißen Damen.
         */
        static void generateWhiteQueenCaptures(Array<Move, 256>& moves, Board& board,
                                                int32_t numAttackers, Bitboard attackingRays,
                                                Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für die schwarzen Damen.
         */
        static void generateBlackQueenCaptures(Array<Move, 256>& moves, Board& board, 
                                                int32_t numAttackers, Bitboard attackingRays,
                                                Bitboard pinnedPiecesBitboard, int32_t* pinnedDirections);

        /**
         * @brief Generiert alle legalen Züge für den weißen König.
         */
        static void generateWhiteKingCaptures(Array<Move, 256>& moves, Board& board);

        /**
         * @brief Generiert alle legalen Züge für den schwarzen König.
         */
        static void generateBlackKingCaptures(Array<Move, 256>& moves, Board& board);

};

template <Array<uint8_t, 10> moveTypes>
void Movegen::generateWhitePawnMoves(Array<Move, 256>& moves, Board& b,
                                     int32_t numAttackers, Bitboard attackingRays,
                                     Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Bauernzug kann legal sein
    if(numAttackers > 1)
        return;

    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Bauer den Angreifer schlagen oder den König schützen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[WHITE_PAWN]) {
            int32_t rank = SQ2R(sq);

            int32_t destForw = sq + NORTH;
            int32_t destForw2 = sq + 2 * NORTH;
            int32_t destLeft = sq + NORTH_WEST;
            int32_t destRight = sq + NORTH_EAST;

            int32_t file = SQ2F(sq);

            bool pinned = pinnedPiecesBitboard.getBit(sq);

            // Wenn der Bauer gefesselt ist, kann er sich nicht bewegen
            if(pinned)
                continue;

            // En-Passant Züge befreien den König nur aus dem Schach, wenn der geschlagene Bauer der Angreifer ist
            if constexpr(moveTypes.contains(MOVE_EN_PASSANT) || moveTypes.contains(MOVE_CAPTURE)) {
                if(b.enPassantSquare != NO_SQ) {
                    int32_t captureSquare = b.enPassantSquare + SOUTH;

                    if(b.enPassantSquare == destLeft && file != FILE_A) {
                        if(attackingRays.getBit(captureSquare))
                            moves.push_back(Move(sq, destLeft, MOVE_EN_PASSANT));
                    } else if(b.enPassantSquare == destRight && file != FILE_H) {
                        if(attackingRays.getBit(captureSquare))
                            moves.push_back(Move(sq, destRight, MOVE_EN_PASSANT));
                    }
                }
            }
            
            // Wenn der Bauer den Angreifer blockiert, ist der Zug legal
            // Ein blockierender Zug kann kein normaler Schlagzug sein, weil der Bauer sonst nicht gefesselt wäre
            if(attackingRays.getBit(destForw) && b.pieces[destForw] == EMPTY) {
                // Promotion
                if(rank == RANK_7) {
                    if constexpr(moveTypes.contains(MOVE_PROMOTION_QUEEN))
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_QUEEN));
                    if constexpr(moveTypes.contains(MOVE_PROMOTION_ROOK))
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_ROOK));
                    if constexpr(moveTypes.contains(MOVE_PROMOTION_BISHOP))
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_BISHOP));
                    if constexpr(moveTypes.contains(MOVE_PROMOTION_KNIGHT))
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_KNIGHT));
                } else {
                    if constexpr(moveTypes.contains(MOVE_QUIET))
                        moves.push_back(Move(sq, destForw, MOVE_QUIET));
                }
            } else if(rank == RANK_2 && attackingRays.getBit(destForw2) && b.pieces[destForw2] == EMPTY && b.pieces[destForw] == EMPTY)
                if constexpr (moveTypes.contains(MOVE_DOUBLE_PAWN))
                    moves.push_back(Move(sq, destForw2, MOVE_DOUBLE_PAWN));
            
            // Wenn der Bauer den Angreifer schlagen kann, ist der Zug legal
            Bitboard pawnAttacks = pawnAttackBitboard(sq, WHITE) & attackingRays & b.blackPiecesBitboard;

            while(pawnAttacks) {
                int32_t dest = pawnAttacks.getFirstSetBit();
                // Promotion
                if(rank == RANK_7) {
                    if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_QUEEN))
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                    if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_ROOK))
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                    if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_BISHOP))
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                    if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_KNIGHT))
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                } else {
                    if constexpr(moveTypes.contains(MOVE_CAPTURE))
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                }

                pawnAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[WHITE_PAWN]) {
            int32_t rank = SQ2R(sq);

            int32_t destForw = sq + NORTH;
            int32_t destForw2 = sq + 2 * NORTH;
            int32_t destLeft = sq + NORTH_WEST;
            int32_t destRight = sq + NORTH_EAST;

            int32_t file = SQ2F(sq);

            bool pinned = pinnedPiecesBitboard.getBit(sq);

            // En-Passant Züge entfernen 2 Figuren aus einer Reihe, daher muss immer überprüft werden, ob der König nach dem Zug im Schach stände
            // -> Ein En-Passant Zug mit einem ungefesselten Bauer kann illegal sein
            if constexpr(moveTypes.contains(MOVE_EN_PASSANT) || moveTypes.contains(MOVE_CAPTURE)) {
                if(b.enPassantSquare != NO_SQ) {
                    int32_t captureSquare = b.enPassantSquare + SOUTH;
                    int32_t kingSquare = b.pieceList[WHITE_KING].front();

                    if(b.enPassantSquare == destLeft && file != FILE_A) {
                        Bitboard allPiecesAfterMove = b.allPiecesBitboard | b.pieceBitboard[BLACK_KING];
                        allPiecesAfterMove.clearBit(sq);
                        allPiecesAfterMove.clearBit(captureSquare);
                        allPiecesAfterMove.setBit(destLeft);

                        if(!b.squareAttacked(kingSquare, BLACK, allPiecesAfterMove))
                            moves.push_back(Move(sq, destLeft, MOVE_EN_PASSANT));

                    } else if(b.enPassantSquare == destRight && file != FILE_H) {
                        Bitboard allPiecesAfterMove = b.allPiecesBitboard | b.pieceBitboard[BLACK_KING];
                        allPiecesAfterMove.clearBit(sq);
                        allPiecesAfterMove.clearBit(captureSquare);
                        allPiecesAfterMove.setBit(destRight);

                        if(!b.squareAttacked(kingSquare, BLACK, allPiecesAfterMove))
                            moves.push_back(Move(sq, destRight, MOVE_EN_PASSANT));
                    }
                }
            }

            // Überprüfe, ob der Bauer horizontal oder diagonal gefesselt ist
            // Wenn ja, kann er sich nur bewegen, wenn er den Angreifer schlägt(diagonal)
            if(pinned) {
                int32_t pinDir = pinDirections[sq];
                if(pinDir == SOUTH_EAST || pinDir == NORTH_WEST) {
                    if(b.pieces[destLeft] != EMPTY && (b.pieces[destLeft] & COLOR_MASK) == BLACK) {
                        // Promotion
                        if(rank == RANK_7) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_QUEEN))
                                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                            if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_ROOK))
                                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                            if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_BISHOP))
                                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                            if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_KNIGHT))
                                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                        } else {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE));
                        }
                    }
                } else if(pinDir == SOUTH_WEST || pinDir == NORTH_EAST) {
                    if(b.pieces[destRight] != EMPTY && (b.pieces[destRight] & COLOR_MASK) == BLACK) {
                        // Promotion
                        if(rank == RANK_7) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_QUEEN))
                                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                            if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_ROOK))
                                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                            if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_BISHOP))
                                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                            if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_KNIGHT))
                                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                        } else {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, destRight, MOVE_CAPTURE));
                        }
                    }
                }
                
                if(!(pinDir == NORTH || pinDir == SOUTH))
                    continue;
            }
            
            // Wenn der Bauer nicht horizontal oder diagonal gefesselt ist, kann er sich nach vorne bewegen
            if(b.pieces[destForw] == EMPTY) {
                // Promotion
                if(rank == RANK_7) {
                    if constexpr(moveTypes.contains(MOVE_PROMOTION_QUEEN))
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_QUEEN));
                    if constexpr(moveTypes.contains(MOVE_PROMOTION_ROOK))
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_ROOK));
                    if constexpr(moveTypes.contains(MOVE_PROMOTION_BISHOP))
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_BISHOP));
                    if constexpr(moveTypes.contains(MOVE_PROMOTION_KNIGHT))
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_KNIGHT));
                } else {
                    if constexpr(moveTypes.contains(MOVE_QUIET))
                        moves.push_back(Move(sq, destForw, MOVE_QUIET));
                }
            }

            if constexpr(moveTypes.contains(MOVE_DOUBLE_PAWN)) {
                if(rank == RANK_2 && b.pieces[destForw2] == EMPTY && b.pieces[destForw] == EMPTY)
                    moves.push_back(Move(sq, destForw2, MOVE_DOUBLE_PAWN));
            }
            
            // Schlagzüge
            // sind nur möglich, wenn der Bauer nicht gefesselt ist
            if(!pinned) {
                Bitboard pawnAttacks = pawnAttackBitboard(sq, WHITE) & b.blackPiecesBitboard;

                while(pawnAttacks) {
                    int32_t dest = pawnAttacks.getFirstSetBit();
                    // Promotion
                    if(rank == RANK_7) {
                        if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_QUEEN))
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                        if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_ROOK))
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                        if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_BISHOP))
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                        if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_KNIGHT))
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                    } else {
                        if constexpr(moveTypes.contains(MOVE_CAPTURE))
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                    }

                    pawnAttacks.clearBit(dest);
                }
            }
        }
    }
}

template <Array<uint8_t, 10> moveTypes>
void Movegen::generateBlackPawnMoves(Array<Move, 256>& moves, Board& b,
                                     int32_t numAttackers, Bitboard attackingRays,
                                     Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Bauerzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Bauer den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[BLACK_PAWN]) {
            int32_t rank = SQ2R(sq);

            int32_t destForw = sq + SOUTH;
            int32_t destForw2 = sq + 2 * SOUTH;
            int32_t destLeft = sq + SOUTH_WEST;
            int32_t destRight = sq + SOUTH_EAST;

            int32_t file = SQ2F(sq);

            bool pinned = pinnedPiecesBitboard.getBit(sq);

            // Wenn der Bauer gefesselt ist, kann er sich nicht bewegen
            if(pinned)
                continue;
            
            // En-Passant Züge befreien den König nur aus dem Schach, wenn der geschlagene Bauer der Angreifer ist
            if constexpr(moveTypes.contains(MOVE_EN_PASSANT) || moveTypes.contains(MOVE_CAPTURE)) {
                if(b.enPassantSquare != NO_SQ) {
                    int32_t captureSquare = b.enPassantSquare + NORTH;

                    if(b.enPassantSquare == destLeft && file != FILE_A) {
                        if(attackingRays.getBit(captureSquare))
                            moves.push_back(Move(sq, destLeft, MOVE_EN_PASSANT));
                    } else if(b.enPassantSquare == destRight && file != FILE_H) {
                        if(attackingRays.getBit(captureSquare))
                            moves.push_back(Move(sq, destRight, MOVE_EN_PASSANT));
                    }
                }
            }
            
            // Wenn der Bauer den Angreifer blockiert, ist der Zug legal
            // Ein blockierender Zug kann kein normaler Schlagzug sein, weil der Bauer sonst nicht gefesselt wäre
            if(attackingRays.getBit(destForw) && b.pieces[destForw] == EMPTY) {
                // Promotion
                if(rank == RANK_2) {
                    if constexpr(moveTypes.contains(MOVE_PROMOTION_QUEEN))
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_QUEEN));
                    if constexpr(moveTypes.contains(MOVE_PROMOTION_ROOK))
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_ROOK));
                    if constexpr(moveTypes.contains(MOVE_PROMOTION_BISHOP))
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_BISHOP));
                    if constexpr(moveTypes.contains(MOVE_PROMOTION_KNIGHT))
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_KNIGHT));
                } else {
                    if constexpr(moveTypes.contains(MOVE_QUIET))
                        moves.push_back(Move(sq, destForw, MOVE_QUIET));
                }
            } else if(rank == RANK_7 && attackingRays.getBit(destForw2) && b.pieces[destForw2] == EMPTY && b.pieces[destForw] == EMPTY)
                if constexpr(moveTypes.contains(MOVE_DOUBLE_PAWN))
                    moves.push_back(Move(sq, destForw2, MOVE_DOUBLE_PAWN));
            
            // Wenn der Bauer den Angreifer schlagen kann, ist der Zug legal
            Bitboard pawnAttacks = pawnAttackBitboard(sq, BLACK) & attackingRays & b.whitePiecesBitboard;

            while(pawnAttacks) {
                int32_t dest = pawnAttacks.getFirstSetBit();
                // Promotion
                if(rank == RANK_2) {
                    if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_QUEEN))
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                    if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_ROOK))
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                    if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_BISHOP))
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                    if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_KNIGHT))
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                } else {
                    if constexpr(moveTypes.contains(MOVE_CAPTURE))
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                }

                pawnAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[BLACK_PAWN]) {
            int32_t rank = SQ2R(sq);

            int32_t destForw = sq + SOUTH;
            int32_t destForw2 = sq + 2 * SOUTH;
            int32_t destLeft = sq + SOUTH_WEST;
            int32_t destRight = sq + SOUTH_EAST;

            int32_t file = SQ2F(sq);

            bool pinned = pinnedPiecesBitboard.getBit(sq);

            // En-Passant Züge entfernen 2 Figuren aus einer Reihe, daher muss immer überprüft werden, ob der König nach dem Zug im Schach stände
            // -> Ein En-Passant Zug mit einem ungefesselten Bauer kann illegal sein
            if constexpr(moveTypes.contains(MOVE_EN_PASSANT) || moveTypes.contains(MOVE_CAPTURE)) {
                if(b.enPassantSquare != NO_SQ) {
                    int32_t captureSquare = b.enPassantSquare + NORTH;
                    int32_t kingSquare = b.pieceList[BLACK_KING].front();

                    if(b.enPassantSquare == destLeft && file != FILE_A) {
                        Bitboard allPiecesAfterMove = b.allPiecesBitboard | b.pieceBitboard[WHITE_KING];
                        allPiecesAfterMove.clearBit(sq);
                        allPiecesAfterMove.clearBit(captureSquare);
                        allPiecesAfterMove.setBit(destLeft);

                        if(!b.squareAttacked(kingSquare, WHITE, allPiecesAfterMove))
                            moves.push_back(Move(sq, destLeft, MOVE_EN_PASSANT));

                    } else if(b.enPassantSquare == destRight && file != FILE_H) {
                        Bitboard allPiecesAfterMove = b.allPiecesBitboard | b.pieceBitboard[WHITE_KING];
                        allPiecesAfterMove.clearBit(sq);
                        allPiecesAfterMove.clearBit(captureSquare);
                        allPiecesAfterMove.setBit(destRight);

                        if(!b.squareAttacked(kingSquare, WHITE, allPiecesAfterMove))
                            moves.push_back(Move(sq, destRight, MOVE_EN_PASSANT));
                    }
                }
            }

            // Überprüfe, ob der Bauer horizontal oder diagonal gefesselt ist
            // Wenn ja, kann er sich nur bewegen, wenn er den Angreifer schlägt(diagonal)
            if(pinned) {
                int32_t pinDir = pinDirections[sq];
                
                if(pinDir == NORTH_EAST || pinDir == SOUTH_WEST) {
                    if(b.pieces[destLeft] != EMPTY && (b.pieces[destLeft] & COLOR_MASK) == WHITE) {
                        // Promotion
                        if(rank == RANK_2) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_QUEEN))
                                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                            if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_ROOK))
                                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                            if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_BISHOP))
                                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                            if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_KNIGHT))
                                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                        } else {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, destLeft, MOVE_CAPTURE));
                        }
                    }
                } else if(pinDir == NORTH_WEST || pinDir == SOUTH_EAST) {
                    if(b.pieces[destRight] != EMPTY && (b.pieces[destRight] & COLOR_MASK) == WHITE) {
                        // Promotion
                        if(rank == RANK_2) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_QUEEN))
                                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                            if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_ROOK))
                                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                            if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_BISHOP))
                                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                            if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_KNIGHT))
                                moves.push_back(Move(sq, destRight, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                        } else {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, destRight, MOVE_CAPTURE));
                        }
                    }
                }
                
                if(!(pinDir == NORTH || pinDir == SOUTH))
                    continue;
            }

            // Wenn der Bauer nicht horizontal oder diagonal gefesselt ist, kann er sich nach vorne bewegen
            if(b.pieces[destForw] == EMPTY) {
                // Promotion
                if(rank == RANK_2) {
                    if constexpr(moveTypes.contains(MOVE_PROMOTION_QUEEN))
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_QUEEN));
                    if constexpr(moveTypes.contains(MOVE_PROMOTION_ROOK))
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_ROOK));
                    if constexpr(moveTypes.contains(MOVE_PROMOTION_BISHOP))
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_BISHOP));
                    if constexpr(moveTypes.contains(MOVE_PROMOTION_KNIGHT))
                        moves.push_back(Move(sq, destForw, MOVE_PROMOTION_KNIGHT));
                } else {
                    if constexpr(moveTypes.contains(MOVE_QUIET))
                        moves.push_back(Move(sq, destForw, MOVE_QUIET));
                }
            }

            if(rank == RANK_7 && b.pieces[destForw2] == EMPTY && b.pieces[destForw] == EMPTY)
                if constexpr(moveTypes.contains(MOVE_DOUBLE_PAWN))
                    moves.push_back(Move(sq, destForw2, MOVE_DOUBLE_PAWN));
            
            // Schlagzüge
            // sind nur möglich, wenn der Bauer nicht gefesselt ist
            if(!pinned) {
                Bitboard pawnAttacks = pawnAttackBitboard(sq, BLACK) & b.whitePiecesBitboard;

                while(pawnAttacks) {
                    int32_t dest = pawnAttacks.getFirstSetBit();
                    // Promotion
                    if(rank == RANK_2) {
                        if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_QUEEN))
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_QUEEN));
                        if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_ROOK))
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_ROOK));
                        if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_BISHOP))
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_BISHOP));
                        if constexpr(moveTypes.contains(MOVE_CAPTURE) || moveTypes.contains(MOVE_PROMOTION_KNIGHT))
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE | MOVE_PROMOTION_KNIGHT));
                    } else {
                        if constexpr(moveTypes.contains(MOVE_CAPTURE))
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                    }

                    pawnAttacks.clearBit(dest);
                }
            }
        }
    }
}

template <Array<uint8_t, 10> moveTypes>
void Movegen::generateWhiteKnightMoves(Array<Move, 256>& moves, Board& b,
                                       int32_t numAttackers, Bitboard attackingRays,
                                       Bitboard pinnedPieces) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Springerzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Springer den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[WHITE_KNIGHT]) {
            // Wenn der Springer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPieces.getBit(sq))
                continue;

            Bitboard knightAttacks = knightAttackBitboard(sq) & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

            knightAttacks &= attackingRays;
            while(knightAttacks) {
                int32_t dest = knightAttacks.getFirstSetBit();
                if(b.pieces[dest] == EMPTY) {
                    if constexpr(moveTypes.contains(MOVE_QUIET))
                        moves.push_back(Move(sq, dest, MOVE_QUIET));
                } else {
                    if constexpr(moveTypes.contains(MOVE_CAPTURE))
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                }

                knightAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[WHITE_KNIGHT]) {
            // Wenn der Springer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPieces.getBit(sq))
                continue;

            // Ansonsten darf er sich frei bewegen
            Bitboard knightAttacks = knightAttackBitboard(sq) & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];
            while(knightAttacks) {
                int32_t dest = knightAttacks.getFirstSetBit();
                if(b.pieces[dest] == EMPTY) {
                    if constexpr(moveTypes.contains(MOVE_QUIET))
                        moves.push_back(Move(sq, dest, MOVE_QUIET));
                } else {
                    if constexpr(moveTypes.contains(MOVE_CAPTURE))
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                }

                knightAttacks.clearBit(dest);
            }
        }
    }
}

template <Array<uint8_t, 10> moveTypes>
void Movegen::generateBlackKnightMoves(Array<Move, 256>& moves, Board& b,
                                       int32_t numAttackers, Bitboard attackingRays,
                                       Bitboard pinnedPieces) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Springerzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Springer den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[BLACK_KNIGHT]) {
            // Wenn der Springer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPieces.getBit(sq))
                continue;

            Bitboard knightAttacks = knightAttackBitboard(sq) & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

            knightAttacks &= attackingRays;
            while(knightAttacks) {
                int32_t dest = knightAttacks.getFirstSetBit();
                if(b.pieces[dest] == EMPTY) {
                    if constexpr(moveTypes.contains(MOVE_QUIET))
                        moves.push_back(Move(sq, dest, MOVE_QUIET));
                } else {
                    if constexpr(moveTypes.contains(MOVE_CAPTURE))
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                }

                knightAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[BLACK_KNIGHT]) {
            // Wenn der Springer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPieces.getBit(sq))
                continue;

            // Ansonsten darf er sich frei bewegen
            Bitboard knightAttacks = knightAttackBitboard(sq) & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];
            while(knightAttacks) {
                int32_t dest = knightAttacks.getFirstSetBit();
                if(b.pieces[dest] == EMPTY) {
                    if constexpr(moveTypes.contains(MOVE_QUIET))
                        moves.push_back(Move(sq, dest, MOVE_QUIET));
                } else {
                    if constexpr(moveTypes.contains(MOVE_CAPTURE))
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                }

                knightAttacks.clearBit(dest);
            }
        }
    }
}

template <Array<uint8_t, 10> moveTypes>
void Movegen::generateWhiteBishopMoves(Array<Move, 256>& moves, Board& b,
                                       int32_t numAttackers, Bitboard attackingRays,
                                       Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Läuferzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Läufer den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[WHITE_BISHOP]) {
            // Wenn der Läufer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPiecesBitboard.getBit(sq))
                continue;

            Bitboard bishopAttacks = diagonalAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                    & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

            bishopAttacks &= attackingRays;

            while(bishopAttacks) {
                int32_t dest = bishopAttacks.getFirstSetBit();
                if(b.pieces[dest] == EMPTY) {
                    if constexpr(moveTypes.contains(MOVE_QUIET))
                        moves.push_back(Move(sq, dest, MOVE_QUIET));
                } else {
                    if constexpr(moveTypes.contains(MOVE_CAPTURE))
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                }

                bishopAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[WHITE_BISHOP]) {
            // Wenn der Läufer gefesselt ist, muss er sich in oder gegen die Richtung bewegen, in die er gefesselt ist
            if(pinnedPiecesBitboard.getBit(sq)) {
                int32_t pinDirection = pinDirections[sq];

                // Diagonale von links unten nach rechts oben
                if(pinDirection == SOUTH_WEST || pinDirection == NORTH_EAST) {
                    int n_sq = sq + SOUTH_WEST;

                    // Eine Abbruchbedingung ist hier nicht nötig, da in beide Fesselrichtungen eine Figur stehen muss
                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += SOUTH_WEST;
                    }

                    n_sq = sq + NORTH_EAST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += NORTH_EAST;
                    }
                } else if(pinDirection == SOUTH_EAST || pinDirection == NORTH_WEST) {
                    // Diagonale von rechts unten nach links oben
                    int n_sq = sq + SOUTH_EAST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += SOUTH_EAST;
                    }

                    n_sq = sq + NORTH_WEST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += NORTH_WEST;
                    }
                }
            } else {
                // Ansonsten darf er sich frei bewegen
                Bitboard bishopAttacks = diagonalAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                        & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

                while(bishopAttacks) {
                    int32_t dest = bishopAttacks.getFirstSetBit();
                    if(b.pieces[dest] == EMPTY) {
                        if constexpr(moveTypes.contains(MOVE_QUIET))
                            moves.push_back(Move(sq, dest, MOVE_QUIET));
                    } else {
                        if constexpr(moveTypes.contains(MOVE_CAPTURE))
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                    }

                    bishopAttacks.clearBit(dest);
                }
            }
        }
    }
}

template <Array<uint8_t, 10> moveTypes>
void Movegen::generateBlackBishopMoves(Array<Move, 256>& moves, Board& b,
                                       int32_t numAttackers, Bitboard attackingRays,
                                       Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Läuferzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Läufer den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[BLACK_BISHOP]) {
            // Wenn der Läufer gefesselt ist, kann er sich nicht bewegen
            if(pinnedPiecesBitboard.getBit(sq))
                continue;

            Bitboard bishopAttacks = diagonalAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                    & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

            bishopAttacks &= attackingRays;

            while(bishopAttacks) {
                int32_t dest = bishopAttacks.getFirstSetBit();
                if(b.pieces[dest] == EMPTY) {
                    if constexpr(moveTypes.contains(MOVE_QUIET))
                        moves.push_back(Move(sq, dest, MOVE_QUIET));
                } else {
                    if constexpr(moveTypes.contains(MOVE_CAPTURE))
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                }

                bishopAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[BLACK_BISHOP]) {
            // Wenn der Läufer gefesselt ist, muss er sich in oder gegen die Richtung bewegen, in die er gefesselt ist
            if(pinnedPiecesBitboard.getBit(sq)) {
                int32_t pinDirection = pinDirections[sq];

                // Diagonale von links unten nach rechts oben
                if(pinDirection == NORTH_EAST || pinDirection == SOUTH_WEST) {
                    int n_sq = sq + NORTH_EAST;

                    // Eine Abbruchbedingung ist hier nicht nötig, da in beide Fesselrichtungen eine Figur stehen muss
                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += NORTH_EAST;
                    }

                    n_sq = sq + SOUTH_WEST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += SOUTH_WEST;
                    }
                } else if(pinDirection == NORTH_WEST || pinDirection == SOUTH_EAST) {
                    // Diagonale von rechts unten nach links oben
                    int n_sq = sq + NORTH_WEST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += NORTH_WEST;
                    }

                    n_sq = sq + SOUTH_EAST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += SOUTH_EAST;
                    }
                }
            } else {
                // Ansonsten darf er sich frei bewegen
                Bitboard bishopAttacks = diagonalAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                        & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

                while(bishopAttacks) {
                    int32_t dest = bishopAttacks.getFirstSetBit();
                    if(b.pieces[dest] == EMPTY) {
                        if constexpr(moveTypes.contains(MOVE_QUIET))
                            moves.push_back(Move(sq, dest, MOVE_QUIET));
                    } else {
                        if constexpr(moveTypes.contains(MOVE_CAPTURE))
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                    }

                    bishopAttacks.clearBit(dest);
                }
            }
        }
    }
}

template <Array<uint8_t, 10> moveTypes>
void Movegen::generateWhiteRookMoves(Array<Move, 256>& moves, Board& b,
                                     int32_t numAttackers, Bitboard attackingRays,
                                     Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Turmzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Turm den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[WHITE_ROOK]) {
            // Wenn der Turm gefesselt ist, kann er sich nicht bewegen
            if(pinnedPiecesBitboard.getBit(sq))
                continue;

            Bitboard rookAttacks = straightAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                    & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

            rookAttacks &= attackingRays;

            while(rookAttacks) {
                int32_t dest = rookAttacks.getFirstSetBit();
                if(b.pieces[dest] == EMPTY) {
                    if constexpr(moveTypes.contains(MOVE_QUIET))
                        moves.push_back(Move(sq, dest, MOVE_QUIET));
                } else {
                    if constexpr(moveTypes.contains(MOVE_CAPTURE))
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                }

                rookAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[WHITE_ROOK]) {
            // Wenn der Turm gefesselt ist, muss er sich in oder gegen die Richtung bewegen, in die er gefesselt ist
            if(pinnedPiecesBitboard.getBit(sq)) {
                int32_t pinDirection = pinDirections[sq];

                // Gerade von oben nach unten
                if(pinDirection == NORTH || pinDirection == SOUTH) {
                    int n_sq = sq + NORTH;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += NORTH;
                    }

                    n_sq = sq + SOUTH;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += SOUTH;
                    }

                } else if(pinDirection == EAST || pinDirection == WEST) {
                    // Gerade von links nach rechts
                    int n_sq = sq + EAST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += EAST;
                    }

                    n_sq = sq + WEST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += WEST;
                    }
                }
            } else {
                // Ansonsten darf er sich frei bewegen
                Bitboard rookAttacks = straightAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                        & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

                while(rookAttacks) {
                    int32_t dest = rookAttacks.getFirstSetBit();
                    if(b.pieces[dest] == EMPTY) {
                        if constexpr(moveTypes.contains(MOVE_QUIET))
                            moves.push_back(Move(sq, dest, MOVE_QUIET));
                    } else {
                        if constexpr(moveTypes.contains(MOVE_CAPTURE))
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                    }

                    rookAttacks.clearBit(dest);
                }
            }
        }
    }
}

template <Array<uint8_t, 10> moveTypes>
void Movegen::generateBlackRookMoves(Array<Move, 256>& moves, Board& b,
                                     int32_t numAttackers, Bitboard attackingRays,
                                     Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Turmzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann der Turm den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[BLACK_ROOK]) {
            // Wenn der Turm gefesselt ist, kann er sich nicht bewegen
            if(pinnedPiecesBitboard.getBit(sq))
                continue;

            Bitboard rookAttacks = straightAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                    & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

            rookAttacks &= attackingRays;

            while(rookAttacks) {
                int32_t dest = rookAttacks.getFirstSetBit();
                if(b.pieces[dest] == EMPTY) {
                    if constexpr(moveTypes.contains(MOVE_QUIET))
                        moves.push_back(Move(sq, dest, MOVE_QUIET));
                } else {
                    if constexpr(moveTypes.contains(MOVE_CAPTURE))
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                }

                rookAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[BLACK_ROOK]) {
            // Wenn der Turm gefesselt ist, muss er sich in oder gegen die Richtung bewegen, in die er gefesselt ist
            if(pinnedPiecesBitboard.getBit(sq)) {
                int32_t pinDirection = pinDirections[sq];

                // Gerade von oben nach unten
                if(pinDirection == NORTH || pinDirection == SOUTH) {
                    int n_sq = sq + NORTH;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += NORTH;
                    }

                    n_sq = sq + SOUTH;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += SOUTH;
                    }

                } else if(pinDirection == EAST || pinDirection == WEST) {
                    // Gerade von links nach rechts
                    int n_sq = sq + EAST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += EAST;
                    }

                    n_sq = sq + WEST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += WEST;
                    }
                }
            } else {
                // Ansonsten darf er sich frei bewegen
                Bitboard rookAttacks = straightAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                        & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

                while(rookAttacks) {
                    int32_t dest = rookAttacks.getFirstSetBit();
                    if(b.pieces[dest] == EMPTY) {
                        if constexpr(moveTypes.contains(MOVE_QUIET))
                            moves.push_back(Move(sq, dest, MOVE_QUIET));
                    } else {
                        if constexpr(moveTypes.contains(MOVE_CAPTURE))
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                    }

                    rookAttacks.clearBit(dest);
                }
            }
        }
    }
}

template <Array<uint8_t, 10> moveTypes>
void Movegen::generateWhiteQueenMoves(Array<Move, 256>& moves, Board& b,
                                      int32_t numAttackers, Bitboard attackingRays,
                                      Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Damenzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann die Dame den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[WHITE_QUEEN]) {
            // Wenn die Dame gefesselt ist, kann sie sich nicht bewegen
            if(pinnedPiecesBitboard.getBit(sq))
                continue;

            // Diagonal
            Bitboard queenAttacks = diagonalAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                    & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

            // Gerade
            queenAttacks |= straightAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                    & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

            queenAttacks &= attackingRays;

            while(queenAttacks) {
                int32_t dest = queenAttacks.getFirstSetBit();
                if(b.pieces[dest] == EMPTY) {
                    if constexpr(moveTypes.contains(MOVE_QUIET))
                        moves.push_back(Move(sq, dest, MOVE_QUIET));
                } else {
                    if constexpr(moveTypes.contains(MOVE_CAPTURE))
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                }

                queenAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[WHITE_QUEEN]) {
            // Wenn die Dame gefesselt ist, muss sie sich in oder gegen die Richtung bewegen, in die sie gefesselt ist
            if(pinnedPiecesBitboard.getBit(sq)) {
                int32_t pinDirection = pinDirections[sq];

                // Diagonal von links oben nach rechts unten
                if(pinDirection == NORTH_WEST || pinDirection == SOUTH_EAST) {
                    int n_sq = sq + NORTH_WEST;

                    // Eine Abbruchbedingung ist hier nicht nötig, da in beide Fesselrichtungen eine Figur stehen muss
                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += NORTH_WEST;
                    }

                    n_sq = sq + SOUTH_EAST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += SOUTH_EAST;
                    }
                } else if(pinDirection == NORTH_EAST || pinDirection == SOUTH_WEST) {
                    int n_sq = sq + NORTH_EAST;

                    while(true) {
                        // Diagonal von rechts oben nach links unten
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;

                        n_sq += NORTH_EAST;
                    }

                    n_sq = sq + SOUTH_WEST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;

                        n_sq += SOUTH_WEST;
                    }
                } else if(pinDirection == NORTH || pinDirection == SOUTH) {
                    // Gerade von oben nach unten
                    int n_sq = sq + NORTH;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else
                            break;

                        n_sq += NORTH;
                    }

                    n_sq = sq + SOUTH;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else
                            break;

                        n_sq += SOUTH;
                    }
                } else if(pinDirection == EAST || pinDirection == WEST) {
                    // Gerade von links nach rechts
                    int n_sq = sq + EAST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else
                            break;

                        n_sq += EAST;
                    }

                    n_sq = sq + WEST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == BLACK) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else
                            break;

                        n_sq += WEST;
                    }
                }
            } else {
                // Ansonsten darf sie sich frei bewegen

                // Diagonal
                Bitboard queenAttacks = diagonalAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                        & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

                // Gerade
                queenAttacks |= straightAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[WHITE_KING])
                                        & ~b.whitePiecesBitboard & ~b.pieceBitboard[WHITE_KING];

                while(queenAttacks) {
                    int32_t dest = queenAttacks.getFirstSetBit();
                    if(b.pieces[dest] == EMPTY) {
                        if constexpr(moveTypes.contains(MOVE_QUIET))
                            moves.push_back(Move(sq, dest, MOVE_QUIET));
                    } else {
                        if constexpr(moveTypes.contains(MOVE_CAPTURE))
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                    }

                    queenAttacks.clearBit(dest);
                }
            }
        }
    }
}

template <Array<uint8_t, 10> moveTypes>
void Movegen::generateBlackQueenMoves(Array<Move, 256>& moves, Board& b,
                                      int32_t numAttackers, Bitboard attackingRays,
                                      Bitboard pinnedPiecesBitboard, int32_t* pinDirections) {
    // Wenn der eigene König von mehr als einer Figur angegriffen wird, muss er sich bewegen
    // -> Kein Damenzug kann legal sein
    if(numAttackers > 1)
        return;
    
    // Wenn der eigene König von genau einer Figur angegriffen wird, kann die Dame den Angreifer schlagen oder sich dazwischen stellen
    if(numAttackers == 1) {
        for(int sq : b.pieceList[BLACK_QUEEN]) {
            // Wenn die Dame gefesselt ist, kann sie sich nicht bewegen
            if(pinnedPiecesBitboard.getBit(sq))
                continue;

            // Diagonal
            Bitboard queenAttacks = diagonalAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                    & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

            // Gerade
            queenAttacks |= straightAttackBitboard(sq,
                                    b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                    & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

            queenAttacks &= attackingRays;

            while(queenAttacks) {
                int32_t dest = queenAttacks.getFirstSetBit();
                if(b.pieces[dest] == EMPTY) {
                    if constexpr(moveTypes.contains(MOVE_QUIET))
                        moves.push_back(Move(sq, dest, MOVE_QUIET));
                } else {
                    if constexpr(moveTypes.contains(MOVE_CAPTURE))
                        moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                }

                queenAttacks.clearBit(dest);
            }
        }
    } else {
        for(int sq : b.pieceList[BLACK_QUEEN]) {
            // Wenn die Dame gefesselt ist, muss sie sich in oder gegen die Richtung bewegen, in die sie gefesselt ist
            if(pinnedPiecesBitboard.getBit(sq)) {
                int32_t pinDirection = pinDirections[sq];

                // Diagonal von links oben nach rechts unten
                if(pinDirection == NORTH_WEST || pinDirection == SOUTH_EAST) {
                    int n_sq = sq + NORTH_WEST;

                    // Eine Abbruchbedingung ist hier nicht nötig, da in beide Fesselrichtungen eine Figur stehen muss
                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += NORTH_WEST;
                    }

                    n_sq = sq + SOUTH_EAST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;
                        n_sq += SOUTH_EAST;
                    }
                } else if(pinDirection == NORTH_EAST || pinDirection == SOUTH_WEST) {
                    int n_sq = sq + NORTH_EAST;

                    while(true) {
                        // Diagonal von rechts oben nach links unten
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;

                        n_sq += NORTH_EAST;
                    }

                    n_sq = sq + SOUTH_WEST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        }
                        else
                            break;

                        n_sq += SOUTH_WEST;
                    }
                } else if(pinDirection == NORTH || pinDirection == SOUTH) {
                    // Gerade von oben nach unten
                    int n_sq = sq + NORTH;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else
                            break;

                        n_sq += NORTH;
                    }

                    n_sq = sq + SOUTH;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else
                            break;

                        n_sq += SOUTH;
                    }
                } else if(pinDirection == EAST || pinDirection == WEST) {
                    // Gerade von links nach rechts
                    int n_sq = sq + EAST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else
                            break;

                        n_sq += EAST;
                    }

                    n_sq = sq + WEST;

                    while(true) {
                        if(b.pieces[n_sq] == EMPTY) {
                            if constexpr(moveTypes.contains(MOVE_QUIET))
                                moves.push_back(Move(sq, n_sq, MOVE_QUIET));
                        } else if((b.pieces[n_sq] & COLOR_MASK) == WHITE) {
                            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                                moves.push_back(Move(sq, n_sq, MOVE_CAPTURE));
                            break;
                        } else
                            break;

                        n_sq += WEST;
                    }
                }
            } else {
                // Ansonsten darf sie sich frei bewegen

                // Diagonal
                Bitboard queenAttacks = diagonalAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                        & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

                // Gerade
                queenAttacks |= straightAttackBitboard(sq,
                                        b.allPiecesBitboard | b.pieceBitboard[BLACK_KING])
                                        & ~b.blackPiecesBitboard & ~b.pieceBitboard[BLACK_KING];

                while(queenAttacks) {
                    int32_t dest = queenAttacks.getFirstSetBit();
                    if(b.pieces[dest] == EMPTY) {
                        if constexpr(moveTypes.contains(MOVE_QUIET))
                            moves.push_back(Move(sq, dest, MOVE_QUIET));
                    } else {
                        if constexpr(moveTypes.contains(MOVE_CAPTURE))
                            moves.push_back(Move(sq, dest, MOVE_CAPTURE));
                    }

                    queenAttacks.clearBit(dest);
                }
            }
        }
    }
}

template <Array<uint8_t, 10> moveTypes>
void Movegen::generateWhiteKingMoves(Array<Move, 256>& moves, Board& b,
                                     Bitboard attackedSquares) {

    int sq = b.pieceList[WHITE_KING].front();
    
    Bitboard kingAttacks = kingAttackBitboard(sq) & ~b.whitePiecesBitboard & ~b.blackAttackBitboard;

    while(kingAttacks) {
        int32_t dest = kingAttacks.getFirstSetBit();
        if(b.pieces[dest] == EMPTY) {
            if constexpr(moveTypes.contains(MOVE_QUIET))
                moves.push_back(Move(sq, dest, MOVE_QUIET));
        } else {
            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                moves.push_back(Move(sq, dest, MOVE_CAPTURE));
        }

        kingAttacks.clearBit(dest);
    }

    if(!attackedSquares.getBit(E1)) {
        if(b.castlingPermission & WHITE_KINGSIDE_CASTLE) {
            if(b.pieces[F1] == EMPTY && b.pieces[G1] == EMPTY &&
                !attackedSquares.getBit(F1) && !attackedSquares.getBit(G1)) {
                if constexpr(moveTypes.contains(MOVE_KINGSIDE_CASTLE))
                    moves.push_back(Move(E1, G1, MOVE_KINGSIDE_CASTLE));
            }
        }

        if(b.castlingPermission & WHITE_QUEENSIDE_CASTLE) {
            if(b.pieces[D1] == EMPTY && b.pieces[C1] == EMPTY && b.pieces[B1] == EMPTY &&
                !attackedSquares.getBit(D1) && !attackedSquares.getBit(C1)) {
                if constexpr(moveTypes.contains(MOVE_QUEENSIDE_CASTLE))
                    moves.push_back(Move(E1, C1, MOVE_QUEENSIDE_CASTLE));
            }
        }
    }
}

template <Array<uint8_t, 10> moveTypes>
void Movegen::generateBlackKingMoves(Array<Move, 256>& moves, Board& b,
                                     Bitboard attackedSquares) {
    int sq = b.pieceList[BLACK_KING].front();
    
    Bitboard kingAttacks = kingAttackBitboard(sq) & ~b.blackPiecesBitboard & ~b.whiteAttackBitboard;

    while(kingAttacks) {
        int32_t dest = kingAttacks.getFirstSetBit();
        if(b.pieces[dest] == EMPTY) {
            if constexpr(moveTypes.contains(MOVE_QUIET))
                moves.push_back(Move(sq, dest, MOVE_QUIET));
        } else {
            if constexpr(moveTypes.contains(MOVE_CAPTURE))
                moves.push_back(Move(sq, dest, MOVE_CAPTURE));
        }

        kingAttacks.clearBit(dest);
    }

    if(!attackedSquares.getBit(E8)) {
        if(b.castlingPermission & BLACK_KINGSIDE_CASTLE) {
            if(b.pieces[F8] == EMPTY && b.pieces[G8] == EMPTY &&
                !attackedSquares.getBit(F8) && !attackedSquares.getBit(G8)) {
                if constexpr(moveTypes.contains(MOVE_KINGSIDE_CASTLE))
                    moves.push_back(Move(E8, G8, MOVE_KINGSIDE_CASTLE));
            }
        }

        if(b.castlingPermission & BLACK_QUEENSIDE_CASTLE) {
            if(b.pieces[D8] == EMPTY && b.pieces[C8] == EMPTY && b.pieces[B8] == EMPTY &&
                !attackedSquares.getBit(D8) && !attackedSquares.getBit(C8)) {
                if constexpr(moveTypes.contains(MOVE_QUEENSIDE_CASTLE))
                    moves.push_back(Move(E8, C8, MOVE_QUEENSIDE_CASTLE));
            }
        }
    }
}

#endif