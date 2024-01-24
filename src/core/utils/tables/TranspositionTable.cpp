#include "core/utils/tables/TranspositionTable.h"

#include <immintrin.h>
#include <new>
#include <stdalign.h>

TranspositionTable::TranspositionTable(size_t capacity) {
    // Die Kapazität muss mindestens 1 sein (sonst rechnen wir später Modulo 0).
    capacity = std::max(capacity, (size_t)1);

    this->capacity = capacity;
    entriesWritten.store(0);

    // Wir reservieren den Speicher für die Tabelle.
    // Falls SSE4.1 verfügbar ist, reservieren wir den Speicher
    // mit 16-Byte Ausrichtung, damit wir die Einträge mit
    // einer Vektorinstruktion laden und speichern können.
    #if defined(__SSE4_1__)
        entries = new (std::align_val_t(16)) Entry[capacity];
    #else
        entries = new Entry[capacity];
    #endif

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

void TranspositionTable::put(uint64_t hash, const TranspositionTableEntry& entry) noexcept {
    // Wir berechnen den Index des Buckets, in dem wir den Eintrag speichern wollen.
    size_t index = hash % capacity;
    
    if(entries[index].data == 0) {
        // Der Bucket ist leer, wir können den Eintrag einfach speichern.

        #if defined(__SSE4_1__)
            // Wir speichern den Eintrag mit SSE4.1.
            // Ausgerichtetes Speichern ist auf vielen Prozessoren
            // atomar, sodass wir korrumpierte Einträge vermeiden können.
            __m128i entry128 = _mm_set_epi64x(entry, hash ^ entry);
            _mm_store_si128((__m128i*)&entries[index], entry128);
        #else
            entries[index].hash = hash ^ entry; // XOR, damit der Hashwert gleichzeitig als Prüfwert dient.
            entries[index].data = entry;
        #endif

        entriesWritten.fetch_add(1);
    } else {
        // Der Bucket ist bereits belegt, wir müssen prüfen, ob der Eintrag
        // eine höhere Priorität als der bisherige Eintrag hat.
        if(entry > entries[index].data) {
            // Der Eintrag hat eine höhere Priorität, wir überschreiben den alten Eintrag.

            #if defined(__SSE4_1__)
                // Wir speichern den Eintrag mit SSE4.1.
                // Ausgerichtetes Speichern ist auf vielen Prozessoren
                // atomar, sodass wir korrumpierte Einträge vermeiden können.
                __m128i entry128 = _mm_set_epi64x(entry, hash ^ entry);
                _mm_store_si128((__m128i*)&entries[index], entry128);
            #else
                entries[index].hash = hash ^ entry; // XOR, damit der Hashwert gleichzeitig als Prüfwert dient.
                entries[index].data = entry;
            #endif
        }
    }
}

bool TranspositionTable::probe(uint64_t hash, TranspositionTableEntry& entry) const noexcept {
    // Wir berechnen den Index des Buckets, in dem ein Eintrag zu dem Hashwert
    // gespeichert sein würde (wenn er existiert).
    size_t index = hash % capacity;

    #if defined(__SSE4_1__)
        // Wir laden den Eintrag mit SSE4.1.
        // Ausgerichtetes Laden ist wie ausgerichtetes Speichern
        // auf vielen Prozessoren atomar.
        __m128i entry128 = _mm_load_si128((__m128i*)&entries[index]);
        uint64_t entryHash = _mm_extract_epi64(entry128, 0);
        uint64_t entryData = _mm_extract_epi64(entry128, 1);
    #else
        uint64_t entryHash = entries[index].hash;
        uint64_t entryData = entries[index].data;
    #endif


    // Überprüfe, ob der Hashwert mit dem gespeicherten Hashwert übereinstimmt
    // (XOR muss vor dem Vergleich brückgängig gemacht werden).
    // Der Randfall, dass der Hashwert 0 ist und der gespeicherte Eintrag im Bucket
    // leer ist (Hash = 0, Data = 0), muss nicht abgedeckt werden, weil das, dann in
    // entry geschriebene, Objekt in diesem Fall eine eingetragene Tiefe von 0 und keinen
    // Hashzug hat (exists() gibt false zurück). Der Eintrag wird dann in der Suche
    // sowieso ignoriert.
    if((entryHash ^ entryData) == hash) {
        // Der Eintrag existiert und der Hashwert stimmt überein.
        entry = entryData;
        return true;
    }

    // Die Transpositionstabelle enthält keinen Eintrag für den Hashwert.
    return false;
}

void TranspositionTable::clear() noexcept {
    std::fill(entries, entries + capacity, Entry{0, 0});
    entriesWritten.store(0);
}

void TranspositionTable::resize(size_t capacity) {
    // Die Kapazität muss mindestens 1 sein (sonst rechnen wir später Modulo 0).
    capacity = std::max(capacity, (size_t)1);

    delete[] entries;

    try {
        // Wir reservieren neuen Speicher für die Tabelle.
        // Falls SSE4.1 verfügbar ist, reservieren wir den Speicher
        // mit 16-Byte Ausrichtung, damit wir die Einträge mit
        // einer Vektorinstruktion laden und speichern können.
        #if defined(__SSE4_1__)
            entries = new (std::align_val_t(16)) Entry[capacity];
        #else
            entries = new Entry[capacity];
        #endif
    } catch(std::bad_alloc& e) {
        // Es ist nicht genügend Speicher verfügbar.
        
        // Wir stellen sicher, dass wir die Kapazität der alten Tabelle wiederherstellen.
        // Wir haben jetzt zwar alle alten Einträge verloren, aber das ist besser als
        // überhaupt keine Tabelle zu haben.
        #if defined(__SSE4_1__)
            entries = new (std::align_val_t(16)) Entry[this->capacity];
        #else
            entries = new Entry[this->capacity];
        #endif

        entriesWritten.store(0);
        // Initialisiere den neuen Speicher mit leeren Einträgen.
        std::fill(entries, entries + this->capacity, Entry{0, 0});

        // Leite die Exception weiter.
        throw e;
    }

    this->capacity = capacity;
    entriesWritten.store(0);

    // Wir initialisieren die Tabelle mit leeren Einträgen,
    // sodass wir später feststellen können, ob ein Eintrag
    // bereits belegt ist.
    std::fill(entries, entries + capacity, Entry{0, 0});
}