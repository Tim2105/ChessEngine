#ifndef MOVE_H
#define MOVE_H

#include <stdint.h>
#include <iostream>
#include "BoardDefinitions.h"
#include <functional>

/**
 * @brief Kapselt einen Zug.
 * Die Klasse Move stellt alle Informationen über einen Zug in einem 32-bit Integer dar.
 * Die Bits 11-17 repräsentieren den Ausgangspunkt des Zuges, die Bits 4-10 das Zielfeld und die Bits 0-3 Flags für Spezialzüge.
 * Das Bit 18 wird gesetzt sobald erkannt wird, dass der Zug das Spiel beendet(Schachmatt oder -Patt).
 * Im Gegensatz zur Klasse Board ist diese Klasse weniger auf Performance sondern mehr auf Speichereffizienz ausgelegt.
 */
class Move {

    private:
        uint32_t move;

    public:
        Move();
        Move(int32_t origin, int32_t destination, int32_t flags);
        virtual ~Move();

        friend std::ostream& operator<< (std::ostream& os, const Move& m);

        inline uint32_t getMove() const { return move; }

        /**
         * @brief Überprüft, ob der Zug Inhalt hat.
         */
        bool exists() const;

        /**
         * @brief Gibt den Zug als String zurück.
         */
        std::string toString() const;

        /**
         * @brief Gibt den Ausgangspunkt des Zuges zurück.
         */
        int32_t getOrigin() const;

        /**
         * @brief Gibt das Zielfeld des Zuges zurück.
         */
        int32_t getDestination() const;

        /**
         * @brief Überprüft ob ein Zug 'leise' ist, also keine Figur geschlagen wird und es kein Spezialzug ist.
         */
        bool isQuiet() const;

        /**
         * @brief Überprüft ob es sich um einen Doppelschritt handelt.
         */
        bool isDoublePawn() const;

        /**
         * @brief Überprüft ob es sich um einen Rochadezug handelt.
         */
        bool isCastle() const;

        /**
         * @brief Überprüft ob es sich um eine Rochade auf Königsseite handelt.
         */
        bool isKingsideCastle() const;

        /**
         * @brief Überprüft ob es sich um eine Rochade auf Damenseite handelt.
         */
        bool isQueensideCastle() const;

        /**
         * @brief Überprüft ob der Zug eine Figur schlägt.
         */
        bool isCapture() const;

        /**
         * @brief Überprüft ob es sich um einen En Passant Zug handelt.
         */
        bool isEnPassant() const;

        /**
         * @brief Überprüft ob es sich um einen Promotionzug handelt.
         */
        bool isPromotion() const;

        /**
         * @brief Überprüft ob es sich um einen Promotionzug auf Springer handelt.
         */
        bool isPromotionKnight() const;

        /**
         * @brief Überprüft ob es sich um einen Promotionzug auf Läufer handelt.
         */
        bool isPromotionBishop() const;

        /**
         * @brief Überprüft ob es sich um einen Promotionzug auf Turm handelt.
         */
        bool isPromotionRook() const;

        /**
         * @brief Überprüft ob es sich um einen Promotionzug auf Dame handelt.
         */
        bool isPromotionQueen() const;

        /**
         * @brief Vergleicht zwei Züge.
         */
        bool operator==(const Move& other) const;

        /**
         * @brief Vergleicht zwei Züge.
         */
        bool operator!=(const Move& other) const;

        /**
         * @brief Vergleicht zwei Züge.
         */
        bool operator<(const Move& other) const;

        /**
         * @brief Vergleicht zwei Züge.
         */
        bool operator>(const Move& other) const;
};

template <>
struct std::hash<Move> {
    std::size_t operator()(const Move& m) const {
        return m.getMove();
    }
};

#endif