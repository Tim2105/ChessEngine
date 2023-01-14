#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <vector>
#include "Move.h"

#define FR2SQ(f,r) ((21 + (f) ) + ( (r) * 10 ))

enum {
    EMPTY,
    WHITE_PAWN,
    WHITE_KNIGHT,
    WHITE_BISHOP,
    WHITE_ROOK,
    WHITE_QUEEN,
    WHITE_KING,
    BLACK_PAWN,
    BLACK_KNIGHT,
    BLACK_BISHOP,
    BLACK_ROOK,
    BLACK_QUEEN,
    BLACK_KING
};

enum {
    FILE_A,
    FILE_B,
    FILE_C,
    FILE_D,
    FILE_E,
    FILE_F,
    FILE_G,
    FILE_H,
    FILE_NONE
};

enum {
    RANK_1,
    RANK_2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8,
    RANK_NONE
};

enum {
    WHITE,
    BLACK,
    BOTH
};

enum {
    A1 = 21, B1, C1, D1, E1, F1, G1, H1,
    A2 = 31, B2, C2, D2, E2, F2, G2, H2,
    A3 = 41, B3, C3, D3, E3, F3, G3, H3,
    A4 = 51, B4, C4, D4, E4, F4, G4, H4,
    A5 = 61, B5, C5, D5, E5, F5, G5, H5,
    A6 = 71, B6, C6, D6, E6, F6, G6, H6,
    A7 = 81, B7, C7, D7, E7, F7, G7, H7,
    A8 = 91, B8, C8, D8, E8, F8, G8, H8, NO_SQ
};

enum {
    WHITE_KINGSIDE_CASTLE = 1,
    WHITE_QUEENSIDE_CASTLE = 2,
    BLACK_KINGSIDE_CASTLE = 4,
    BLACK_QUEENSIDE_CASTLE = 8
};

bool isSlidingPiece[13] = {
    false, false, false, true, true, true, false, false, false, true, true, true, false
};

std::vector<int32_t> directions[13] = {
    {},
    {},
    {-21,-19,-12, -8,  8, 12, 19, 21},
    {-11, -9,  9, 11},
    {-10, -1,  1, 10},
    {-11, -10, -9, -1, 1, 9, 10, 11},
    {-11, -10, -9, -1, 1, 9, 10, 11},
    {},
    {-21,-19,-12, -8,  8, 12, 19, 21},
    {-11, -9,  9, 11},
    {-10, -1,  1, 10},
    {-11, -10, -9, -1, 1, 9, 10, 11},
    {-11, -10, -9, -1, 1, 9, 10, 11}
};

/**
 * @brief Enthält alle notwendigen Informationen um einen Zug rückgängig zu machen.
 */
class MoveHistoryEntry {
    friend class Board;

    private:
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
        int32_t castlePermission;

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
    
    public:
        /**
         * @brief Erstellt einen neuen MoveHistoryEntry.
         * @param move Der Zug der rückgängig gemacht werden soll.
         * @param capturedPiece Speichert den Typ der geschlagenen Figur.
         * @param castlePermission Speichert alle möglichen Rochaden vor diesem Zug.
         * @param enPassantSquare Speichert die Position eines möglichen En Passant Zuges vor diesem Zug(wenn möglich).
         * @param fiftyMoveRule Der 50-Zug Counter vor diesem Zug.
         * @param hashValue Speichert den Hashwert vor diesem Zug.
         */
        MoveHistoryEntry(Move move, int32_t capturedPiece, int32_t castlePermission, int32_t enPassantSquare, int32_t fiftyMoveRule, uint64_t hashValue) {
            this->move = move;
            this->capturedPiece = capturedPiece;
            this->castlePermission = castlePermission;
            this->enPassantSquare = enPassantSquare;
            this->fiftyMoveRule = fiftyMoveRule;
            this->hashValue = hashValue;
        }
};

/**
 * @brief Stellt ein Schachbrett dar.
 * Die Klasse Board stellt ein Schachbrett dar und enthält Methoden zur Zuggeneration.
 */
class Board {

    private:
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
         */
        std::vector<int32_t> pieceList[13] = {
            {},
            {A2, B2, C2, D2, E2, F2, G2, H2},
            {B1, G1},
            {C1, F1},
            {A1, H1},
            {D1},
            {E1},
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
        int32_t enPassantIndex;

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
        int32_t ply;

        /**
         * @brief Der Zobristhash des Schachbretts.
         * https://www.chessprogramming.org/Zobrist_Hashing
         */
        uint64_t hashValue;

        /**
         * @brief Speichert alle gespielten Züge und notwendige Informationen um diesen effizient rückgängig zu machen.
         */
        std::vector<MoveHistoryEntry> moveHistory;

        /**
         * @brief Array zur Konvertierung der 10x12 Notation in 8x8 Notation.
         * Invalide Felder werden mit -1 markiert.
         */
        int32_t mailbox[120];

        /**
         * @brief Array zur Konvertierung der 8x8 Notation in 10x12 Notation.
         */
        int32_t mailbox64[64];

        /**
         * @brief Initialisiert die Mailbox-Arrays
         */
        void initMailbox();

    public:
        /**
         * @brief Erstellt ein neues Schachbrett.
         */
        Board();

        virtual ~Board();

        /**
         * @brief Generiert alle Pseudo-Legalen Züge.
         * Pseudo-Legale Züge sind Züge, die auf dem Schachbrett möglich sind, aber eventuell den eigenen König im Schach lassen.
         */
        std::vector<int32_t> generatePseudoLegalMoves();
};

#endif