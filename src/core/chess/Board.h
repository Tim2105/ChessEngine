#ifndef BOARD_H
#define BOARD_H

#include "core/chess/BoardDefinitions.h"
#include "core/chess/Move.h"
#include "core/utils/Array.h"
#include "core/utils/Bitboard.h"

#include <cstring>
#include <limits>
#include <stdalign.h>
#include <stdint.h>
#include <string>
#include <sstream>
#include <tuple>
#include <vector>

/**
 * @brief Enthält alle notwendigen Informationen um einen Zug rückgängig zu machen.
 */
class MoveHistoryEntry {
    public:
        /**
         * @brief Speichert den Typ der geschlagenen Figur.
         */
        int capturedPiece;

        /**
         * @brief Speichert alle möglichen Rochaden vor diesem Zug.
         */
        int castlingPermission;

        /**
         * @brief Speichert die Position eines möglichen En Passant Zuges vor diesem Zug(wenn möglich).
         */
        int enPassantSquare;

        /**
         * @brief Der 50-Zug Counter vor diesem Zug.
         */
        int fiftyMoveRule;

        /**
         * @brief Speichert den Hashwert vor diesem Zug.
         */
        uint64_t hashValue;

        /**
         * @brief Speichert alle individuellen Figurenbitboards vor diesem Zug.
        */
        Bitboard pieceBitboard[15];

        /**
         * @brief Speichert die Angriffsbitboards der Figuren vor diesem Zug.
         */
        Bitboard attackBitboard[15];

        /**
         * @brief Der Zug der rückgängig gemacht werden soll.
         */
        Move move;

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
        constexpr MoveHistoryEntry(Move move, int capturedPiece, int castlePermission,
                        int enPassantSquare, int fiftyMoveRule, uint64_t hashValue,
                        Bitboard pieceBitboards[15], Bitboard attackBitboards[15]) {
            this->move = move;
            this->capturedPiece = capturedPiece;
            this->castlingPermission = castlePermission;
            this->enPassantSquare = enPassantSquare;
            this->fiftyMoveRule = fiftyMoveRule;
            this->hashValue = hashValue;

            std::copy(pieceBitboards, pieceBitboards + 15, this->pieceBitboard);
            std::copy(attackBitboards, attackBitboards + 15, this->attackBitboard);
        }

        /**
         * @brief Erstellt einen neuen MoveHistoryEntry.
         * @param move Der Zug der rückgängig gemacht werden soll.
         */
        constexpr MoveHistoryEntry(Move move) {
            this->move = move;
        }
};

/**
 * @brief Enthält Meta-Informationen eines PGN-Strings.
 */
struct PGNData {
    enum Result {
        ONGOING,
        WHITE_WINS,
        BLACK_WINS,
        DRAW
    };

    /**
     * @brief Speichert das Seven Tag Roster (oder beliebige andere Schlüssel-Wert-Paare).
     */
    std::vector<std::tuple<std::string, std::string>> metadata;

    /**
     * @brief Speichert Kommentare in { - Klammern als Tupel
     * mit dem Halbzug, hinter dem der Kommentar steht.
     */
    std::vector<std::tuple<std::string, size_t>> comments;

    /**
     * @brief Speichert das Ergebnis des Spiels.
     * "1-0": Weiß gewinnt,
     * "0-1": Schwarz gewinnt,
     * "1/2-1/2": Remis,
     * "*": Spiel läuft noch
     */
    Result result = ONGOING;

    friend std::ostream& operator<<(std::ostream& os, const PGNData& data);

    std::string metadataToString() const {
        std::stringstream ss;
        for(auto& [key, value] : metadata)
            ss << "[" << key << ": \"" << value << "\"]\n";

        return ss.str();
    }
};

/**
 * @brief Gibt die PGN-Metadaten aus.
 */
inline std::ostream& operator<<(std::ostream& os, const PGNData& data) {
    os << data.metadataToString();

    for(auto& [comment, halfmove] : data.comments)
        os << "(" << halfmove << ") \""  << comment << "\"\n";

    os << "Result: ";
    switch(data.result) {
        case PGNData::WHITE_WINS:
            os << "1-0";
            break;
        case PGNData::BLACK_WINS:
            os << "0-1";
            break;
        case PGNData::DRAW:
            os << "1/2-1/2";
            break;
        case PGNData::ONGOING:
            os << "*";
            break;
    }

    os << std::endl;

    return os;
}

