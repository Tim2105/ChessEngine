#ifndef BOARD_H
#define BOARD_H

#include "core/chess/BoardDefinitions.h"
#include "core/chess/Move.h"
#include "core/utils/Array.h"
#include "core/utils/Bitboard.h"

#include <stdalign.h>
#include <stdint.h>
#include <string>
#include <vector>

/**
 * @brief Enthält alle notwendigen Informationen um einen Zug rückgängig zu machen.
 */
class MoveHistoryEntry {
    public:
        /**
         * @brief Der Zug der rückgängig gemacht werden soll.
         */
        Move move;

        /**
         * @brief Speichert den Typ der geschlagenen Figur.
         */
        int32_t capturedPiece;

        /**
         * @brief Speichert alle möglichen Rochaden vor diesem Zug.
         */
        int32_t castlingPermission;

        /**
         * @brief Speichert die Position eines möglichen En Passant Zuges vor diesem Zug(wenn möglich).
         */
        int32_t enPassantSquare;

        /**
         * @brief Der 50-Zug Counter vor diesem Zug.
         */
        int32_t fiftyMoveRule;

        /**
         * @brief Speichert den Hashwert vor diesem Zug.
         */
        uint64_t hashValue;

        /**
         * @brief Speichert das weiße Figurenbitboard vor diesem Zug.
        */
        Bitboard whitePiecesBitboard;

        /**
         * @brief Speichert das schwarze Figurenbitboard vor diesem Zug.
        */
        Bitboard blackPiecesBitboard;

        /**
         * @brief Speichert alle individuellen Figurenbitboards vor diesem Zug.
        */
        Bitboard pieceBitboard[15];

        /**
         * @brief Speichert das weiße Angriffsbitboard vor diesem Zug.
         */
        Bitboard whiteAttackBitboard;

        /**
         * @brief Speichert das schwarze Angriffsbitboard vor diesem Zug.
         */
        Bitboard blackAttackBitboard;

        /**
         * @brief Speichert die Angriffsbitboards der Figuren vor diesem Zug.
         */
        Bitboard pieceAttackBitboard[15];

        /**
         * @brief Erstellt einen neuen MoveHistoryEntry.
         * @param move Der Zug der rückgängig gemacht werden soll.
         * @param capturedPiece Speichert den Typ der geschlagenen Figur.
         * @param castlePermission Speichert alle möglichen Rochaden vor diesem Zug.
         * @param enPassantSquare Speichert die Position eines möglichen En Passant Zuges vor diesem Zug(wenn möglich).
         * @param fiftyMoveRule Der 50-Zug Counter vor diesem Zug.
         * @param hashValue Speichert den Hashwert vor diesem Zug.
         * @param whitePiecesBitboard Speichert das weiße Figurenbitboard vor diesem Zug.
         * @param blackPiecesBitboard Speichert das schwarze Figurenbitboard vor diesem Zug.
         * @param pieceBitboards Speichert alle individuellen Figurenbitboards vor diesem Zug.
         * @param whiteAttackBitboard Speichert das weiße Angriffsbitboard vor diesem Zug.
         * @param blackAttackBitboard Speichert das schwarze Angriffsbitboard vor diesem Zug.
         * @param pieceAttackBitboards Speichert die Angriffsbitboards der Figuren vor diesem Zug.
         */
        MoveHistoryEntry(Move move, int32_t capturedPiece, int32_t castlePermission,
                        int32_t enPassantSquare, int32_t fiftyMoveRule, uint64_t hashValue,
                        Bitboard whitePiecesBitboard, Bitboard blackPiecesBitboard, Bitboard pieceBitboards[15],
                        Bitboard whiteAttackBitboard, Bitboard blackAttackBitboard, Bitboard pieceAttackBitboards[15]) {
            this->move = move;
            this->capturedPiece = capturedPiece;
            this->castlingPermission = castlePermission;
            this->enPassantSquare = enPassantSquare;
            this->fiftyMoveRule = fiftyMoveRule;
            this->hashValue = hashValue;
            this->whitePiecesBitboard = whitePiecesBitboard;
            this->blackPiecesBitboard = blackPiecesBitboard;

            for (int i = 0; i < 15; i++) {
                this->pieceBitboard[i] = pieceBitboards[i];
            }

            this->whiteAttackBitboard = whiteAttackBitboard;
            this->blackAttackBitboard = blackAttackBitboard;
            
            for (int i = 0; i < 15; i++) {
                this->pieceAttackBitboard[i] = pieceAttackBitboards[i];
            }
        }

