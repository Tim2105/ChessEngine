#ifndef BITBOARD_H
#define BITBOARD_H

#include <stdint.h>
#include <iostream>
#include <functional>

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
         * @brief Gibt ein Bitboard zurück, das die Bits des Bitboards in umgekehrter Reihenfolge enthält.
         */
        Bitboard reversed() const;

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
 * @brief Hashfunktion für Bitboards.
 */
template <>
struct std::hash<Bitboard> {
    size_t operator()(const Bitboard& bitboard) const {
        uint64_t res = bitboard;

        res ^= res >> 33;
        res *= 0xff51afd7ed558ccdULL;
        res ^= res >> 33;
        res *= 0xc4ceb9fe1a85ec53ULL;
        res ^= res >> 33;

        return res;
    }
};

/**
 * @brief Liefert ein Bitboard, dass, ausgehend von einem Feld, alle Felder enthält die von einer diagonal laufenden Figur angegriffen werden.
 * 
 * @param sq Das Feld in 64er Notation.
 * @param occupied Das Bitboard mit den besetzten Feldern.
 */
Bitboard diagonalAttackBitboard(int32_t sq, const Bitboard occupied);

/**
 * @brief Liefert ein Bitboard, dass, ausgehend von einem Feld, alle Felder enthält die von einer horizontal/vertikal laufenden Figur angegriffen werden.
 * 
 * @param sq Das Feld in 64er Notation.
 * @param occupied Das Bitboard mit den besetzten Feldern.
 */
Bitboard straightAttackBitboard(int32_t sq, const Bitboard occupied);

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

/**
 * @brief Gibt ein Bitboard aller Diagonalen zurück, die von einem Feld aus eine Liste von Zielen angreifen.
 * Diagonale, die aus dem Spielfeld herausgehen, werden ignoriert.
 * 
 * @param sq Das Feld in 64er Notation.
 */
Bitboard diagonalAttackUntilBlocked(int32_t sq, const Bitboard target, const Bitboard occupied);

/**
 * @brief Gibt ein Bitboard aller Geraden zurück, die von einem Feld aus eine Liste von Zielen angreifen.
 * Geraden, die aus dem Spielfeld herausgehen, werden ignoriert.
 * 
 * @param sq Das Feld in 64er Notation.
 */
Bitboard straightAttackUntilBlocked(int32_t sq, const Bitboard target, const Bitboard occupied);

/**
 * @brief Überprüft, ob auf einem Spielfeld eine/mehrere Figuren an ein Feld diagonal gefesselt ist.
 * 
 * @param sq Das Feld in 64er Notation.
 * @param ownPieces Das Bitboard mit den eigenen Figuren(die möglicherweise gefesselt sein könnten).
 * @param enemyPieces Das Bitboard mit den gegnerischen Figuren(die möglicherweise die Fesselung verursachen).
 * @param occupied Das Bitboard mit allen Figuren, die weder eigene noch fesselnde Figuren sind.
 * @param pinnedSquare Die Felder, auf denen die gefesselten Figuren stehen(muss mind. Größe 4 haben).
 * @param pinnedDirection Die Richtungen, aus denen die Figuren gefesselt sind(muss mind. Größe 4 haben).
 */
int32_t getDiagonallyPinnedToSquare(int32_t sq, Bitboard ownPieces, Bitboard enemyPieces, Bitboard occupied, int32_t* pinnedSquare, int32_t* pinnedDirection);

/**
 * @brief Überprüft, ob auf einem Spielfeld eine/mehrere Figuren an ein Feld geradlinig gefesselt ist.
 * 
 * @param sq Das Feld in 64er Notation.
 * @param ownPieces Das Bitboard mit den eigenen Figuren(die möglicherweise gefesselt sein könnten).
 * @param enemyPieces Das Bitboard mit den gegnerischen Figuren(die möglicherweise die Fesselung verursachen).
 * @param occupied Das Bitboard mit allen Figuren, die weder eigene noch fesselnde Figuren sind.
 * @param pinnedSquare Die Felder, auf denen die gefesselten Figuren stehen(muss mind. Größe 4 haben).
 * @param pinnedDirection Die Richtungen, aus denen die Figuren gefesselt sind(muss mind. Größe 4 haben).
 */
int32_t getStraightPinnedToSquare(int32_t sq, Bitboard ownPieces, Bitboard enemyPieces, Bitboard occupied, int32_t* pinnedSquare, int32_t* pinnedDirection);

#endif