#ifndef BITBOARD_H
#define BITBOARD_H

#include <stdint.h>
#include <iostream>

class Bitboard {
    private:
        uint64_t bitboard;

    public:
        Bitboard();
        Bitboard(uint64_t bitboard);
        Bitboard(const Bitboard& bitboard);

        friend std::ostream& operator<<(std::ostream& os, const Bitboard& bitboard);

        uint64_t getBitboard() const;
        void setBitboard(uint64_t bitboard);

        /**
         * @brief Setze das Bit an der Stelle index.
         */
        void setBit(int32_t index);

        /**
         * @brief Lösche das Bit an der Stelle index.
         */
        void clearBit(int32_t index);

        /**
         * @brief Liefere den Wert des Bits an der Stelle index zurück.
         */
        bool getBit(int32_t index) const;

        /**
         * @brief Gibt den Index des ersten gesetzten Bits zurück.
         */
        int32_t getFirstSetBit() const;

        /**
         * @brief Gibt den Index des letzten gesetzten Bits zurück.
         */
        int32_t getLastSetBit() const;

        /**
         * @brief Gibt die Anzahl der gesetzten Bits zurück.
         */
        int32_t getNumberOfSetBits() const;

        /**
         * @brief Erlaubt explizite und implizite Konvertierung in bool.
         */
        operator bool() const;

        /**
         * @brief Erlaubt explizite und implizite Konvertierung in uint64_t.
         */
        operator uint64_t() const;

        /**
         * @brief Bitweises AND.
         */
        Bitboard operator&(const Bitboard& bitboard) const;

        /**
         * @brief Bitweises OR.
         */
        Bitboard operator|(const Bitboard& bitboard) const;

        /**
         * @brief Bitweises OR mit Zuweisung.
         */
        Bitboard operator|= (const Bitboard& bitboard);

        /**
         * @brief Bitweises XOR.
         */
        Bitboard operator^(const Bitboard& bitboard) const;

        /**
         * @brief Bitweises NOT.
         */
        Bitboard operator~() const;

        /**
         * @brief Bitweises Verschieben nach links.
         */
        Bitboard operator<<(int32_t shift) const;

        /**
         * @brief Bitweises Verschieben nach rechts.
         */
        Bitboard operator>>(int32_t shift) const;

        /**
         * @brief Überprüft zwei Bitboards auf Gleichheit..
         */
        bool operator==(const Bitboard& bitboard) const;

        /**
         * @brief Überprüft zwei Bitboards auf Ungleichheit.
         */
        bool operator!=(const Bitboard& bitboard) const;

        /**
         * @brief Zuweisungsoperator.
         */
        Bitboard operator=(const Bitboard& bitboard);
};

/**
 * @brief Liefert ein Bitboard, dass, ausgehend von einem Feld, alle Felder enthält die von einer diagonal laufenden Figur angegriffen werden.
 * 
 * @param sq Das Feld in 64er Notation.
 * @param occupied Das Bitboard mit den besetzten Feldern.
 */
Bitboard diagonalAttackBitboard(int32_t sq, const Bitboard& occupied);

/**
 * @brief Liefert ein Bitboard, dass, ausgehend von einem Feld, alle Felder enthält die von einer horizontal/vertikal laufenden Figur angegriffen werden.
 * 
 * @param sq Das Feld in 64er Notation.
 * @param occupied Das Bitboard mit den besetzten Feldern.
 */
Bitboard straightAttackBitboard(int32_t sq, const Bitboard& occupied);

/**
 * @brief Liefert ein Bitboard, dass, ausgehend von einem Feld, alle Felder enthält die von einer Springer angegriffen werden.
 * 
 * @param sq Das Feld in 64er Notation.
 */
Bitboard knightAttackBitboard(int32_t sq);

/**
 * @brief Liefert ein Bitboard, dass, ausgehend von einem Feld, alle Felder enthält die von einem Bauern angegriffen werden.
 * 
 * @param sq Das Feld in 64er Notation.
 * @param side Die Farbe des Bauern.
 */
Bitboard pawnAttackBitboard(int32_t sq, int32_t side);

/**
 * @brief Liefert ein Bitboard, dass, ausgehend von einem Feld, alle Felder enthält die von einem König angegriffen werden.
 * 
 * @param sq Das Feld in 64er Notation.
 */
Bitboard kingAttackBitboard(int32_t sq);

#endif