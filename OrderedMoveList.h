#ifndef ORDEREDMOVELIST_H
#define ORDEREDMOVELIST_H

#include "Move.h"
#include <list>

/**
 * @brief Kapselt einen Zug und dessen Bewertung.
 */
class IntMovePair {
    friend class OrderedMoveList;

    private:
        int32_t value;
        Move move;
};

/**
 * @brief Kapselt eine Liste von Zügen, die in einer bestimmten Reihenfolge abgearbeitet werden sollen.
 * Die Züge werden beim Einfügen sortiert, sodass die Züge mit der höchsten Bewertung zuerst abgearbeitet werden.
 */
class OrderedMoveList {

    private:
        std::list<IntMovePair> moves;
    
    public:
        OrderedMoveList();
        OrderedMoveList(const OrderedMoveList& orderedMoveList);

        std::list<Move> getMoves() const;

        /**
         * @brief Fügt einen Zug hinzu.
         */
        void addMove(const Move& move);

        /**
         * @brief Fügt eine Liste von Zügen hinzu.
         */
        void addMoves(const std::list<Move>& moves);


        /**
         * @brief Entfernt einen Zug.
         */
        void removeMove(const Move& move);

        /**
         * @brief Entfernt eine Liste von Zügen.
         */
        void removeMoves(const std::list<Move>& moves);

        /**
         * @brief Entfernt alle Züge.
         */
        void clear();

        /**
         * @brief Prüft, ob ein Zug enthalten ist.
         */
        bool contains(const Move& move) const;

        /**
         * @brief Prüft, ob eine Liste von Zügen enthalten ist.
         */
        bool contains(const std::list<Move>& moves) const;

        /**
         * @brief Prüft, ob die Liste leer ist.
         */
        bool isEmpty() const;

        /**
         * @brief Gibt die Anzahl der enthaltenen Züge zurück.
         */
        int32_t size() const;

};

/**
 * @brief Bewertet einen Zug.
 */
int32_t scoreMove(const Move& move);

#endif