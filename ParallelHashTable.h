#ifndef PARALLEL_HASH_TABLE_H
#define PARALLEL_HASH_TABLE_H

#include <stdint.h>

template <typename K, typename V, size_t size>
class ParallelHashTable {

    private:
        struct Entry {
            K key;
            V value;
        };

        Entry* entries;

    public:
        ParallelHashTable();
        ~ParallelHashTable();

        ParallelHashTable(const ParallelHashTable&);
        ParallelHashTable& operator=(const ParallelHashTable&);

        ParallelHashTable(ParallelHashTable&& other);
        ParallelHashTable& operator=(ParallelHashTable&& other);

        void put(K key, V value);
        bool probe(K key, V& value);
        void clear();
};

template <typename K, typename V, size_t size>
ParallelHashTable<K, V, size>::ParallelHashTable() {
    entries = new Entry[size];
    clear();
}

template <typename K, typename V, size_t size>
ParallelHashTable<K, V, size>::~ParallelHashTable() {
    delete[] entries;
}

template <typename K, typename V, size_t size>
ParallelHashTable<K, V, size>::ParallelHashTable(const ParallelHashTable& other) {
    entries = other.entries;
}

template <typename K, typename V, size_t size>
ParallelHashTable<K, V, size>& ParallelHashTable<K, V, size>::operator=(const ParallelHashTable& other) {
    entries = other.entries;
    return *this;
}

template <typename K, typename V, size_t size>
ParallelHashTable<K, V, size>::ParallelHashTable(ParallelHashTable&& other) {
    delete[] entries;
    entries = other.entries;
    other.entries = nullptr;
}

template <typename K, typename V, size_t size>
ParallelHashTable<K, V, size>& ParallelHashTable<K, V, size>::operator=(ParallelHashTable&& other) {
    delete[] entries;
    entries = other.entries;
    other.entries = nullptr;
    return *this;
}

template <typename K, typename V, size_t size>
void ParallelHashTable<K, V, size>::put(K key, V value) {
    size_t index = key % size;
    entries[index].key = key ^ value;
    entries[index].value = value;
}

template <typename K, typename V, size_t size>
bool ParallelHashTable<K, V, size>::probe(K key, V& value) {
    size_t index = key % size;
    Entry entry = entries[index];

    if (entry.key == (key ^ entry.value)) {
        value = entry.value;
        return true;
    }

    return false;
}

template <typename K, typename V, size_t size>
void ParallelHashTable<K, V, size>::clear() {
    for (size_t i = 0; i < size; i++) {
        entries[i].key = 0;
        entries[i].value = 0;
    }
}

#endif