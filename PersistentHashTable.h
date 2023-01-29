#ifndef PERSISTENT_HASH_TABLE_H
#define PERSISTENT_HASH_TABLE_H

#include <vector>

template <typename K, typename V, size_t bucketCount, size_t bucketSize>
class PersistentHashTable
{
    private:
        struct Entry
        {
            K key;
            V value;
        };

        std::vector<Entry> table[bucketCount];

    public:
        PersistentHashTable();
        ~PersistentHashTable();

        PersistentHashTable(const PersistentHashTable& other);

        PersistentHashTable& operator=(const PersistentHashTable& other);

        void put(K key, V value);

        bool probe(K key, V& value);

        void remove(K key);
};

template <typename K, typename V, size_t bucketCount, size_t bucketSize>
PersistentHashTable<K, V, bucketCount, bucketSize>::PersistentHashTable()
{
    for(size_t i = 0; i < bucketCount; i++)
        table[i].reserve(bucketSize);
}

template <typename K, typename V, size_t bucketCount, size_t bucketSize>
PersistentHashTable<K, V, bucketCount, bucketSize>::~PersistentHashTable()
{
}

template <typename K, typename V, size_t bucketCount, size_t bucketSize>
PersistentHashTable<K, V, bucketCount, bucketSize>::PersistentHashTable(const PersistentHashTable& other)
{
    for(size_t i = 0; i < bucketCount; i++)
        table[i] = other.table[i];
}

template <typename K, typename V, size_t bucketCount, size_t bucketSize>
PersistentHashTable<K, V, bucketCount, bucketSize>& PersistentHashTable<K, V, bucketCount, bucketSize>::operator=(const PersistentHashTable& other)
{
    if(this != &other)
        for(size_t i = 0; i < bucketCount; i++)
            table[i] = other.table[i];

    return *this;
}

template <typename K, typename V, size_t bucketCount, size_t bucketSize>
void PersistentHashTable<K, V, bucketCount, bucketSize>::put(K key, V value)
{
    size_t bucket = key % bucketCount;

    for(size_t i = 0; i < table[bucket].size(); i++)
        if(table[bucket][i].key == key) {
            table[bucket][i].value = value;
        }

    table[bucket].push_back(Entry{key, value});
}

template <typename K, typename V, size_t bucketCount, size_t bucketSize>
bool PersistentHashTable<K, V, bucketCount, bucketSize>::probe(K key, V& value)
{
    size_t bucket = key % bucketCount;

    for (size_t i = 0; i < table[bucket].size(); i++)
        if(table[bucket][i].key == key) {
            value = table[bucket][i].value;
            return true;
        }

    return false;
}

template <typename K, typename V, size_t bucketCount, size_t bucketSize>
void PersistentHashTable<K, V, bucketCount, bucketSize>::remove(K key)
{
    size_t bucket = key % bucketCount;

    for(size_t i = 0; i < table[bucket].size(); i++)
        if(table[bucket][i].key == key) {
            table[bucket].erase(table[bucket].begin() + i);
            return;
        }
}

#endif