#ifndef BITBOARD_H
#define BITBOARD_H

#include <bit>
#include <iostream>
#include <stdint.h>

#include "core/chess/BoardDefinitions.h"
#include "core/utils/magics/Magics.h"

class Bitboard {
    private:
        uint64_t bits;

    public:
        static constexpr uint64_t ZEROS = 0ULL;
        static constexpr uint64_t ONES = 0xFFFFFFFFFFFFFFFFULL;

        constexpr Bitboard() noexcept : bits(0) {};
        constexpr Bitboard(uint64_t bitboard) noexcept : bits(bitboard) {};
        constexpr Bitboard(const Bitboard& bitboard) noexcept = default;

        constexpr ~Bitboard() noexcept = default;

        friend std::ostream& operator<<(std::ostream& os, const Bitboard& bitboard);

        constexpr uint64_t toU64() const {
            return bits;
        }

        constexpr void fromU64(uint64_t bitboard) {
            this->bits = bitboard;
        }

        /**
         * @brief Setze das Bit an der Stelle index.
         */
        constexpr void setBit(int32_t index) {
            bits |= (1ULL << index);
        }

        /**
         * @brief Lösche das Bit an der Stelle index.
         */
        constexpr void clearBit(int32_t index) {
            bits &= ~(1ULL << index);
        }

        /**
         * @brief Liefere den Wert des Bits an der Stelle index zurück.
         */
        constexpr bool getBit(int32_t index) const {
            return (bits & (1ULL << index));
        }

        /**
         * @brief Gibt den Index des ersten gesetzten Bits zurück.
         */
        constexpr int32_t getFSB() const {
            return std::countr_zero(bits);
        }

        /**
         * @brief Gibt den Index des letzten gesetzten Bits zurück.
         */
        constexpr int32_t getLSB() const {
            return 63 - std::countl_zero(bits);
        }

        /**
         * @brief Löscht das erste gesetzte Bit und gibt dessen Index zurück.
         */
        constexpr int32_t popFSB() {
            int32_t index = getFSB();
            bits &= bits - 1;
            return index;
        }

        /**
         * @brief Löscht das letzte gesetzte Bit und gibt dessen Index zurück.
         */
        constexpr int32_t popLSB() {
            int32_t index = getLSB();
            clearBit(index);
            return index;
        }

        /**
         * @brief Führt ein Population Count auf dem Bitboard durch,
         * d.h. gibt die Anzahl der gesetzten Bits zurück.
         */
        constexpr int32_t popcount() const {
            return std::popcount(bits);
        }

        /**
         * @brief Gibt ein Bitboard zurück, das die Bits des Bitboards in umgekehrter Reihenfolge enthält.
         */
        constexpr Bitboard reversed() const {
            uint64_t res = bits;
            res = ((res >> 1) & 0x5555555555555555ULL) | ((res & 0x5555555555555555ULL) << 1);
            res = ((res >> 2) & 0x3333333333333333ULL) | ((res & 0x3333333333333333ULL) << 2);
            res = ((res >> 4) & 0x0F0F0F0F0F0F0F0FULL) | ((res & 0x0F0F0F0F0F0F0F0FULL) << 4);
            res = ((res >> 8) & 0x00FF00FF00FF00FFULL) | ((res & 0x00FF00FF00FF00FFULL) << 8);
            res = ((res >> 16) & 0x0000FFFF0000FFFFULL) | ((res & 0x0000FFFF0000FFFFULL) << 16);
            res = (res >> 32) | (res << 32);
            return Bitboard(res);
        }

        /**
         * @brief Führt ein Byteswap auf dem Bitboard durch.
         * Ein Byteswap spiegelt die Bits an der horizontalen Achse.
         */
        constexpr Bitboard bswap() const {
            // TODO: Ersetze die intrinsische Funktion durch std::byteswap aus C++23.
            return Bitboard(__builtin_bswap64(bits));
        }

