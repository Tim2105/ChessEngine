#ifndef BITBOARD_H
#define BITBOARD_H

#include <stdint.h>
#include <iostream>
#include <functional>

class Bitboard {
    private:
        uint64_t bitboard;

    public:
        constexpr Bitboard() : bitboard(0) {};
        constexpr Bitboard(uint64_t bitboard) : bitboard(bitboard) {};
        constexpr Bitboard(const Bitboard& bitboard) : bitboard(bitboard.bitboard) {};

        friend std::ostream& operator<<(std::ostream& os, const Bitboard& bitboard);

        constexpr uint64_t getBitboard() const {
            return bitboard;
        }
        constexpr void setBitboard(uint64_t bitboard) {
            this->bitboard = bitboard;
        }

        /**
         * @brief Setze das Bit an der Stelle index.
         */
        constexpr void setBit(int32_t index) {
            bitboard |= (1ULL << index);
        }

        /**
         * @brief Lösche das Bit an der Stelle index.
         */
        constexpr void clearBit(int32_t index) {
            bitboard &= ~(1ULL << index);
        }

        /**
         * @brief Liefere den Wert des Bits an der Stelle index zurück.
         */
        constexpr bool getBit(int32_t index) const {
            return (bitboard & (1ULL << index));
        }

        /**
         * @brief Gibt den Index des ersten gesetzten Bits zurück.
         */
        constexpr int32_t getFirstSetBit() const {
            return __builtin_ctzll(bitboard);
        }

        /**
         * @brief Gibt den Index des letzten gesetzten Bits zurück.
         */
        constexpr int32_t getLastSetBit() const {
            return 63 - __builtin_clzll(bitboard);
        }

        /**
         * @brief Gibt die Anzahl der gesetzten Bits zurück.
         */
        constexpr int32_t getNumberOfSetBits() const {
            return __builtin_popcountll(bitboard);
        }

        /**
         * @brief Gibt ein Bitboard zurück, das die Bits des Bitboards in umgekehrter Reihenfolge enthält.
         */
        constexpr Bitboard reversed() const {
            uint64_t res = bitboard;
            res = ((res >> 1) & 0x5555555555555555ULL) | ((res & 0x5555555555555555ULL) << 1);
            res = ((res >> 2) & 0x3333333333333333ULL) | ((res & 0x3333333333333333ULL) << 2);
            res = ((res >> 4) & 0x0F0F0F0F0F0F0F0FULL) | ((res & 0x0F0F0F0F0F0F0F0FULL) << 4);
            res = ((res >> 8) & 0x00FF00FF00FF00FFULL) | ((res & 0x00FF00FF00FF00FFULL) << 8);
            res = ((res >> 16) & 0x0000FFFF0000FFFFULL) | ((res & 0x0000FFFF0000FFFFULL) << 16);
            res = (res >> 32) | (res << 32);
            return Bitboard(res);
        }

        /**
         * @brief Erlaubt explizite und implizite Konvertierung in bool.
         */
        constexpr operator bool() const {
            return bitboard != 0;
        }

        /**
         * @brief Erlaubt explizite und implizite Konvertierung in uint64_t.
         */
        constexpr operator uint64_t() const {
            return bitboard;
        }

        /**
         * @brief Bitweises AND.
         */
        constexpr Bitboard operator&(const Bitboard& bitboard) const {
            return Bitboard(bitboard.bitboard & this->bitboard);
        }

        /**
         * @brief Bitweises OR.
         */
        constexpr Bitboard operator|(const Bitboard& bitboard) const {
            return Bitboard(bitboard.bitboard | this->bitboard);
        }

        /**
         * @brief Bitweises OR mit Zuweisung.
         */
        constexpr Bitboard operator|= (const Bitboard& bitboard) {
            this->bitboard |= bitboard.bitboard;
            return *this;
        }

        /**
         * @brief Bitweises XOR.
         */
        constexpr Bitboard operator^(const Bitboard& bitboard) const {
            return Bitboard(bitboard.bitboard ^ this->bitboard);
        }

        /**
         * @brief Bitweises NOT.
         */
        constexpr Bitboard operator~() const {
            return Bitboard(~bitboard);
        }

        /**
         * @brief Bitweises Verschieben nach links.
         */
        constexpr Bitboard operator<<(int32_t shift) const {
            return Bitboard(bitboard << shift);
        }

        /**
         * @brief Bitweises Verschieben nach rechts.
         */
        constexpr Bitboard operator>>(int32_t shift) const {
            return Bitboard(bitboard >> shift);
        }

        /**
         * @brief Überprüft zwei Bitboards auf Gleichheit..
         */
        constexpr bool operator==(const Bitboard& bitboard) const {
            return this->bitboard == bitboard.bitboard;
        }

        /**
         * @brief Überprüft zwei Bitboards auf Ungleichheit.
         */
        constexpr bool operator!=(const Bitboard& bitboard) const {
            return this->bitboard != bitboard.bitboard;
        }

        /**
         * @brief Zuweisungsoperator.
         */
        constexpr Bitboard operator=(const Bitboard& bitboard) {
            this->bitboard = bitboard.bitboard;
            return *this;
        }
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

extern Bitboard pawnAttacks[][64];
extern Bitboard knightAttacks[];
extern Bitboard kingAttacks[];

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