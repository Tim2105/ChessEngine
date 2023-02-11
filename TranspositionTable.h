#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include <stdint.h>
#include "Move.h"
#include <cstring>
#include <functional>

struct TranspositionTableEntry {
    uint16_t age;
    int16_t depth;
    int16_t score;
    uint8_t type;
    Move hashMove;

    bool operator<(const TranspositionTableEntry& other) const {
        if(age == other.age)
            return depth < other.depth;

        return age > other.age;
    }

    bool operator>(const TranspositionTableEntry& other) const {
        return (uint16_t)depth + age > (uint16_t)other.depth + other.age;
    }
};

template<size_t bucketCount, size_t bucketSize>
class TranspositionTable {
    private:
        struct Entry {
            uint64_t hash;
            TranspositionTableEntry entry;
        };

        static constexpr auto replacementPredicate = [](const Entry& lhs, const TranspositionTableEntry& rhs) {
            return lhs.entry > rhs;
        };

        static constexpr auto equalityPredicate = [](const Entry& lhs, const uint64_t& rhs) {
            return lhs.hash == rhs;
        };

        Entry* table;
        int32_t bucketSizes[bucketCount];
    
    public:
        TranspositionTable();
        ~TranspositionTable();

        TranspositionTable(const TranspositionTable& other) = delete;
        TranspositionTable& operator=(const TranspositionTable& other) = delete;

        TranspositionTable(TranspositionTable&& other);
        TranspositionTable& operator=(TranspositionTable&& other);

        void put(uint64_t hash, TranspositionTableEntry entry);

        bool probe(uint64_t hash, TranspositionTableEntry& entry);

        void clear();
};

template<size_t bucketCount, size_t bucketSize>
TranspositionTable<bucketCount, bucketSize>::TranspositionTable() {
    table = new Entry[bucketCount * bucketSize];

    for (size_t i = 0; i < bucketCount; i++) {
        bucketSizes[i] = 0;
    }
}

template<size_t bucketCount, size_t bucketSize>
TranspositionTable<bucketCount, bucketSize>::~TranspositionTable() {
    delete[] table;
}

template<size_t bucketCount, size_t bucketSize>
TranspositionTable<bucketCount, bucketSize>::TranspositionTable(TranspositionTable&& other) {
    delete[] table;
    table = other.table;
    other.table = nullptr;

    for (size_t i = 0; i < bucketCount; i++) {
        bucketSizes[i] = other.bucketSizes[i];
    }
}

template<size_t bucketCount, size_t bucketSize>
TranspositionTable<bucketCount, bucketSize>& TranspositionTable<bucketCount, bucketSize>::operator=(TranspositionTable&& other) {
    delete[] table;
    table = other.table;
    other.table = nullptr;

    for (size_t i = 0; i < bucketCount; i++) {
        bucketSizes[i] = other.bucketSizes[i];
    }

    return *this;
}

template <size_t bucketCount, size_t bucketSize>
void TranspositionTable<bucketCount, bucketSize>::put(uint64_t hash, TranspositionTableEntry entry) {
    size_t index = hash % bucketCount;
    Entry* bucket = table + index * bucketSize;

    // Überprüfe, ob der Schlüssel bereits existiert.
    for (size_t i = 0; i < bucketSizes[index]; i++) {
        if (bucket[i].hash == hash) {
            bucket[i].entry = entry;
            return;
        }
    }

    // Wenn der Schlüssel nicht existiert, füge ihn hinzu.
    if (bucketSizes[index] < bucketSize) {
        int32_t insertionIndex = std::lower_bound(bucket, bucket + bucketSizes[index], entry, replacementPredicate) - bucket;
        
        memmove(bucket + insertionIndex + 1, bucket + insertionIndex, sizeof(Entry) * (bucketSizes[index] - insertionIndex));
        bucket[insertionIndex].hash = hash;
        bucket[insertionIndex].entry = entry;

        bucketSizes[index]++;
    } else {
        int32_t insertionIndex = std::lower_bound(bucket, bucket + bucketSize, entry, replacementPredicate) - bucket;
        if (insertionIndex < bucketSize) {
            memmove(bucket + insertionIndex + 1, bucket + insertionIndex, sizeof(Entry) * (bucketSize - insertionIndex - 1));
            bucket[insertionIndex].hash = hash;
            bucket[insertionIndex].entry = entry;
        }
    }
}

template <size_t bucketCount, size_t bucketSize>
bool TranspositionTable<bucketCount, bucketSize>::probe(uint64_t hash, TranspositionTableEntry& entry) {
    size_t index = hash % bucketCount;
    Entry* bucket = table + index * bucketSize;

    for (size_t i = 0; i < bucketSizes[index]; i++) {
        if (bucket[i].hash == hash) {
            entry = bucket[i].entry;
            return true;
        }
    }

    return false;
}

template <size_t bucketCount, size_t bucketSize>
void TranspositionTable<bucketCount, bucketSize>::clear() {
    for (size_t i = 0; i < bucketCount; i++) {
        bucketSizes[i] = 0;
    }
}

#endif