        /**
         * @brief Verschiebt alle gesetzten Bits in Richtung Norden.
         */
        constexpr Bitboard shiftNorth(int32_t shift) const {
            return Bitboard(bits << (8 * shift));
        }

        /**
         * @brief Verschiebt alle gesetzten Bits in Richtung Süden.
         */
        constexpr Bitboard shiftSouth(int32_t shift) const {
            return Bitboard(bits >> (8 * shift));
        }

        /**
         * @brief Verschiebt alle gesetzten Bits eine Position nach Norden.
         */
        constexpr Bitboard shiftNorth() const {
            return Bitboard(bits << 8);
        }

        /**
         * @brief Verschiebt alle gesetzten Bits eine Position nach Süden.
         */
        constexpr Bitboard shiftSouth() const {
            return Bitboard(bits >> 8);
        }

        /**
         * @brief Verschiebt alle gesetzten Bits eine Position nach Osten.
         */
        constexpr Bitboard shiftEast() const {
            return Bitboard((bits << 1) & 0xFEFEFEFEFEFEFEFEULL);
        }

        /**
         * @brief Verschiebt alle gesetzten Bits eine Position nach Westen.
         */
        constexpr Bitboard shiftWest() const {
            return Bitboard((bits >> 1) & 0x7F7F7F7F7F7F7F7FULL);
        }

        /**
         * @brief Verschiebt alle gesetzten Bits eine Position diagonal nach Nordosten.
         */
        constexpr Bitboard shiftNorthEast() const {
            return Bitboard((bits << 9) & 0xFEFEFEFEFEFEFEFEULL);
        }

        /**
         * @brief Verschiebt alle gesetzten Bits eine Position diagonal nach Nordwesten.
         */
        constexpr Bitboard shiftNorthWest() const {
            return Bitboard((bits << 7) & 0x7F7F7F7F7F7F7F7FULL);
        }

        /**
         * @brief Verschiebt alle gesetzten Bits eine Position diagonal nach Südosten.
         */
        constexpr Bitboard shiftSouthEast() const {
            return Bitboard((bits >> 7) & 0xFEFEFEFEFEFEFEFEULL);
        }

        /**
         * @brief Verschiebt alle gesetzten Bits eine Position diagonal nach Südwesten.
         */
        constexpr Bitboard shiftSouthWest() const {
            return Bitboard((bits >> 9) & 0x7F7F7F7F7F7F7F7FULL);
        }

        /**
         * @brief Verschiebt alle gesetzten Bits eine Position diagonal nach Nordwesten und Nordosten.
         */
        constexpr Bitboard shiftNorthWestEast() const {
            return shiftNorthWest() | shiftNorthEast();
        }

        /**
         * @brief Verschiebt alle gesetzten Bits eine Position diagonal nach Südwesten und Südosten.
         */
        constexpr Bitboard shiftSouthWestEast() const {
            return shiftSouthWest() | shiftSouthEast();
        }

        /**
         * @brief Setzt alle ungesetzten Bits nördlich der gesetzten Bits.
         */
        constexpr Bitboard extrudeNorth() const {
            uint64_t temp = bits;
            temp |= temp << 8;
            temp |= temp << 16;
            temp |= temp << 32;
            return Bitboard(temp);
        }

        /**
         * @brief Setzt alle ungesetzten Bits südlich der gesetzten Bits.
         */
        constexpr Bitboard extrudeSouth() const {
            uint64_t temp = bits;
            temp |= temp >> 8;
            temp |= temp >> 16;
            temp |= temp >> 32;
            return Bitboard(temp);
        }

        /**
         * @brief Setzt alle ungesetzten Bits östlich der gesetzten Bits.
         */
        constexpr Bitboard extrudeEast() const {
            uint64_t step1 = bits | (bits << 1);
            step1 &= 0xFEFEFEFEFEFEFEFEULL | bits;
            uint64_t step2 = step1 | (step1 << 2);
            step2 &= 0xFCFCFCFCFCFCFCFCULL | step1;
            uint64_t step3 = step2 | (step2 << 4);
            step3 &= 0xF0F0F0F0F0F0F0F0ULL | step2;
            return Bitboard(step3);
        }