        /**
         * @brief Erstellt einen neuen MoveHistoryEntry.
         * @param move Der Zug der rückgängig gemacht werden soll.
         */
        MoveHistoryEntry(Move move) {
            this->move = move;
            this->capturedPiece = EMPTY;
            this->castlingPermission = 0;
            this->enPassantSquare = 0;
            this->fiftyMoveRule = 0;
            this->hashValue = 0;
            this->whitePiecesBitboard = 0;
            this->blackPiecesBitboard = 0;

            for (int i = 0; i < 15; i++) {
                this->pieceBitboard[i] = 0;
            }

            this->whiteAttackBitboard = 0;
            this->blackAttackBitboard = 0;

            for (int i = 0; i < 15; i++) {
                this->pieceAttackBitboard[i] = 0;
            }
        }
};

/**
 * @brief Stellt ein Schachbrett dar.
 * Die Klasse Board stellt ein Schachbrett dar und enthält Methoden zur Zuggeneration.
 */
class Board {
    friend class Movegen;
    
    private:
         /**
          * @brief Stellt das Schachbrett in 8x8 Notation dar.
          */
        alignas(64) int32_t pieces[64] = {
            WHITE_ROOK, WHITE_KNIGHT, WHITE_BISHOP, WHITE_QUEEN, WHITE_KING, WHITE_BISHOP, WHITE_KNIGHT, WHITE_ROOK,
            WHITE_PAWN,   WHITE_PAWN,   WHITE_PAWN,  WHITE_PAWN, WHITE_PAWN,   WHITE_PAWN,   WHITE_PAWN, WHITE_PAWN,
                 EMPTY,        EMPTY,        EMPTY,       EMPTY,      EMPTY,        EMPTY,        EMPTY,      EMPTY,
                 EMPTY,        EMPTY,        EMPTY,       EMPTY,      EMPTY,        EMPTY,        EMPTY,      EMPTY,
                 EMPTY,        EMPTY,        EMPTY,       EMPTY,      EMPTY,        EMPTY,        EMPTY,      EMPTY,
                 EMPTY,        EMPTY,        EMPTY,       EMPTY,      EMPTY,        EMPTY,        EMPTY,      EMPTY,
            BLACK_PAWN,   BLACK_PAWN,   BLACK_PAWN,  BLACK_PAWN, BLACK_PAWN,   BLACK_PAWN,   BLACK_PAWN, BLACK_PAWN,
            BLACK_ROOK, BLACK_KNIGHT, BLACK_BISHOP, BLACK_QUEEN, BLACK_KING, BLACK_BISHOP, BLACK_KNIGHT, BLACK_ROOK
        };

        /**
         * @brief Speichert Belegbitboards für alle Figurentypen.
         */
        alignas(64) Bitboard pieceBitboard[15] = {
            0x0ULL,
            0xff00ULL,
            0x42ULL,
            0x24ULL,
            0x81ULL,
            0x8ULL,
            0x10ULL,
            0x0ULL,
            0x0ULL,
            0xff000000000000ULL,
            0x4200000000000000ULL,
            0x2400000000000000ULL,
            0x8100000000000000ULL,
            0x800000000000000ULL,
            0x1000000000000000ULL
        };

        /**
         * @brief Speichert die Farbe, die am Zug ist.
         */
        int32_t side;

        /**
         * @brief Speichert den Index des Feldes, auf dem ein Bauer En Passant geschlagen werden kann(wenn vorhanden).
         */
        int32_t enPassantSquare;

        /**
         * @brief Speichert die Anzahl der Halbzüge, die seit dem letzten Bauer- oder Schlagzug vergangen sind.
         */
        int32_t fiftyMoveRule;

        /**
         * @brief Speichert alle noch offenen Rochaden.
         */
        int32_t castlingPermission;

        /**
         * @brief Speichert die Anzahl der Halbzüge, die seit dem Anfang des Spiels vergangen sind.
         */
        uint16_t ply;

