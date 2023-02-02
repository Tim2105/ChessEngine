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
        constexpr Move() : move(0) {};
        constexpr Move(uint32_t from) : move(from) {};
        constexpr Move(int32_t origin, int32_t destination, int32_t flags) : move(origin << 11 | destination << 4 | flags) {};
        constexpr ~Move() = default;

        friend std::ostream& operator<< (std::ostream& os, const Move& m);

        constexpr uint32_t getMove() const { return move; }

        /**
         * @brief Überprüft, ob der Zug Inhalt hat.
         */
        constexpr bool exists() const { return move != 0; }

        /**
         * @brief Gibt den Zug als String zurück.
         */
        std::string toString() const;

        /**
         * @brief Gibt den Ausgangspunkt des Zuges zurück.
         */
        constexpr int32_t getOrigin() const { return (move >> 11) & 0x7F; }

        /**
         * @brief Gibt das Zielfeld des Zuges zurück.
         */
        constexpr int32_t getDestination() const { return (move >> 4) & 0x7F; }

        /**
         * @brief Überprüft ob ein Zug 'leise' ist, also keine Figur geschlagen wird und es kein Spezialzug ist.
         */
        constexpr bool isQuiet() const { return (move & 0xC) == MOVE_QUIET; }

        /**
         * @brief Überprüft ob es sich um einen Doppelschritt handelt.
         */
        constexpr bool isDoublePawn() const { return (move & 0xF) == MOVE_DOUBLE_PAWN; }

        /**
         * @brief Überprüft ob es sich um einen Rochadezug handelt.
         */
        constexpr bool isCastle() const { return (move & 0xF) == MOVE_KINGSIDE_CASTLE || (move & 0xF) == MOVE_QUEENSIDE_CASTLE; }

        /**
         * @brief Überprüft ob es sich um eine Rochade auf Königsseite handelt.
         */
        constexpr bool isKingsideCastle() const { return (move & 0xF) == MOVE_KINGSIDE_CASTLE; }

        /**
         * @brief Überprüft ob es sich um eine Rochade auf Damenseite handelt.
         */
        constexpr bool isQueensideCastle() const { return (move & 0xF) == MOVE_QUEENSIDE_CASTLE; }

        /**
         * @brief Überprüft ob der Zug eine Figur schlägt.
         */
        constexpr bool isCapture() const { return (move & MOVE_CAPTURE); }

        /**
         * @brief Überprüft ob es sich um einen En Passant Zug handelt.
         */
        constexpr bool isEnPassant() const { return (move & 0xF) == MOVE_EN_PASSANT; }

        /**
         * @brief Überprüft ob es sich um einen Promotionzug handelt.
         */
        constexpr bool isPromotion() const { return (move & MOVE_PROMOTION); }

        /**
         * @brief Überprüft ob es sich um einen Promotionzug auf Springer handelt.
         */
        constexpr bool isPromotionKnight() const { return (move & 0b1011) == MOVE_PROMOTION_KNIGHT; }

        /**
         * @brief Überprüft ob es sich um einen Promotionzug auf Läufer handelt.
         */
        constexpr bool isPromotionBishop() const { return (move & 0b1011) == MOVE_PROMOTION_BISHOP; }

        /**
         * @brief Überprüft ob es sich um einen Promotionzug auf Turm handelt.
         */
        constexpr bool isPromotionRook() const { return (move & 0b1011) == MOVE_PROMOTION_ROOK; }

        /**
         * @brief Überprüft ob es sich um einen Promotionzug auf Dame handelt.
         */
        constexpr bool isPromotionQueen() const { return (move & 0b1011) == MOVE_PROMOTION_QUEEN; }

        /**
         * @brief Vergleicht zwei Züge.
         */
        constexpr bool operator==(const Move& other) const { return move == other.move; }

        /**
         * @brief Vergleicht zwei Züge.
         */
        constexpr bool operator!=(const Move& other) const { return move != other.move; }

        /**
         * @brief Vergleicht zwei Züge.
         */
        constexpr bool operator<(const Move& other) const { return move < other.move; }

        /**
         * @brief Vergleicht zwei Züge.
         */
        constexpr bool operator>(const Move& other) const { return move > other.move; }
};

template <>
struct std::hash<Move> {
    std::size_t operator()(const Move& m) const {
        return m.getMove();
    }
};

#endif