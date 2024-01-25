#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include "core/chess/Move.h"

#include <atomic>
#include <stdint.h>

/**
 * @brief 8-Byte Datenkomponente eines Eintrags in der Transpositionstabelle.
 */
struct TranspositionTableEntry {
    Move hashMove; // 2 Byte
    int16_t score; // 2 Byte
    uint16_t age; // 2 Byte
    uint8_t depth; // 1 Byte
    uint8_t type; // 1 Byte

    constexpr TranspositionTableEntry() {}

    /**
     * @brief Komponentenweise Initialisierung.
     */
    constexpr TranspositionTableEntry(Move hashMove, int16_t score, uint16_t age,
                                      uint8_t depth, uint8_t type) :
        hashMove(hashMove),
        score(score),
        age(age),
        depth(depth),
        type(type) {}

    /**
     * @brief Bitweise Initialisierung.
     */
    constexpr TranspositionTableEntry(uint64_t bits) :
        hashMove(bits & 0xFFFF),
        score((bits >> 16) & 0xFFFF),
        age((bits >> 32) & 0xFFFF),
        depth((bits >> 48) & 0xFF),
        type((bits >> 56) & 0xFF) {}

    /**
     * @brief Implizite Konvertierung in u64.
     */
    constexpr operator uint64_t() const {
        return *reinterpret_cast<const uint64_t*>(this);
    }

    /**
     * @brief Ersetzungsprädikat für die Transpositionstabelle.
     * 
     * @param other Der andere Eintrag.
     * 
     * @return true, wenn dieser Eintrag den anderen Eintrag ersetzen soll.
     */
    constexpr bool operator>(const TranspositionTableEntry& other) const {
        return depth + age >= other.depth + other.age;
    }
};

/**
 * @brief 16-Byte Eintrag in der Transpositionstabelle.
 * Besteht aus einem 8-Byte Hash und einem 8-Byte Datenfeld.
 */
struct Entry {
    uint64_t hash;
    TranspositionTableEntry data;
};

static constexpr size_t TT_ENTRY_SIZE = sizeof(Entry);
static constexpr size_t TT_DEFAULT_CAPACITY = 1 << 22;

/**
 * @brief Eine, nicht synchronisierte aber thread-sichere (put und probe),
 * flüchtige Hash-Tabelle für die Speicherung von Knoteninformationen
 * in einer Hauptvariantensuche.
 * 
 * Die Tabelle ist nicht synchronisiert. Dass bedeutet, es besteht die Möglichkeit,
 * dass zwei Threads gleichzeitig in denselben Eintrag schreiben.
 * Ungültige Einträge werden aber (in der Regel) erkannt und nicht zurückgegeben.
 */
class TranspositionTable {
    private:
        Entry* entries;
        size_t capacity;
        std::atomic<size_t> entriesWritten;
    
    public:
        /**
         * @brief Konstruktor für Transpositionstabellen mit gegebener Kapazität.
         * 
         * @param capacity Die Kapazität der Tabelle.
         * 
         * @throws Wenn nicht genügend Speicher verfügbar ist.
         */
        TranspositionTable(size_t capacity = TT_DEFAULT_CAPACITY);
        ~TranspositionTable();

        TranspositionTable(const TranspositionTable& other) = delete;
        TranspositionTable& operator=(const TranspositionTable& other) = delete;

        TranspositionTable(TranspositionTable&& other);
        TranspositionTable& operator=(TranspositionTable&& other);

        constexpr size_t getCapacity() const noexcept {
            return capacity;
        }

        inline size_t getEntriesWritten() const noexcept {
            return entriesWritten.load();
        }

        /**
         * @brief Speichert einen Eintrag in der Transpositionstabelle,
         * wenn der zugehörige Bucket leer ist oder der Eintrag nach dem
         * Ersetzungsprädikat eine höhere Priorität als der bisherige Eintrag hat.
         * 
         * @param hash Der Hashwert des Knotens.
         * @param entry Der Eintrag.
         */
        void put(uint64_t hash, const TranspositionTableEntry& entry) noexcept;

        /**
         * @brief Sucht einen Eintrag in der Transpositionstabelle
         * und schreibt ihn in das übergebene Entry-Objekt.
         * 
         * @param hash Der Hashwert des Knotens.
         * @param entry Das Entry-Objekt, in das der Eintrag geschrieben werden soll.
         * 
         * @return true, wenn ein Eintrag gefunden wurde.
         */
        bool probe(uint64_t hash, TranspositionTableEntry& entry) const noexcept;

        /**
         * @brief Entfernt alle Einträge aus der Transpositionstabelle.
         * 
         * @throws Wenn nicht genügend Speicher verfügbar ist.
         * 
         * @note Diese Methode ist nicht thread-sicher und sollte nie während
         * einer laufenden Suche aufgerufen werden.
         */
        void clear() noexcept;

        /**
         * @brief Ändert die Kapazität der Transpositionstabelle.
         * 
         * @note Diese Methode ist nicht thread-sicher. Eine Verwendung
         * während einer laufenden Suche wird mit Sicherheit zu einem
         * Absturz führen, weil diese Methode den Speicherblock der
         * Transpositionstabelle austauscht.
         */
        void resize(size_t capacity);
};

#endif