        /**
         * @brief Der Zobristhash des Schachbretts.
         * https://www.chessprogramming.org/Zobrist_Hashing
         */
        uint64_t hashValue;

        /**
         * @brief Speichert alle von Figuren(nicht Könige) belegten Felder.
         */
        Bitboard allPiecesBitboard;

        /**
         * @brief Speichert alle Felder, auf denen sich weiße Figuren(außer König) befinden.
         */
        Bitboard whitePiecesBitboard;

        /**
         * @brief Speichert alle Felder, auf denen sich schwarze Figuren(außer König) befinden.
         */
        Bitboard blackPiecesBitboard;

        /**
         * @brief Speichert alle Felder, die Weiß angreift(In Pseudo-Legalen Zügen).
         */
        Bitboard whiteAttackBitboard;

        /**
         * @brief Speichert alle Felder, die Schwarz angreift(In Pseudo-Legalen Zügen).
         */
        Bitboard blackAttackBitboard;

        /**
         * @brief Speichert alle Felder, die ein Figurentyp angreift(In Pseudo-Legalen Zügen).
         */
        Bitboard pieceAttackBitboard[15];

        /**
         * @brief Speichert alle gespielten Züge und notwendige Informationen um diesen effizient rückgängig zu machen.
         */
        std::vector<MoveHistoryEntry> moveHistory = {};

        /**
         * @brief Generiert einen Zobrist-Hash für das aktuelle Schachbrett.
         */
        uint64_t generateHashValue();

        /**
         * @brief Aktualisiert die Angriffsbitboards der Figuren und
         * die allgemeinen Figurenbitboards von beiden Seiten.
         */
        void generateSpecialBitboards();

        /**
         * @brief Generiert ein Bitboard mit allen Feldern, die von einer Seite angegriffen werden.
         * Die Angriffsbitboard der einzelnen Figurentypen werden aktualisiert.
         * 
         * @param side Die Seite.
         */
        Bitboard generateAttackBitboard(int32_t side);

        /**
         * @brief Generiert ein Bitboard mit allen Feldern, die von einer Seite angegriffen werden.
         * Die Angriffsbitboard der einzelnen Figurentypen werden aktualisiert. Es werden nur
         * notwendige Bitboards aktualisiert, um die Effizienz zu steigern.
         * 
         * @param side Die Seite.
         * @param updatedSquares Das Bitboard, das alle Felder enthält, dessen Belegung sich geändert hat.
         * @param capturedPiece Der Typ der geschlagenen Figur, oder EMPTY, wenn keine Figur geschlagen wurde.
         * @param wasPromotion Gibt an, ob der letzte Zug eine Bauernaufwertung war.
         */
        Bitboard generateAttackBitboard(int32_t side, Bitboard updatedSquares, int32_t capturedPiece, bool wasPromotion);

        /**
         * @brief Generiert ein Bitboard, das alle Felder enthält, auf denen sich gefesselte Figuren befinden.
         * Außerdem wird die Richtung, aus der die Figuren gefesselt sind, in einem 64 int großen Array gespeichert.
         * 
         * @param side Die Seite, für die gefesselte Figuren gefunden werden sollen.
         * @param pinnedPiecesBitboard Das Bitboard, das alle gefesselten Figuren enthält.
         * @param pinnedPiecesDirection Das Array, in dem die Richtung, aus der die Figuren gefesselt sind, gespeichert wird(muss mind. 8 groß sein).
         */
        void generatePinnedPiecesBitboards(int32_t side, Bitboard& pinnedPiecesBitboard,
                                           int32_t* pinnedPiecesDirection) const;

    public:
        /**
         * @brief Erstellt ein neues Schachbrett.
         */
        Board();

        /**
         * @brief Erstellt ein neues Schachbrett, das eine Kopie eines anderen Schachbretts ist.
         * @param other Das andere Schachbrett.
         */
        Board(const Board& other);

        /**
         * @brief Movekonstruktor.
         */
        Board(Board&& other);

        /**
         * @brief Erstellt ausgehend von einem FEN-String ein neues Schachbrett.
         * @param fen Die FEN-Notation des Schachbretts.
         */
        Board(std::string fen);

        /**
         * @brief Erstellt ausgehend von einem PGN-String ein neues Schachbrett.
         * @param pgn Die PGN-Notation des Schachbretts.
         */
        static Board fromPGN(std::string pgn);

