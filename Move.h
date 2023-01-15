#ifndef MOVE_H
#define MOVE_H

#include <stdint.h>
#include <iostream>
#include "Definitions.h"

/**
 * @brief Kapselt einen Zug.
 * Die Klasse Move stellt alle Informationen über einen Zug in einem 32-bit Integer dar.
 * Die Bits 11-17 repräsentieren den Ausgangspunkt des Zuges, die Bits 4-10 das Zielfeld und die Bits 0-3 Flags für Spezialzüge.
 * Im Gegensatz zur Klasse Board ist diese Klasse weniger auf Performance sondern mehr auf Speichereffizienz ausgelegt.
 */
class Move {

    private:
        uint32_t move;

    public:
        Move();
        Move(int32_t origin, int32_t destination, int32_t flags);
        virtual ~Move();

        friend std::ostream& operator<< (std::ostream& os, Move& m);

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