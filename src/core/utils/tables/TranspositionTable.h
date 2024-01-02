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

struct Entry {
    uint64_t hash;
    TranspositionTableEntry entry;
    bool exists;
};

static constexpr size_t TT_ENTRY_SIZE = sizeof(Entry);

template<bool (*replacementPredicate)(const TranspositionTableEntry&, const TranspositionTableEntry&)>
class TranspositionTable {
    private:
        static constexpr auto equalityPredicate = [](const Entry& lhs, const uint64_t& rhs) {
            return lhs.hash == rhs;
        };

        Entry* predicateBucket;
        Entry* alwaysReplaceBucket;
        size_t bucketCount;
    
    public:
        TranspositionTable(size_t bucketCount = 1 << 19);
        ~TranspositionTable();

        TranspositionTable(const TranspositionTable& other) = delete;
        TranspositionTable& operator=(const TranspositionTable& other) = delete;

        TranspositionTable(TranspositionTable&& other);
        TranspositionTable& operator=(TranspositionTable&& other);

        constexpr size_t getBucketCount() const {
            return bucketCount;
        }

        void put(uint64_t hash, const TranspositionTableEntry& entry);

        bool probe(uint64_t hash, TranspositionTableEntry& entry);

        void clear();

        void resize(size_t bucketCount);
};

template<bool (*replacementPredicate)(const TranspositionTableEntry&, const TranspositionTableEntry&)>
TranspositionTable<replacementPredicate>::TranspositionTable(size_t bucketCount) {
    this->bucketCount = bucketCount;

    predicateBucket = new Entry[bucketCount];
    alwaysReplaceBucket = new Entry[bucketCount];

    clear();
}

template<bool (*replacementPredicate)(const TranspositionTableEntry&, const TranspositionTableEntry&)>
TranspositionTable<replacementPredicate>::~TranspositionTable() {
    delete[] predicateBucket;
    delete[] alwaysReplaceBucket;
}

template<bool (*replacementPredicate)(const TranspositionTableEntry&, const TranspositionTableEntry&)>
TranspositionTable<replacementPredicate>::TranspositionTable(TranspositionTable&& other) {
    bucketCount = other.bucketCount;

    predicateBucket = other.predicateBucket;
    other.predicateBucket = nullptr;

    alwaysReplaceBucket = other.alwaysReplaceBucket;
    other.alwaysReplaceBucket = nullptr;
}

template<bool (*replacementPredicate)(const TranspositionTableEntry&, const TranspositionTableEntry&)>
TranspositionTable<replacementPredicate>& TranspositionTable<replacementPredicate>::operator=(TranspositionTable&& other) {
    delete[] predicateBucket;
    delete[] alwaysReplaceBucket;

    bucketCount = other.bucketCount;

    predicateBucket = other.predicateBucket;
    other.predicateBucket = nullptr;

    alwaysReplaceBucket = other.alwaysReplaceBucket;
    other.alwaysReplaceBucket = nullptr;

    return *this;
}

template<bool (*replacementPredicate)(const TranspositionTableEntry&, const TranspositionTableEntry&)>
void TranspositionTable<replacementPredicate>::put(uint64_t hash, const TranspositionTableEntry& entry) {
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

template<bool (*replacementPredicate)(const TranspositionTableEntry&, const TranspositionTableEntry&)>
bool TranspositionTable<replacementPredicate>::probe(uint64_t hash, TranspositionTableEntry& entry) {
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

template<bool (*replacementPredicate)(const TranspositionTableEntry&, const TranspositionTableEntry&)>
void TranspositionTable<replacementPredicate>::clear() {
    // Setze alle Einträge auf nicht existent.
    for(size_t i = 0; i < bucketCount; i++) {
        predicateBucket[i].exists = false;
        alwaysReplaceBucket[i].exists = false;
    }
}

template<bool (*replacementPredicate)(const TranspositionTableEntry&, const TranspositionTableEntry&)>
void TranspositionTable<replacementPredicate>::resize(size_t bucketCount) {
    bucketCount = std::max((size_t)1, bucketCount);

    // Lösche die alten Buckets.
    delete[] predicateBucket;
    delete[] alwaysReplaceBucket;

    // Erstelle neue Buckets.
    this->bucketCount = bucketCount;

    predicateBucket = new Entry[bucketCount];
    alwaysReplaceBucket = new Entry[bucketCount];

    clear();
}

#endif