        /**
         * @brief Setzt alle ungesetzten Bits westlich der gesetzten Bits.
         */
        constexpr Bitboard extrudeWest() const {
            uint64_t step1 = bits | (bits >> 1);
            step1 &= 0x7F7F7F7F7F7F7F7FULL | bits;
            uint64_t step2 = step1 | (step1 >> 2);
            step2 &= 0x3F3F3F3F3F3F3F3FULL | step1;
            uint64_t step3 = step2 | (step2 >> 4);
            step3 &= 0x0F0F0F0F0F0F0F0FULL | step2;
            return Bitboard(step3);
        }

        /**
         * @brief Setzt alle ungesetzten Bits auf derselben Linie (vertikal)
         * wie die gesetzten Bits.
         */
        constexpr Bitboard extrudeVertically() const {
            uint64_t temp = bits;
            temp |= temp << 8;
            temp |= temp >> 8;
            temp |= temp << 16;
            temp |= temp >> 16;
            temp |= temp << 32;
            temp |= temp >> 32;
            return Bitboard(temp);
        }

        /**
         * @brief Setzt alle ungesetzten Bits auf derselben Reihe (horizontal)
         * wie die gesetzten Bits.
         */
        constexpr Bitboard extrudeHorizontally() const {
            uint64_t step1 = bits | (bits << 1);
            step1 &= 0xFEFEFEFEFEFEFEFEULL | bits;
            uint64_t step2 = step1 | (step1 >> 1);
            step2 &= 0x7F7F7F7F7F7F7F7FULL | step1;
            uint64_t step3 = step2 | (step2 << 2);
            step3 &= 0xFCFCFCFCFCFCFCFCULL | step2;
            uint64_t step4 = step3 | (step3 >> 2);
            step4 &= 0x3F3F3F3F3F3F3F3FULL | step3;
            uint64_t step5 = step4 | (step4 << 4);
            step5 &= 0xF0F0F0F0F0F0F0F0ULL | step4;
            uint64_t step6 = step5 | (step5 >> 4);
            step6 &= 0x0F0F0F0F0F0F0F0FULL | step5;
            return Bitboard(step6);
        }

        /**
         * @brief Erlaubt explizite und implizite Konvertierung in bool.
         */
        constexpr operator bool() const {
            return bits != 0;
        }

        /**
         * @brief Erlaubt explizite und implizite Konvertierung in uint64_t.
         */
        constexpr operator uint64_t() const {
            return bits;
        }

        /**
         * @brief Bitweises AND.
         */
        constexpr Bitboard operator&(const Bitboard& bitboard) const {
            return Bitboard(bitboard.bits & this->bits);
        }

        /**
         * @brief Bitweises AND mit Zuweisung.
         */
        constexpr Bitboard operator&=(const Bitboard& bitboard) {
            this->bits &= bitboard.bits;
            return *this;
        }

        /**
         * @brief Bitweises OR.
         */
        constexpr Bitboard operator|(const Bitboard& bitboard) const {
            return Bitboard(bitboard.bits | this->bits);
        }

        /**
         * @brief Bitweises OR mit Zuweisung.
         */
        constexpr Bitboard operator|= (const Bitboard& bitboard) {
            this->bits |= bitboard.bits;
            return *this;
        }

        /**
         * @brief Bitweises XOR.
         */
        constexpr Bitboard operator^(const Bitboard& bitboard) const {
            return Bitboard(bitboard.bits ^ this->bits);
        }

        /**
         * @brief Bitweises NOT.
         */
        constexpr Bitboard operator~() const {
            return Bitboard(~bits);
        }

        /**
         * @brief Bitweises Verschieben nach links.
         */
        constexpr Bitboard operator<<(int32_t shift) const {
            return Bitboard(bits << shift);
        }

        /**
         * @brief Bitweises Verschieben nach rechts.
         */
        constexpr Bitboard operator>>(int32_t shift) const {
            return Bitboard(bits >> shift);
        }

