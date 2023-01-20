#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "Array.h"
#include <stdint.h>
#include <functional>

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
class HashTable {
    private:
        struct Entry {
            K key;
            V value;
        };

        Array<Entry, bucketSize> table[bucketCount];

    public:
        HashTable();

        /**
         * @brief Fügt ein Element in die Hash-Tabelle ein.
         */
        void put(K key, V value);

        /**
         * @brief Gibt den Wert des Elements mit dem Schlüssel key zurück.
         * 
         * @param key Der Schlüssel des Elements.
         * @param value Der Wert wird hierhin geschrieben.
         * @return true Wenn das Element gefunden wurde.
         * @return false Wenn das Element nicht gefunden wurde.
         */
        bool get(K key, V& value);
};

template <typename K, typename V, size_t bucketCount, size_t bucketSize>
HashTable<K, V, bucketCount, bucketSize>::HashTable() {
    
}

template <typename K, typename V, size_t bucketCount, size_t bucketSize>
void HashTable<K, V, bucketCount, bucketSize>::put(K key, V value) {
    size_t hash = std::hash<K>{}(key);
    size_t index = hash % bucketCount;

    if(table[index].size() == bucketSize)
        table[index].remove(0);

    table[index].push_back(Entry{key, value});
}

template <typename K, typename V, size_t bucketCount, size_t bucketSize>
bool HashTable<K, V, bucketCount, bucketSize>::get(K key, V& value) {
    size_t hash = std::hash<K>{}(key);
    size_t index = hash % bucketCount;

    for(size_t i = 0; i < table[index].size(); i++) {
        if(table[index][i].key == key) {
            value = table[index][i].value;
            return true;
        }
    }

    return false;
}

#endif