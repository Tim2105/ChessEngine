#ifndef MOVE_H
#define MOVE_H

#include <stdint.h>

#define MOVE_QUIET 0
#define MOVE_DOUBLE_PAWN 1
#define MOVE_KINGSIDE_CASTLE 2
#define MOVE_QUEENSIDE_CASTLE 3
#define MOVE_CAPTURE 4
#define MOVE_EN_PASSANT 5
#define MOVE_PROMOTION 8
#define MOVE_PROMOTION_KNIGHT 8
#define MOVE_PROMOTION_BISHOP 9
#define MOVE_PROMOTION_ROOK 10
#define MOVE_PROMOTION_QUEEN 11

class Move {

    private:
        uint32_t move;

    public:
        Move();
        Move(int32_t origin, int32_t destination, int32_t flags);
        virtual ~Move();

        /**
         * @brief Gibt den Ausgangspunkt des Zuges zurück.
         */
        int32_t getOrigin();

        /**
         * @brief Gibt das Zielfeld des Zuges zurück.
         */
        int32_t getDestination();

        /**
         * @brief Überprüft ob ein Zug 'leise' ist, also keine Figur geschlagen wird und es kein Spezialzug ist.
         */
        bool isQuiet();

        /**
         * @brief Überprüft ob es sich um einen Doppelschritt handelt.
         */
        bool isDoublePawn();

        /**
         * @brief Überprüft ob es sich um einen Rochadezug handelt.
         */
        bool isCastle();

        /**
         * @brief Überprüft ob es sich um eine Rochade auf Königsseite handelt.
         */
        bool isKingsideCastle();

        /**
         * @brief Überprüft ob es sich um eine Rochade auf Damenseite handelt.
         */
        bool isQueensideCastle();

        /**
         * @brief Überprüft ob der Zug eine Figur schlägt.
         */
        bool isCapture();

        /**
         * @brief Überprüft ob es sich um einen En Passant Zug handelt.
         */
        bool isEnPassant();

        /**
         * @brief Überprüft ob es sich um einen Promotionzug handelt.
         */
        bool isPromotion();

        /**
         * @brief Überprüft ob es sich um einen Promotionzug auf Springer handelt.
         */
        bool isPromotionKnight();

        /**
         * @brief Überprüft ob es sich um einen Promotionzug auf Läufer handelt.
         */
        bool isPromotionBishop();

        /**
         * @brief Überprüft ob es sich um einen Promotionzug auf Turm handelt.
         */
        bool isPromotionRook();

        /**
         * @brief Überprüft ob es sich um einen Promotionzug auf Dame handelt.
         */
        bool isPromotionQueen();
};

#endif