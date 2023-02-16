#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <stdint.h>
#include "core/chess/Move.h"
#include "core/chess/BoardDefinitions.h"
#include <string>
#include "core/utils/Bitboard.h"
#include "core/chess/Movegen.h"
#include "core/utils/Array.h"

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
         * @param whiteAttackBitboard Speichert das weiße Angriffsbitboard vor diesem Zug.
         * @param blackAttackBitboard Speichert das schwarze Angriffsbitboard vor diesem Zug.
         */
        constexpr MoveHistoryEntry(Move move, int32_t capturedPiece, int32_t castlePermission,
                        int32_t enPassantSquare, int32_t fiftyMoveRule, uint64_t hashValue,
                        Bitboard whiteAttackBitboard, Bitboard blackAttackBitboard, Bitboard pieceAttackBitboards[15]) {
            this->move = move;
            this->capturedPiece = capturedPiece;
            this->castlingPermission = castlePermission;
            this->enPassantSquare = enPassantSquare;
            this->fiftyMoveRule = fiftyMoveRule;
            this->hashValue = hashValue;
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
        constexpr MoveHistoryEntry(Move move) {
            this->move = move;
            this->capturedPiece = EMPTY;
            this->castlingPermission = 0;
            this->enPassantSquare = 0;
            this->fiftyMoveRule = 0;
            this->hashValue = 0;
            this->whiteAttackBitboard = 0;
            this->blackAttackBitboard = 0;

            for (int i = 0; i < 15; i++) {
                this->pieceAttackBitboard[i] = 0;
            }
        }
};

/**
 * @brief Speichert die Anzahl der Wiederholungen für alle, bereits gesehenen, Stellungen.
 */
class RepetitionTable {
    private:
        struct Entry {
            uint64_t key;
            uint8_t count;
        };

        std::vector<Entry> entries;
    
    public:
        inline void increment(uint64_t key) {
            for(auto it = entries.rbegin(); it != entries.rend(); it++) {
                if (it->key == key) {
                    it->count++;
                    return;
                }
            }

            entries.push_back({key, 1});
        }

        inline void decrement(uint64_t key) {
            for(auto it = entries.rbegin(); it != entries.rend(); it++) {
                if (it->key == key) {
                    it->count--;
                    if (it->count == 0) {
                        entries.erase(std::next(it).base());
                    }
                    
                    return;
                }
            }
        }

        inline uint8_t get(uint64_t key) {
            for(auto it = entries.rbegin(); it != entries.rend(); it++) {
                if (it->key == key) {
                    return it->count;
                }
            }

            return 0;
        }
};

/**
 * @brief Stellt ein Schachbrett dar.
 * Die Klasse Board stellt ein Schachbrett dar und enthält Methoden zur Zuggeneration.
 * Dieser Teil des Programms ist sehr Performancekritisch und folgt daher nicht strikt den Regeln der Objektorientierten Programmierung.
 */
class Board {
    friend class Movegen;
    
    private:
        inline static bool initialized = false;

         /**
          * @brief Stellt das Schachbrett in 10x12 Notation dar.
          * https://www.chessprogramming.org/10x12_Board
          */
        int32_t pieces[120] = {
            EMPTY,      EMPTY,        EMPTY,        EMPTY,       EMPTY,      EMPTY,        EMPTY,        EMPTY,      EMPTY, EMPTY,
            EMPTY,      EMPTY,        EMPTY,        EMPTY,       EMPTY,      EMPTY,        EMPTY,        EMPTY,      EMPTY, EMPTY,
            EMPTY, WHITE_ROOK, WHITE_KNIGHT, WHITE_BISHOP, WHITE_QUEEN, WHITE_KING, WHITE_BISHOP, WHITE_KNIGHT, WHITE_ROOK, EMPTY,
            EMPTY, WHITE_PAWN,   WHITE_PAWN,   WHITE_PAWN,  WHITE_PAWN, WHITE_PAWN,   WHITE_PAWN,   WHITE_PAWN, WHITE_PAWN, EMPTY,
            EMPTY,      EMPTY,        EMPTY,        EMPTY,       EMPTY,      EMPTY,        EMPTY,        EMPTY,      EMPTY, EMPTY,
            EMPTY,      EMPTY,        EMPTY,        EMPTY,       EMPTY,      EMPTY,        EMPTY,        EMPTY,      EMPTY, EMPTY,
            EMPTY,      EMPTY,        EMPTY,        EMPTY,       EMPTY,      EMPTY,        EMPTY,        EMPTY,      EMPTY, EMPTY,
            EMPTY,      EMPTY,        EMPTY,        EMPTY,       EMPTY,      EMPTY,        EMPTY,        EMPTY,      EMPTY, EMPTY,
            EMPTY, BLACK_PAWN,   BLACK_PAWN,   BLACK_PAWN,  BLACK_PAWN, BLACK_PAWN,   BLACK_PAWN,   BLACK_PAWN, BLACK_PAWN, EMPTY,
            EMPTY, BLACK_ROOK, BLACK_KNIGHT, BLACK_BISHOP, BLACK_QUEEN, BLACK_KING, BLACK_BISHOP, BLACK_KNIGHT, BLACK_ROOK, EMPTY,
            EMPTY,      EMPTY,        EMPTY,        EMPTY,       EMPTY,      EMPTY,        EMPTY,        EMPTY,      EMPTY, EMPTY,
            EMPTY,      EMPTY,        EMPTY,        EMPTY,       EMPTY,      EMPTY,        EMPTY,        EMPTY,      EMPTY, EMPTY
        };