        /**
         * @brief Erstellt ein neues Schachbrett, das eine Kopie eines anderen Schachbretts ist.
         * @param other Das andere Schachbrett.
         */
        Board& operator=(const Board& other);

        /**
         * @brief Movezuweisungsoperator.
         */
        Board& operator=(Board&& other);

        virtual ~Board();

        constexpr uint64_t getHashValue() const { return hashValue; };

        /**
         * @brief Überprüft, ob ein Zug legal ist.
         * Diese Variante der Legalitätsüberprüfung ist ineffizient,
         * weil zunächst überprüft wird, ob der Zug ein Pseudo-Legaler Zug ist und anschließend,
         * ob der Zug den eigenen König im Schach lässt.
         * 
         * @param move Der Zug.
         */
        bool isMoveLegal(Move move);

        /**
         * @brief Überprüft, ob der Spieler, der am Zug ist, im Schach steht.
         */
        bool isCheck() const;

        /**
         * @brief Generiert alle legalen Züge.
         * Legale Züge sind Züge, die auf dem Schachbrett möglich sind und den eigenen König nicht im Schach lassen.
         */
        Array<Move, 256> generateLegalMoves() const noexcept;

        /**
         * @brief Generiert alle legalen Züge.
         * Legale Züge sind Züge, die auf dem Schachbrett möglich sind und den eigenen König nicht im Schach lassen.
         *
         * @param moves Das Array, in das die Züge gespeichert werden sollen.
         */
        void generateLegalMoves(Array<Move, 256>& moves) const noexcept;

        /**
         * @brief Generiert alle legalen Züge, die eine Figur schlagen
         */
        Array<Move, 256> generateLegalCaptures() const noexcept;

        /**
         * @brief Generiert alle legalen Züge, die eine Figur schlagen
         *
         * @param moves Das Array, in das die Züge gespeichert werden sollen.
         */
        void generateLegalCaptures(Array<Move, 256>& moves) const noexcept;

        /**
         * @brief Führt einen Zug aus.
         */
        void makeMove(Move move);

        /**
         * @brief Macht den letzten gespielten Zug rückgängig.
         */
        void undoMove();

        /**
         * @brief Wandelt die aktuelle Stellung in eine FEN-Notation um.
         */
        std::string toFen() const;

        /**
         * @brief Wandelt das Spiel in einen PGN-String um.
         */
        std::string toPgn() const;

        /**
         * @brief Überprüft, ob ein Feld von einer bestimmten Seite angegriffen wird.
         * 
         * @param square Das Feld, das überprüft werden soll.
         * @param side Die Seite, die das Feld angreifen soll.
         */
        bool squareAttacked(int32_t square, int32_t side) const;

        /**
         * @brief Überprüft bei einem anzugebenden Belegbitboard, ob ein Feld von einer bestimmten Seite angegriffen wird.
         * 
         * @param sq Das Feld, das überprüft werden soll.
         * @param ownSide Die Seite, die das Feld angreifen soll.
         * @param occupied Das Bitboard mit den besetzten Feldern.
         */
        bool squareAttacked(int32_t sq, int32_t ownSide, Bitboard occupied) const;

        /**
         * @brief Überprüft bei einem anzugebenden Belegbitboard, ob ein Feld von einer bestimmten Seite angegriffen wird.
         * 
         * @param square Das Feld, das überprüft werden soll.
         * @param side Die Seite, die das Feld angreifen soll.
         * @param occupied Das Bitboard mit den besetzten Feldern.
         * @param attackingRays Gibt dein Bitboard mit allen Angreifern zurück. Bei laufenden Figuren sind außerdem sind außerdem die Verbindungsfelder mit enthalten.
         */
        bool squareAttacked(int32_t square, int32_t side, Bitboard occupied, Bitboard& attackingRays) const;

        /**
         * @brief Gibt die Anzahl der angreifenden Figuren eines Feldes zurück.
         * 
         * @param square Das Feld, das überprüft werden soll.
         * @param side Die Seite, die das Feld angreifen soll.
         * @param occupied Das Bitboard mit den besetzten Feldern.
         */
        int32_t numSquareAttackers(int32_t square, int32_t side, Bitboard occupied) const;

