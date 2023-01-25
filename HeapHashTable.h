#ifndef HEAP_HASH_TABLE_H
#define HEAP_HASH_TABLE_H

#include <stdint.h>
#include <functional>
#include <cstring>

/**
 * @brief Hash-Tabelle mit statischer Größe. Für Geschwindigkeit optimiert.
 * Bei einem Hash-Konflikt wird das älteste Element überschrieben.
 * 
 * @tparam K Der Schlüsseltyp. Muss std::hash<K>{}(key) unterstützen.
 * @tparam V Der Wertetyp.
 * @tparam bucketCount Anzahl der Indizes(sollte ein Teiler der Anzahl der möglichen Hashwerte sein).
 * @tparam bucketSize  Anzahl der Elemente, die pro Index gespeichert werden können.
 */
template <typename K, typename V, size_t bucketCount, size_t bucketSize>
class HeapHashTable {
    private:
        struct Entry {
            K key;
            V value;
        };

        Entry* table;
        int32_t bucketSizes[bucketCount];

    public:
        HeapHashTable();
        ~HeapHashTable();

        HeapHashTable(const HeapHashTable& other) = delete;
        HeapHashTable& operator=(const HeapHashTable& other) = delete;

        HeapHashTable(HeapHashTable&& other);
        HeapHashTable& operator=(HeapHashTable&& other);

        /**
         * @brief Fügt ein Element in die Hash-Tabelle ein.
         */
        void put(K key, V value);

        /**
         * @brief Überprüft, ob ein Schlüssel existiert.
         * Wenn ja, wird der Wert in value geschrieben.
         * 
         * @param key Der Schlüssel des Elements.
         * @param value Der Wert wird hierhin geschrieben.
         * @return true Wenn das Element gefunden wurde.
         * @return false Wenn das Element nicht gefunden wurde.
         */
        bool probe(K key, V& value);

        /**
         * @brief Löscht alle Elemente aus der Hash-Tabelle.
         */
        void clear();
};

template <typename K, typename V, size_t bucketCount, size_t bucketSize>
HeapHashTable<K, V, bucketCount, bucketSize>::HeapHashTable() {
    table = new Entry[bucketCount * bucketSize];

    for (size_t i = 0; i < bucketCount; i++) {
        bucketSizes[i] = 0;
    }
}

template <typename K, typename V, size_t bucketCount, size_t bucketSize>
HeapHashTable<K, V, bucketCount, bucketSize>::~HeapHashTable() {
    delete[] table;
}

template <typename K, typename V, size_t bucketCount, size_t bucketSize>
HeapHashTable<K, V, bucketCount, bucketSize>::HeapHashTable(HeapHashTable&& other) {
    table = other.table;
    other.table = nullptr;

    for (size_t i = 0; i < bucketCount; i++) {
        bucketSizes[i] = other.bucketSizes[i];
    }
}

template <typename K, typename V, size_t bucketCount, size_t bucketSize>
HeapHashTable<K, V, bucketCount, bucketSize>& HeapHashTable<K, V, bucketCount, bucketSize>::operator=(HeapHashTable&& other) {
    table = other.table;
    other.table = nullptr;

    for (size_t i = 0; i < bucketCount; i++) {
        bucketSizes[i] = other.bucketSizes[i];
    }

    return *this;
}

template <typename K, typename V, size_t bucketCount, size_t bucketSize>
void HeapHashTable<K, V, bucketCount, bucketSize>::put(K key, V value) {
    size_t index = std::hash<K>{}(key) % bucketCount;
    Entry* bucket = table + index * bucketSize;

    // Überprüfe, ob der Schlüssel bereits existiert.
    for (size_t i = 0; i < bucketSizes[index]; i++) {
        if (bucket[i].key == key) {
            bucket[i].value = value;
            return;
        }
    }

    // Wenn der Schlüssel nicht existiert, füge ihn hinzu.
    if (bucketSizes[index] < bucketSize) {
        bucket[bucketSizes[index]].key = key;
        bucket[bucketSizes[index]].value = value;
        bucketSizes[index]++;
    } else {
        // Wenn der Bucket voll ist, überschreibe das älteste Element.
        memmove(bucket, bucket + 1, sizeof(Entry) * (bucketSize - 1));

        bucket[bucketSize - 1].key = key;
        bucket[bucketSize - 1].value = value;
    }
}

template <typename K, typename V, size_t bucketCount, size_t bucketSize>
bool HeapHashTable<K, V, bucketCount, bucketSize>::probe(K key, V& value) {
    size_t index = std::hash<K>{}(key) % bucketCount;
    Entry* bucket = table + index * bucketSize;

    for (size_t i = 0; i < bucketSize; i++) {
        if (bucket[i].key == key) {
            value = bucket[i].value;
            return true;
        }
    }

    return false;
}

template <typename K, typename V, size_t bucketCount, size_t bucketSize>
void HeapHashTable<K, V, bucketCount, bucketSize>::clear() {
    for (size_t i = 0; i < bucketCount; i++) {
        bucketSizes[i] = 0;
    }
}

#endif