/**
 * @brief Stellt ein Schachbrett dar.
 * Die Klasse Board stellt ein Schachbrett dar und enthält Methoden zur Zuggeneration.
 */
class alignas(64) Board {
    friend class Movegen;
    
    private:
         /**
          * @brief Stellt das Schachbrett in 8x8 Notation dar.
          */
        int pieces[64] = {
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
         * @brief Speichert Belegbitboards für alle Figurentypen und Farben.
         */
        Bitboard pieceBitboard[15] = {
            0xffefULL,
            0xff00ULL,
            0x42ULL,
            0x24ULL,
            0x81ULL,
            0x8ULL,
            0x10ULL,
            0xefff00000000ffefULL,
            0xefff000000000000ULL,
            0xff000000000000ULL,
            0x4200000000000000ULL,
            0x2400000000000000ULL,
            0x8100000000000000ULL,
            0x800000000000000ULL,
            0x1000000000000000ULL
        };

        /**
         * @brief Speichert alle Felder, die ein Figurentyp angreift(In Pseudo-Legalen Zügen).
         */
        Bitboard attackBitboard[15] = {
            0xffff7eULL,
            0xff0000ULL,
            0xa51800ULL,
            0x5a00ULL,
            0x8142ULL,
            0x1c14ULL,
            0x3828ULL,
            0x7effff0000ffff7eULL,
            0x7effff0000000000ULL,
            0xff0000000000ULL,
            0x18a50000000000ULL,
            0x5a000000000000ULL,
            0x4281000000000000ULL,
            0x141c000000000000ULL,
            0x2838000000000000ULL
        };

        /**
         * @brief Speichert die Farbe, die am Zug ist.
         */
        int side = WHITE;

        /**
         * @brief Speichert den Index des Feldes, auf dem ein Bauer En Passant geschlagen werden kann(wenn vorhanden).
         */
        int enPassantSquare = NO_SQ;

        /**
         * @brief Speichert die Anzahl der Halbzüge, die seit dem letzten Bauer- oder Schlagzug vergangen sind.
         */
        int fiftyMoveRule = 0;

        /**
         * @brief Speichert alle noch offenen Rochaden.
         */
        int castlingPermission = WHITE_KINGSIDE_CASTLE  |
                                 WHITE_QUEENSIDE_CASTLE |
                                 BLACK_KINGSIDE_CASTLE  |
                                 BLACK_QUEENSIDE_CASTLE;

        /**
         * @brief Ein Zobristhash des Schachbretts.
         * https://www.chessprogramming.org/Zobrist_Hashing
         */
        uint64_t hashValue;

        /**
         * @brief Speichert alle gespielten Züge und notwendige Informationen um diesen effizient rückgängig zu machen.
         */
        std::vector<MoveHistoryEntry> moveHistory;

        /**
         * @brief Speichert die Anzahl der Halbzüge, die seit dem Anfang des Spiels vergangen sind.
         */
        unsigned int age = 0;

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
         * @brief Aktualisiert die Angriffsbitboards der Figuren und
         * die allgemeinen Figurenbitboards von beiden Seiten.
         * 
         * @param side Die Seite.
         */
        void updateAttackBitboards(int side);

        /**
         * @brief Aktualisiert die Angriffsbitboards der Figuren und
         * die allgemeinen Figurenbitboards von beiden Seiten.
         * Betrachtet dabei nur die Bitboards, die sich geändert haben.
         * 
         * @param side Die Seite.
         * @param updatedSquares Das Bitboard, das alle Felder enthält, dessen Belegung sich geändert hat.
         * @param capturedPiece Der Typ der geschlagenen Figur, oder EMPTY, wenn keine Figur geschlagen wurde.
         * @param wasPromotion Gibt an, ob der letzte Zug eine Bauernaufwertung war.
         */
        void updateAttackBitboards(int side, Bitboard updatedSquares, int capturedPiece, bool wasPromotion);

        /**
         * @brief Generiert ein Bitboard, das alle Felder enthält, auf denen sich gefesselte Figuren befinden.
         * Außerdem wird die Richtung, aus der die Figuren gefesselt sind, in einem 64 int großen Array gespeichert.
         * 
         * @param side Die Seite, für die gefesselte Figuren gefunden werden sollen.
         * @param pinnedPiecesBitboard Das Bitboard, das alle gefesselten Figuren enthält.
         * @param pinnedPiecesDirection Das Array, in dem die Richtung, aus der die Figuren gefesselt sind, gespeichert wird(muss mind. 8 groß sein).
         */
        void generatePinnedPiecesBitboards(int side, Bitboard& pinnedPiecesBitboard,
                                           int* pinnedPiecesDirection) const;

    public:
        /**
         * @brief Erstellt ein neues Schachbrett.
         */
        Board();

        /**
         * @brief Erstellt ausgehend von einem FEN-String ein neues Schachbrett.
         * @param fen Die FEN-Notation des Schachbretts.
         */
        Board(std::string fen);

        /**
         * @brief Liest aus einem Inputstream ein Schachbrett in PGN-Notation.
         * Der Inputstream wird sich nach dem Lesen am Ende des PGN-Strings befinden.
         * 
         * @param pgn Die PGN-Notation des Schachbretts als Inputstream.
         * @param numMoves Die Anzahl der Züge, die gelesen werden sollen.
         * 
         * @return Ein Tupel, das das Schachbrett und die PGN-Metadaten enthält.
         */
        static std::tuple<Board, PGNData> fromPGN(std::istream& pgn, size_t numMoves = std::numeric_limits<size_t>::max());

        /**
         * @brief Liest aus einem String ein Schachbrett in PGN-Notation.
         * 
         * @param pgn Die PGN-Notation des Schachbretts als String.
         * @param numMoves Die Anzahl der Züge, die gelesen werden sollen.
         * 
         * @return Ein Tupel, das das Schachbrett und die PGN-Metadaten enthält.
         */
        static inline std::tuple<Board, PGNData> fromPGN(const std::string& pgn, size_t numMoves = std::numeric_limits<size_t>::max()) {
            std::stringstream ss(pgn);
            return fromPGN(ss, numMoves);
        }

        /**
         * @brief Überprüft, ob zwei Schachbretter die gleiche Position darstellen.
         * Dieser Operator betrachtet nicht die Zughistorie, sondern nur die aktuelle Position.
         * 
         * @param other Das andere Schachbrett.
         */
        bool operator==(const Board& other) const;

        /**
         * @brief Überprüft, ob zwei Schachbretter unterschiedliche Positionen darstellen.
         * Dieser Operator betrachtet nicht die Zughistorie, sondern nur die aktuelle Position.
         * 
         * @param other Das andere Schachbrett.
         */
        inline bool operator!=(const Board& other) const { return !(*this == other); };

        /**
         * @brief Gibt einen Hashwert des Schachbretts zurück.
         */
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
        std::string toFEN() const;

        /**
         * @brief Wandelt das Spiel in einen PGN-String um.
         * 
         * @param data Die PGN-Metadaten.
         */
        std::string toPGN(const PGNData& data) const;

        /**
         * @brief Überprüft, ob ein Feld von einer bestimmten Seite angegriffen wird.
         * 
         * @param square Das Feld, das überprüft werden soll.
         * @param side Die Seite, die das Feld angreifen soll.
         */
        bool squareAttacked(int square, int side) const;

        /**
         * @brief Überprüft bei einem anzugebenden Belegbitboard, ob ein Feld von einer bestimmten Seite angegriffen wird.
         * 
         * @param sq Das Feld, das überprüft werden soll.
         * @param ownSide Die Seite, die das Feld angreifen soll.
         * @param occupied Das Bitboard mit den besetzten Feldern.
         */
        bool squareAttacked(int sq, int ownSide, Bitboard occupied) const;

        /**
         * @brief Überprüft bei einem anzugebenden Belegbitboard, ob ein Feld von einer bestimmten Seite angegriffen wird.
         * 
         * @param square Das Feld, das überprüft werden soll.
         * @param side Die Seite, die das Feld angreifen soll.
         * @param occupied Das Bitboard mit den besetzten Feldern.
         * @param attackingRays Gibt dein Bitboard mit allen Angreifern zurück. Bei laufenden Figuren sind außerdem sind außerdem die Verbindungsfelder mit enthalten.
         */
        bool squareAttacked(int square, int side, Bitboard occupied, Bitboard& attackingRays) const;

        /**
         * @brief Gibt die Anzahl der angreifenden Figuren eines Feldes zurück.
         * 
         * @param square Das Feld, das überprüft werden soll.
         * @param side Die Seite, die das Feld angreifen soll.
         * @param occupied Das Bitboard mit den besetzten Feldern.
         */
        int numSquareAttackers(int square, int side, Bitboard occupied) const;

        /**
         * @brief Gibt die Anzahl der angreifenden Figuren eines Feldes zurück.
         * 
         * @param square Das Feld, das überprüft werden soll.
         * @param side Die Seite, die das Feld angreifen soll.
         * @param occupied Das Bitboard mit den besetzten Feldern.
         * @param attackingRays Gibt dein Bitboard mit allen Angreifern zurück. Bei laufenden Figuren sind außerdem sind außerdem die Verbindungsfelder mit enthalten.
         */
        int numSquareAttackers(int square, int side, Bitboard occupied, Bitboard& attackingRays) const;

        /**
         * @brief Gibt die Figur auf einem Feld zurück.
         */
        constexpr int pieceAt(int square) const { return pieces[square]; };

        /**
         * @brief Gibt die Seite zurück, die am Zug ist.
         */
        constexpr int getSideToMove() const { return side; };

        /**
         * @brief Gibt die Rochadenberechtigung zurück.
         */
        constexpr int getCastlingPermission() const { return castlingPermission; };

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
         * @brief Gibt alle Positionen eines bestimmten Figurentyps oder einer Farbe zurück.
         */
        constexpr Bitboard getPieceBitboard(int piece) const { return pieceBitboard[piece]; };

        /**
         * @brief Gibt alle Felder zurück, auf denen eine Figur steht(Könige ausgenommen).
         */
        constexpr Bitboard getPieceBitboard() const { return pieceBitboard[ALL_PIECES]; };

        /**
         * @brief Gibt alle Felder zurück, die von einer bestimmten Figur oder Farbe angegriffen werden.
         */
        constexpr Bitboard getAttackBitboard(int piece) const { return attackBitboard[piece]; };

        /**
         * @brief Gibt alle Felder zurück, die von irgendeiner Figur angegriffen werden.
         */
        constexpr Bitboard getAttackBitboard() const { return attackBitboard[ALL_PIECES]; };

        /**
         * @brief Überprüft, wie häufig die momentane Position schon aufgetreten ist. 
         */
        unsigned int repetitionCount() const;

        /**
         * @brief Gibt die Anzahl der Halbzüge zurück, die gespielt wurden.
         */
        constexpr unsigned int getAge() const { return age; };

        /**
         * @brief Setzt die Anzahl der Halbzüge, die gespielt wurden.
         */
        constexpr void setAge(unsigned int age) { this->age = age; };

        /**
         * @brief Gibt die Anzahl der Halbzüge zurück, die seit dem letzten Bauernzug oder Schlagzug gespielt wurden.
         */
        constexpr int getFiftyMoveCounter() const { return fiftyMoveRule; };

        /**
         * @brief Gibt das Feld zurück, auf dem En-Passant geschlagen werden kann.
         */
        constexpr int getEnPassantSquare() const { return enPassantSquare; };

        /**
         * @brief Gibt das Feld zurück, auf dem der weiße König steht.
         */
        constexpr int getWhiteKingSquare() const { return pieceBitboard[WHITE_KING].getFSB(); };

        /**
         * @brief Gibt das Feld zurück, auf dem der schwarze König steht.
         */
        constexpr int getBlackKingSquare() const { return pieceBitboard[BLACK_KING].getFSB(); };

        /**
         * @brief Gibt das Feld zurück, auf dem der König einer bestimmten Seite steht.
         */
        constexpr int getKingSquare(int side) const { return pieceBitboard[side | KING].getFSB(); };
};

#endif