        /**
         * @brief Enthält eine Liste aller Figuren für jeden Figurentyp auf dem Schachbrett.
         * Speichert den Index der Position.
         * Figurenlisten haben die Größe 9,
         * weil die ein(e) Springer/Läufer/Dame mit 8 Bauernaufwertungen 9-mal auf dem Schachbrett sein können.
         */
        Array<int32_t, 9> pieceList[15] = {
            {},
            {A2, B2, C2, D2, E2, F2, G2, H2},
            {B1, G1},
            {C1, F1},
            {A1, H1},
            {D1},
            {E1},
            {},
            {},
            {A7, B7, C7, D7, E7, F7, G7, H7},
            {B8, G8}, 
            {C8, F8},
            {A8, H8},
            {D8},
            {E8}
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
         * @brief Speichert alle Felder, auf denen sich Figuren befinden.
         */
        Bitboard pieceBitboard[15];

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
         * @brief Enthält alle, bereits gesehenen, Positionen und ihre Häufigkeit
         */
        RepetitionTable repetitionTable;

        /**
         * @brief Generiert einen Zobrist-Hash für das aktuelle Schachbrett.
         */
        uint64_t generateHashValue();

        /**
         * @brief Generiert ein Bitboard, das alle besetzten Felder enthält(König ausgeschlossen).
         */
        void generateBitboards();

        /**
         * @brief Generiert ein Bitboard mit allen Feldern, die von einer Seite angegriffen werden.
         * Die Angriffsbitboard der einzelnen Figurentypen werden aktualisiert.
         * 
         * @param side Die Seite.
         */
        Bitboard generateAttackBitboard(int32_t side);

        /**
         * @brief Generiert ein Bitboard, das alle Felder enthält, auf denen sich gefesselte Figuren befinden.
         * Außerdem wird die Richtung, aus der die Figuren gefesselt sind, in einem 64 int großen Array gespeichert(in 120er Notation).
         * 
         * @param side Die Seite, für die gefesselte Figuren gefunden werden sollen.
         * @param pinnedPiecesBitboard Das Bitboard, das alle gefesselten Figuren enthält.
         * @param pinnedPiecesDirection Das Array, in dem die Richtung, aus der die Figuren gefesselt sind, gespeichert wird(muss mind. 8 groß sein).
         */
        void generatePinnedPiecesBitboards(int32_t side, Bitboard& pinnedPiecesBitboard,
                                           int32_t* pinnedPiecesDirection);

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
         * @brief Erstellt ausgehend von einem FEN-String ein neues Schachbrett.
         * @param fen Die FEN-Notation des Schachbretts.
         */
        Board(std::string fen);

        /**
         * @brief Erstellt ein neues Schachbrett, das eine Kopie eines anderen Schachbretts ist.
         * @param other Das andere Schachbrett.
         */
        Board& operator=(const Board& other);

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
        bool isCheck();

        /**
         * @brief Generiert alle Pseudo-Legalen Züge.
         * Pseudo-Legale Züge sind Züge, die auf dem Schachbrett möglich sind, aber eventuell den eigenen König im Schach lassen.
         */
        Array<Move, 256> generatePseudoLegalMoves();

        /**
         * @brief Generiert alle legalen Züge.
         * Legale Züge sind Züge, die auf dem Schachbrett möglich sind und den eigenen König nicht im Schach lassen.
         */
        Array<Move, 256> generateLegalMoves();

        /**
         * @brief Generiert alle legalen Züge, die eine Figur schlagen
         */
        Array<Move, 256> generateLegalCaptures();

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
        std::string fenString() const;

        /**
         * @brief Wandelt das Spiel in einen PGN-String um.
         */
        std::string pgnString() const;

        /**
         * @brief Überprüft bei einem anzugebenden Belegbitboard, ob ein Feld von einer bestimmten Seite angegriffen wird.
         * 
         * @param square Das Feld, das überprüft werden soll(in 120er Notation).
         * @param side Die Seite, die das Feld angreifen soll.
         */
        bool squareAttacked(int32_t square, int32_t side);

        /**
         * @brief Überprüft bei einem anzugebenden Belegbitboard, ob ein Feld von einer bestimmten Seite angegriffen wird.
         * 
         * @param sq120 Das Feld, das überprüft werden soll(in 120er Notation).
         * @param ownSide Die Seite, die das Feld angreifen soll.
         * @param occupied Das Bitboard mit den besetzten Feldern.
         */
        bool squareAttacked(int32_t sq120, int32_t ownSide, Bitboard occupied);

        /**
         * @brief Überprüft bei einem anzugebenden Belegbitboard, ob ein Feld von einer bestimmten Seite angegriffen wird.
         * 
         * @param square Das Feld, das überprüft werden soll(in 120er Notation).
         * @param side Die Seite, die das Feld angreifen soll.
         * @param occupied Das Bitboard mit den besetzten Feldern.
         * @param attackingRays Gibt dein Bitboard mit allen Angreifern zurück. Bei laufenden Figuren sind außerdem sind außerdem die Verbindungsfelder mit enthalten.
         */
        bool squareAttacked(int32_t square, int32_t side, Bitboard occupied, Bitboard& attackingRays);

        /**
         * @brief Gibt die Anzahl der angreifenden Figuren eines Feldes zurück.
         * 
         * @param square Das Feld, das überprüft werden soll(in 120er Notation).
         * @param side Die Seite, die das Feld angreifen soll.
         * @param occupied Das Bitboard mit den besetzten Feldern.
         */
        int32_t numSquareAttackers(int32_t square, int32_t side, Bitboard occupied);

        /**
         * @brief Gibt die Anzahl der angreifenden Figuren eines Feldes zurück.
         * 
         * @param square Das Feld, das überprüft werden soll(in 120er Notation).
         * @param side Die Seite, die das Feld angreifen soll.
         * @param occupied Das Bitboard mit den besetzten Feldern.
         * @param attackingRays Gibt dein Bitboard mit allen Angreifern zurück. Bei laufenden Figuren sind außerdem sind außerdem die Verbindungsfelder mit enthalten.
         */
        int32_t numSquareAttackers(int32_t square, int32_t side, Bitboard occupied, Bitboard& attackingRays);

        /**
         * @brief Gibt die Figur auf einem Feld zurück.
         */
        inline int32_t pieceAt(int32_t square) const { return pieces[square]; };

        /**
         * @brief Gibt die Seite zurück, die am Zug ist.
         */
        inline int32_t getSideToMove() const { return side; };

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
        inline MoveHistoryEntry getLastMoveHistoryEntry() const { 
            return moveHistory.back();
        };

        inline std::vector<MoveHistoryEntry> getMoveHistory() const { return moveHistory; };

        /**
         * @brief Gibt alle Positionen eines bestimmten Figurentyps zurück.
         */
        inline Array<int32_t, 9> getPieceList(int32_t piece) const { return pieceList[piece]; };

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
        uint8_t repetitionCount();

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
        inline int32_t getWhiteKingSquare() const { return pieceList[WHITE_KING].front(); };

        /**
         * @brief Gibt das Feld zurück, auf dem der schwarze König steht.
         */
        inline int32_t getBlackKingSquare() const { return pieceList[BLACK_KING].front(); };

        /**
         * @brief Gibt das Feld zurück, auf dem der König einer bestimmten Seite steht.
         */
        inline int32_t getKingSquare(int32_t side) const { return pieceList[side | KING].front(); };
};

#endif