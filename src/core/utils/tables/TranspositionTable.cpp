#include "core/utils/tables/TranspositionTable.h"

TranspositionTable::TranspositionTable(size_t capacity) {
    // Die Kapazität muss mindestens 1 sein (sonst rechnen wir später Modulo 0).
    capacity = std::max(capacity, (size_t)1);

    this->capacity = capacity;
    entriesWritten.store(0);
    entries = new Entry[capacity];

    // Wir initialisieren die Tabelle mit leeren Einträgen,
    // sodass wir später feststellen können, ob ein Eintrag
    // bereits belegt ist.
    std::fill(entries, entries + capacity, Entry{0, 0});
}

TranspositionTable::~TranspositionTable() {
    delete[] entries;
}

TranspositionTable::TranspositionTable(TranspositionTable&& other) {
    entries = other.entries;
    capacity = other.capacity;
    entriesWritten.store(other.entriesWritten.load());

    other.entries = nullptr;
    other.capacity = 0;
    other.entriesWritten.store(0);
}

TranspositionTable& TranspositionTable::operator=(TranspositionTable&& other) {
    entries = other.entries;
    capacity = other.capacity;
    entriesWritten.store(other.entriesWritten.load());

    other.entries = nullptr;
    other.capacity = 0;
    other.entriesWritten.store(0);

    return *this;
}

void TranspositionTable::put(uint64_t hash, const TranspositionTableEntry& entry) {
    // Wir berechnen den Index des Buckets, in dem wir den Eintrag speichern wollen.
    size_t index = hash % capacity;
    
    if(entries[index].data == 0) {
        // Der Bucket ist leer, wir können den Eintrag einfach speichern.
        entries[index].hash = hash ^ entry; // XOR, damit der Hashwert gleichzeitig als Prüfwert dient.
        entries[index].data = entry;
        entriesWritten.fetch_add(1);
    } else {
        // Der Bucket ist bereits belegt, wir müssen prüfen, ob der Eintrag
        // eine höhere Priorität als der bisherige Eintrag hat.
        if(entry > entries[index].data) {
            entries[index].hash = hash ^ entry; // XOR, damit der Hashwert gleichzeitig als Prüfwert dient.
            entries[index].data = entry;
        }
    }
}

bool TranspositionTable::probe(uint64_t hash, TranspositionTableEntry& entry) {
    // Wir berechnen den Index des Buckets, in dem ein Eintrag zu dem Hashwert
    // gespeichert sein würde (wenn er existiert).
    size_t index = hash % capacity;

    // Überprüfe, ob der Hashwert mit dem gespeicherten Hashwert übereinstimmt
    // (XOR muss vor dem Vergleich brückgängig gemacht werden).
    // Der Randfall, dass der Hashwert 0 ist und der gespeicherte Eintrag im Bucket
    // leer ist (Hash = 0, Data = 0), muss nicht abgedeckt werden, weil das, dann in
    // entry geschriebene, Objekt in diesem Fall eine eingetragene Tiefe von 0 und keinen
    // Hashzug hat (exists() gibt false zurück). Der Eintrag wird dann in der Suche
    // sowieso ignoriert.
    if((entries[index].hash ^ entries[index].data) == hash) {
        // Der Eintrag existiert und der Hashwert stimmt überein.
        entry = entries[index].data;
        return true;
    }

    // Die Transpositionstabelle enthält keinen Eintrag für den Hashwert.
    return false;
}

void TranspositionTable::clear() {
    std::fill(entries, entries + capacity, Entry{0, 0});
    entriesWritten.store(0);
}

void TranspositionTable::resize(size_t capacity) {
    // Die Kapazität muss mindestens 1 sein (sonst rechnen wir später Modulo 0).
    capacity = std::max(capacity, (size_t)1);

    delete[] entries;
    entries = new Entry[capacity];
    this->capacity = capacity;
    entriesWritten.store(0);

    // Wir initialisieren die Tabelle mit leeren Einträgen,
    // sodass wir später feststellen können, ob ein Eintrag
    // bereits belegt ist.
    std::fill(entries, entries + capacity, Entry{0, 0});
}