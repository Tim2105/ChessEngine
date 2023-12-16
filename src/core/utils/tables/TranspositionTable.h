#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include "core/chess/Move.h"

#include <cstring>
#include <stdint.h>
#include <functional>

struct TranspositionTableEntry {
    int32_t age;
    int16_t depth;
    int16_t score;
    uint8_t type;
    Move hashMove;
};

template<size_t bucketCount,
        bool (*replacementPredicate)(const TranspositionTableEntry&, const TranspositionTableEntry&)>
class TranspositionTable {
    private:
        struct Entry {
            uint64_t hash;
            TranspositionTableEntry entry;
            bool exists;
        };

        static constexpr auto equalityPredicate = [](const Entry& lhs, const uint64_t& rhs) {
            return lhs.hash == rhs;
        };

        Entry* predicateBucket;
        Entry* alwaysReplaceBucket;
    
    public:
        TranspositionTable();
        ~TranspositionTable();

        TranspositionTable(const TranspositionTable& other) = delete;
        TranspositionTable& operator=(const TranspositionTable& other) = delete;

        TranspositionTable(TranspositionTable&& other);
        TranspositionTable& operator=(TranspositionTable&& other);

        void put(uint64_t hash, const TranspositionTableEntry& entry);

        bool probe(uint64_t hash, TranspositionTableEntry& entry);

        void clear();
};

template<size_t bucketCount,
    bool (*replacementPredicate)(const TranspositionTableEntry&, const TranspositionTableEntry&)>
TranspositionTable<bucketCount, replacementPredicate>::TranspositionTable() {
    predicateBucket = new Entry[bucketCount];
    alwaysReplaceBucket = new Entry[bucketCount];

    clear();
}

template<size_t bucketCount,
    bool (*replacementPredicate)(const TranspositionTableEntry&, const TranspositionTableEntry&)>
TranspositionTable<bucketCount, replacementPredicate>::~TranspositionTable() {
    delete[] predicateBucket;
    delete[] alwaysReplaceBucket;
}

template<size_t bucketCount,
    bool (*replacementPredicate)(const TranspositionTableEntry&, const TranspositionTableEntry&)>
TranspositionTable<bucketCount, replacementPredicate>::TranspositionTable(TranspositionTable&& other) {
    predicateBucket = other.predicateBucket;
    other.predicateBucket = nullptr;

    alwaysReplaceBucket = other.alwaysReplaceBucket;
    other.alwaysReplaceBucket = nullptr;
}

template<size_t bucketCount,
    bool (*replacementPredicate)(const TranspositionTableEntry&, const TranspositionTableEntry&)>
TranspositionTable<bucketCount, replacementPredicate>& TranspositionTable<bucketCount, replacementPredicate>::operator=(TranspositionTable&& other) {
    delete[] predicateBucket;
    delete[] alwaysReplaceBucket;

    predicateBucket = other.predicateBucket;
    other.predicateBucket = nullptr;

    alwaysReplaceBucket = other.alwaysReplaceBucket;
    other.alwaysReplaceBucket = nullptr;

    return *this;
}

template<size_t bucketCount,
    bool (*replacementPredicate)(const TranspositionTableEntry&, const TranspositionTableEntry&)>
void TranspositionTable<bucketCount, replacementPredicate>::put(uint64_t hash, const TranspositionTableEntry& entry) {
    size_t index = hash % bucketCount;

    // Überprüfe, ob der Bucket mit Ersetzungsprädikat leer ist.
    if(!predicateBucket[index].exists)
        predicateBucket[index] = { hash, entry, true };
    else if(replacementPredicate(entry, predicateBucket[index].entry)) {
        // Wenn der Bucket mit Ersetzungsprädikat nicht leer ist,
        // bestimme, ob der Eintrag ersetzt werden soll.
        predicateBucket[index] = { hash, entry, true };
    }

    // Ersetze den Eintrag im Bucket ohne Ersetzungsprädikat.
    alwaysReplaceBucket[index] = { hash, entry, true };
}

template<size_t bucketCount,
    bool (*replacementPredicate)(const TranspositionTableEntry&, const TranspositionTableEntry&)>
bool TranspositionTable<bucketCount, replacementPredicate>::probe(uint64_t hash, TranspositionTableEntry& entry) {
    size_t index = hash % bucketCount;

    // Überprüfe den Bucket mit Ersetzungsprädikat.
    if(predicateBucket[index].exists && predicateBucket[index].hash == hash) {
        entry = predicateBucket[index].entry;
        return true;
    }

    // Überprüfe den Bucket ohne Ersetzungsprädikat.
    if(alwaysReplaceBucket[index].exists && alwaysReplaceBucket[index].hash == hash) {
        entry = alwaysReplaceBucket[index].entry;
        return true;
    }

    return false;
}

template<size_t bucketCount,
    bool (*replacementPredicate)(const TranspositionTableEntry&, const TranspositionTableEntry&)>
void TranspositionTable<bucketCount, replacementPredicate>::clear() {
    // Setze alle Einträge auf nicht existent.
    for(size_t i = 0; i < bucketCount; i++) {
        predicateBucket[i].exists = false;
        alwaysReplaceBucket[i].exists = false;
    }
}

#endif