        /**
         * @brief Gibt die Anzahl der angreifenden Figuren eines Feldes zurück.
         * 
         * @param square Das Feld, das überprüft werden soll.
         * @param side Die Seite, die das Feld angreifen soll.
         * @param occupied Das Bitboard mit den besetzten Feldern.
         * @param attackingRays Gibt dein Bitboard mit allen Angreifern zurück. Bei laufenden Figuren sind außerdem sind außerdem die Verbindungsfelder mit enthalten.
         */
        int32_t numSquareAttackers(int32_t square, int32_t side, Bitboard occupied, Bitboard& attackingRays) const;

        /**
         * @brief Gibt die Figur auf einem Feld zurück.
         */
        constexpr int32_t pieceAt(int32_t square) const { return pieces[square]; };

        /**
         * @brief Gibt die Seite zurück, die am Zug ist.
         */
        constexpr int32_t getSideToMove() const { return side; };

        /**
         * @brief Gibt en letzten gespielten Zug zurück.
         */
        inline Move getLastMove() const { 
            if(moveHistory.size() > 0)
                return moveHistory.back().move; 
            else
                return Move();
        };

        /**
         * @brief Gibt den neuesten Eintrag der Zughistorie zurück.
         */
        inline const MoveHistoryEntry& getLastMoveHistoryEntry() const { 
            return moveHistory.back();
        };

        constexpr const std::vector<MoveHistoryEntry>& getMoveHistory() const { return moveHistory; };

        /**
         * @brief Gibt alle Positionen eines bestimmten Figurentyps als Bitboard zurück.
         */
        constexpr Bitboard getPieceBitboard(int32_t piece) const { return pieceBitboard[piece]; };

        /**
         * @brief Gibt alle Felder zurück, die von einer bestimmten Figur angegriffen werden.
         */
        constexpr Bitboard getPieceAttackBitboard(int32_t piece) const { return pieceAttackBitboard[piece]; };

        /**
         * @brief Gibt alle Felder zurück, die von einer bestimmten Seite angegriffen werden.
         */
        constexpr Bitboard getAttackBitboard(int32_t side) const { return side == WHITE ? whiteAttackBitboard : blackAttackBitboard; };

        /**
         * @brief Gibt alle Felder zurück, auf denen eine Figur steht(Könige ausgenommen).
         */
        constexpr Bitboard getOccupiedBitboard() const { return allPiecesBitboard; };

        /**
         * @brief Gibt alle Felder zurück, auf denen eine weiße Figur steht(Könige ausgenommen).
         */
        constexpr Bitboard getWhiteOccupiedBitboard() const { return whitePiecesBitboard; };

        /**
         * @brief Gibt alle Felder zurück, auf denen eine schwarze Figur steht(Könige ausgenommen).
         */
        constexpr Bitboard getBlackOccupiedBitboard() const { return blackPiecesBitboard; };

        /**
         * @brief Überprüft, wie häufig die momentane Position schon aufgetreten ist. 
         */
        uint16_t repetitionCount() const;

        /**
         * @brief Gibt die Anzahl der Halbzüge zurück, die gespielt wurden.
         */
        constexpr uint16_t getPly() const { return ply; };

        /**
         * @brief Gibt die Anzahl der Halbzüge zurück, die seit dem letzten Bauernzug oder Schlagzug gespielt wurden.
         */
        constexpr int32_t getFiftyMoveCounter() const { return fiftyMoveRule; };

        /**
         * @brief Gibt das Feld zurück, auf dem En-Passant geschlagen werden kann.
         */
        constexpr int32_t getEnPassantSquare() const { return enPassantSquare; };

        /**
         * @brief Gibt das Feld zurück, auf dem der weiße König steht.
         */
        constexpr int32_t getWhiteKingSquare() const { return pieceBitboard[WHITE_KING].getFirstSetBit(); };

        /**
         * @brief Gibt das Feld zurück, auf dem der schwarze König steht.
         */
        constexpr int32_t getBlackKingSquare() const { return pieceBitboard[BLACK_KING].getFirstSetBit(); };

        /**
         * @brief Gibt das Feld zurück, auf dem der König einer bestimmten Seite steht.
         */
        constexpr int32_t getKingSquare(int32_t side) const { return pieceBitboard[side | KING].getFirstSetBit(); };
};

#endif