        /**
         * @brief Überprüft zwei Bitboards auf Gleichheit.
         */
        constexpr bool operator==(const Bitboard& bitboard) const {
            return this->bits == bitboard.bits;
        }

        /**
         * @brief Überprüft zwei Bitboards auf Ungleichheit.
         */
        constexpr bool operator!=(const Bitboard& bitboard) const {
            return this->bits != bitboard.bits;
        }

        /**
         * @brief Zuweisungsoperator.
         */
        constexpr Bitboard& operator=(const Bitboard& bitboard) = default;
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

constexpr Bitboard pawnAttacks[2][64] {
    // White
    {
        0x200ULL,0x500ULL,0xA00ULL,0x1400ULL,0x2800ULL,0x5000ULL,0xA000ULL,0x4000ULL,
        0x20000ULL,0x50000ULL,0xA0000ULL,0x140000ULL,0x280000ULL,0x500000ULL,0xA00000ULL,0x400000ULL,
        0x2000000ULL,0x5000000ULL,0xA000000ULL,0x14000000ULL,0x28000000ULL,0x50000000ULL,0xA0000000ULL,0x40000000ULL,
        0x200000000ULL,0x500000000ULL,0xA00000000ULL,0x1400000000ULL,0x2800000000ULL,0x5000000000ULL,0xA000000000ULL,0x4000000000ULL,
        0x20000000000ULL,0x50000000000ULL,0xA0000000000ULL,0x140000000000ULL,0x280000000000ULL,0x500000000000ULL,0xA00000000000ULL,0x400000000000ULL,
        0x2000000000000ULL,0x5000000000000ULL,0xA000000000000ULL,0x14000000000000ULL,0x28000000000000ULL,0x50000000000000ULL,0xA0000000000000ULL,0x40000000000000ULL,
        0x200000000000000ULL,0x500000000000000ULL,0xA00000000000000ULL,0x1400000000000000ULL,0x2800000000000000ULL,0x5000000000000000ULL,0xA000000000000000ULL,0x4000000000000000ULL,
        0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL
    },
    // Black
    {
        0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,0x0ULL,
        0x2ULL,0x5ULL,0xAULL,0x14ULL,0x28ULL,0x50ULL,0xA0ULL,0x40ULL,
        0x200ULL,0x500ULL,0xA00ULL,0x1400ULL,0x2800ULL,0x5000ULL,0xA000ULL,0x4000ULL,
        0x20000ULL,0x50000ULL,0xA0000ULL,0x140000ULL,0x280000ULL,0x500000ULL,0xA00000ULL,0x400000ULL,
        0x2000000ULL,0x5000000ULL,0xA000000ULL,0x14000000ULL,0x28000000ULL,0x50000000ULL,0xA0000000ULL,0x40000000ULL,
        0x200000000ULL,0x500000000ULL,0xA00000000ULL,0x1400000000ULL,0x2800000000ULL,0x5000000000ULL,0xA000000000ULL,0x4000000000ULL,
        0x20000000000ULL,0x50000000000ULL,0xA0000000000ULL,0x140000000000ULL,0x280000000000ULL,0x500000000000ULL,0xA00000000000ULL,0x400000000000ULL,
        0x2000000000000ULL,0x5000000000000ULL,0xA000000000000ULL,0x14000000000000ULL,0x28000000000000ULL,0x50000000000000ULL,0xA0000000000000ULL,0x40000000000000ULL
    }
};

constexpr Bitboard knightAttacks[64] = {
    0x20400ULL,0x50800ULL,0xa1100ULL,0x142200ULL,0x284400ULL,0x508800ULL,0xa01000ULL,0x402000ULL,
    0x2040004ULL,0x5080008ULL,0xa110011ULL,0x14220022ULL,0x28440044ULL,0x50880088ULL,0xa0100010ULL,0x40200020ULL,
    0x204000402ULL,0x508000805ULL,0xa1100110aULL,0x1422002214ULL,0x2844004428ULL,0x5088008850ULL,0xa0100010a0ULL,0x4020002040ULL,
    0x20400040200ULL,0x50800080500ULL,0xa1100110a00ULL,0x142200221400ULL,0x284400442800ULL,0x508800885000ULL,0xa0100010a000ULL,0x402000204000ULL,
    0x2040004020000ULL,0x5080008050000ULL,0xa1100110a0000ULL,0x14220022140000ULL,0x28440044280000ULL,0x50880088500000ULL,0xa0100010a00000ULL,0x40200020400000ULL,
    0x204000402000000ULL,0x508000805000000ULL,0xa1100110a000000ULL,0x1422002214000000ULL,0x2844004428000000ULL,0x5088008850000000ULL,0xa0100010a0000000ULL,0x4020002040000000ULL,
    0x400040200000000ULL,0x800080500000000ULL,0x1100110a00000000ULL,0x2200221400000000ULL,0x4400442800000000ULL,0x8800885000000000ULL,0x100010a000000000ULL,0x2000204000000000ULL,
    0x4020000000000ULL,0x8050000000000ULL,0x110a0000000000ULL,0x22140000000000ULL,0x44280000000000ULL,0x88500000000000ULL,0x10a00000000000ULL,0x20400000000000ULL
};

constexpr Bitboard kingAttacks[64] = {
    0x302ULL,0x705ULL,0xE0AULL,0x1C14ULL,0x3828ULL,0x7050ULL,0xE0A0ULL,0xC040ULL,
    0x30203ULL,0x70507ULL,0xE0A0EULL,0x1C141CULL,0x382838ULL,0x705070ULL,0xE0A0E0ULL,0xC040C0ULL,
    0x3020300ULL,0x7050700ULL,0xE0A0E00ULL,0x1C141C00ULL,0x38283800ULL,0x70507000ULL,0xE0A0E000ULL,0xC040C000ULL,
    0x302030000ULL,0x705070000ULL,0xE0A0E0000ULL,0x1C141C0000ULL,0x3828380000ULL,0x7050700000ULL,0xE0A0E00000ULL,0xC040C00000ULL,
    0x30203000000ULL,0x70507000000ULL,0xE0A0E000000ULL,0x1C141C000000ULL,0x382838000000ULL,0x705070000000ULL,0xE0A0E0000000ULL,0xC040C0000000ULL,
    0x3020300000000ULL,0x7050700000000ULL,0xE0A0E00000000ULL,0x1C141C00000000ULL,0x38283800000000ULL,0x70507000000000ULL,0xE0A0E000000000ULL,0xC040C000000000ULL,
    0x302030000000000ULL,0x705070000000000ULL,0xE0A0E0000000000ULL,0x1C141C0000000000ULL,0x3828380000000000ULL,0x7050700000000000ULL,0xE0A0E00000000000ULL,0xC040C00000000000ULL,
    0x203000000000000ULL,0x507000000000000ULL,0xA0E000000000000ULL,0x141C000000000000ULL,0x2838000000000000ULL,0x5070000000000000ULL,0xA0E0000000000000ULL,0x40C0000000000000ULL
};

/**
 * @brief Liefert ein Bitboard, dass, ausgehend von einem Feld, alle Felder enthält die von einer diagonal laufenden Figur angegriffen werden.
 * 
 * @param sq Das Feld.
 * @param occupied Das Bitboard mit den besetzten Feldern.
 */
inline Bitboard diagonalAttackBitboard(int32_t sq, const Bitboard occupied) {
    return Magics::lookupBishopAttacks(sq, occupied);
}

/**
 * @brief Liefert ein Bitboard, dass, ausgehend von einem Feld, alle Felder enthält die von einer horizontal/vertikal laufenden Figur angegriffen werden.
 * 
 * @param sq Das Feld.
 * @param occupied Das Bitboard mit den besetzten Feldern.
 */
inline Bitboard horizontalAttackBitboard(int32_t sq, const Bitboard occupied) {
    return Magics::lookupRookAttacks(sq, occupied);
}

/**
 * @brief Liefert ein Bitboard, dass, ausgehend von einem Feld, alle Felder enthält die von einer Springer angegriffen werden.
 * 
 * @param sq Das Feld.
 */
inline Bitboard knightAttackBitboard(int32_t sq) {
    return knightAttacks[sq];
}

/**
 * @brief Liefert ein Bitboard, dass, ausgehend von einem Feld, alle Felder enthält die von einem Bauern angegriffen werden.
 * 
 * @param sq Das Feld.
 * @param side Die Farbe des Bauern.
 */
inline Bitboard pawnAttackBitboard(int32_t sq, int32_t side) {
    return pawnAttacks[side / COLOR_MASK][sq];
}

/**
 * @brief Liefert ein Bitboard, dass, ausgehend von einem Feld, alle Felder enthält die von einem König angegriffen werden.
 * 
 * @param sq Das Feld.
 */
inline Bitboard kingAttackBitboard(int32_t sq) {
    return kingAttacks[sq];
}

/**
 * @brief Gibt ein Bitboard aller Diagonalen zurück, die von einem Feld aus eine Liste von Zielen angreifen.
 * Diagonale, die aus dem Spielfeld herausgehen, werden ignoriert.
 * 
 * @param sq Das Feld.
 */
Bitboard diagonalAttackUntilBlocked(int32_t sq, Bitboard target, Bitboard occupied);

/**
 * @brief Gibt ein Bitboard aller Geraden zurück, die von einem Feld aus eine Liste von Zielen angreifen.
 * Geraden, die aus dem Spielfeld herausgehen, werden ignoriert.
 * 
 * @param sq Das Feld.
 */
Bitboard horizontalAttackUntilBlocked(int32_t sq, Bitboard target, Bitboard occupied);

/**
 * @brief Überprüft, ob auf einem Spielfeld eine/mehrere Figuren an ein Feld diagonal gefesselt ist.
 * 
 * @param sq Das Feld.
 * @param ownPieces Das Bitboard mit den eigenen Figuren(die möglicherweise gefesselt sein könnten).
 * @param enemyPieces Das Bitboard mit den gegnerischen Figuren(die möglicherweise die Fesselung verursachen).
 * @param occupied Das Bitboard mit allen Figuren, die weder eigene noch fesselnde Figuren sind.
 * @param pinnedSquare Die Felder, auf denen die gefesselten Figuren stehen(muss mind. Größe 4 haben).
 * @param pinnedDirection Die Richtungen, aus denen die Figuren gefesselt sind(muss mind. Größe 4 haben).
 * 
 * @return Die Anzahl der gefesselten Figuren.
 */
int32_t getDiagonallyPinnedToSquare(int32_t sq, Bitboard ownPieces, Bitboard enemyPieces, Bitboard occupied, int32_t* pinnedSquare, int32_t* pinnedDirection);

/**
 * @brief Überprüft, ob auf einem Spielfeld eine/mehrere Figuren an ein Feld gradlinig gefesselt ist.
 * 
 * @param sq Das Feld.
 * @param ownPieces Das Bitboard mit den eigenen Figuren(die möglicherweise gefesselt sein könnten).
 * @param enemyPieces Das Bitboard mit den gegnerischen Figuren(die möglicherweise die Fesselung verursachen).
 * @param occupied Das Bitboard mit allen Figuren, die weder eigene noch fesselnde Figuren sind.
 * @param pinnedSquare Die Felder, auf denen die gefesselten Figuren stehen(muss mind. Größe 4 haben).
 * @param pinnedDirection Die Richtungen, aus denen die Figuren gefesselt sind(muss mind. Größe 4 haben).
 * 
 * @return Die Anzahl der gefesselten Figuren.
 */
int32_t getHorizontallyPinnedToSquare(int32_t sq, Bitboard ownPieces, Bitboard enemyPieces, Bitboard occupied, int32_t* pinnedSquare, int32_t* pinnedDirection);

#endif