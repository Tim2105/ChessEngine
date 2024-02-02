#ifndef ARRAY_H
#define ARRAY_H

#include <algorithm>
#include <cstdalign>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <type_traits>
#include <stdint.h>
#include <vector>

/**
 * @brief Array Klasse mit statischer Größe. Für Geschwindigkeit optimiert.
 * Bei einer Zugriffsoperation wird nicht geprüft, ob der Index gültig ist.
 * 
 * @tparam T Der Typ der Elemente im Array.
 * @tparam s Die Größe des Arrays.
 */
template <typename T, size_t s>
class Array {
    /**
     * @brief Compile-Time Funktion, die die Ausrichtung des Arrays bestimmt.
     * Ab einer Größe von 256 Byte wird das Array auf 64 Byte (Cache Line) ausgerichtet.
     */
    static constexpr size_t DETERMINE_ALIGNMENT() {
        if constexpr(sizeof(T) * s >= 256)
            return 64;
        else
            return alignof(T);
    }

    public:
        alignas(DETERMINE_ALIGNMENT()) T array[s];
        size_t count;

        constexpr Array() : count(0) {}

        Array(const Array<T, s>& other) {
            count = other.count;
            memcpy(array, other.array, sizeof(T) * count);
        }

        constexpr Array(const std::initializer_list<T>& list) {
            count = 0;
            for(const T& elem : list)
                array[count++] = elem;
        }

        constexpr ~Array() = default;

        constexpr Array& operator=(const Array& other) {
            if(this == &other)
                return *this;

            count = other.count;
            memcpy(array, other.array, sizeof(T) * count);

            return *this;
        }

        constexpr T& operator[](size_t index) { return array[index]; };
        constexpr const T& operator[](size_t index) const { return array[index]; };

        constexpr T& operator[](int32_t index) { return array[index]; };
        constexpr const T& operator[](int32_t index) const { return array[index]; };

        constexpr operator T*() { return array; };
        constexpr operator const T*() const { return array; };

        /**
         * @brief Fügt ein Element hinten an den Array an.
         */
        inline void push_back(const T& elem) {
            array[count++] = elem;
        }

        /**
         * @brief Entfernt das letzte Element aus dem Array und gibt es zurück.
         */
        inline T pop_back() {
            return array[--count];
        }

        /**
         * @brief Fügt ein Array hinten an das Array an.
         */
        template <size_t s2>
        inline void push_back(const Array<T, s2>& other) {
            memcpy(array + count, other.array, sizeof(T) * other.count);
            count += other.count;
        }

        /**
         * @brief Fügt ein Element an der angegebenen Position ein.
         */
        inline void insert(size_t index, const T& elem) {
            std::copy_backward(array + index, array + count, array + count + 1);
            array[index] = elem;
            count++;
        }

        /**
         * @brief Fügt ein Element sortiert in den Array ein
         * unter der Annahme, dass der Array bereits sortiert ist.
         * 
         * @param elem Das einzufügende Element.
         * @param compare Die Vergleichsfunktion, die die Sortierung des Arrays bestimmt.
         *                Standardmäßig wird std::less<T> verwendet, sodass das Array aufsteigend sortiert bleibt.
         */
        inline void insert_sorted(const T& elem, std::function<bool(const T&, const T&)> compare = std::less<T>()) {
            size_t insertionIndex = std::lower_bound(array, array + count, elem, compare) - array;
            insert(insertionIndex, elem);
        }

        /**
         * @brief Entfernt das erste Vorkommen des Elements aus dem Array.
         * 
         * @return true, wenn das Element entfernt wurde, sonst false.
         */
        inline bool remove_first(const T& elem) {
            for(size_t i = 0; i < count; i++) {
                if(array[i] == elem) {
                    std::copy(array + i + 1, array + count, array + i);
                    count--;
                    return true;
                }
            }

            return false;
        }

        /**
         * @brief Entfernt das Element an der angegebenen Position aus dem Array.
         */
        inline void remove(size_t index) {
            std::copy(array + index + 1, array + count, array + index);

            count--;
        }

        /**
         * @brief Ersetzt das Element an der angegebenen Position durch das angegebene Element.
         */
        inline void replace(size_t index, const T& elem) {
            array[index] = elem;
        }

        /**
         * @brief Verschiebt alle Elemente ab der angegebenen Position eins nach links.
         * Das erste Element wird überschrieben.
         */
        inline void shiftLeft(size_t index) {
            std::copy(array + index + 1, array + count, array + index);
            count--;
        }

        /**
         * @brief Verschiebt alle Elemente ab der angegebenen Position eins nach rechts.
         * Wenn das Array voll ist, wird das letzte Element überschrieben.
         */
        inline void shiftRight(size_t index) {
            if(count == s)
                std::copy_backward(array + index, array + count - 1, array + count);
            else {
                std::copy_backward(array + index, array + count, array + count + 1);
                count++;
            }
        }

        /**
         * @brief Gibt die Anzahl der Elemente im Array zurück.
         */
        constexpr size_t size() const { return count; };

        /**
         * @brief Entfernt alle Elemente aus dem Array.
         */
        constexpr void clear() {
            count = 0;
        }

        /**
         * @brief Ändert die Größe des Arrays.
         * 
         * @param newSize Die neue Größe des Arrays.
         */
        constexpr void resize(size_t newSize) {
            count = newSize;
        }

        /**
         * @brief Gibt das erste Element des Arrays zurück.
         */
        inline T front() const {
            return array[0];
        }

        inline T& front() {
            return array[0];
        }

        /**
         * @brief Gibt das letzte Element des Arrays zurück.
         */
        inline T back() const {
            return array[count - 1];
        }

        inline T& back() {
            return array[count - 1];
        }

        /**
         * @brief Überprüft, ob ein Element im Array enthalten ist.
         * 
         * Existiert nur, wenn T ein trivialer Typ ist.
         */
        constexpr bool contains(const T& elem) const requires(std::is_trivial_v<T>) {
            for(size_t i = 0; i < count; i++) {
                if(array[i] == elem)
                    return true;
            }

            return false;
        }

        /**
         * @brief Überprüft, ob ein Element im Array enthalten ist.
         * 
         * Existiert nur, wenn T kein trivialer Typ ist.
         */
        inline bool contains(const T& elem) const requires(!std::is_trivial_v<T>) {
            for(size_t i = 0; i < count; i++) {
                if(array[i] == elem)
                    return true;
            }

            return false;
        }

        constexpr const T* begin() const { return array; };
        constexpr const T* end() const { return array + count; };

        constexpr T* begin() { return array; };
        constexpr T* end() { return array + count; };
};

#endif