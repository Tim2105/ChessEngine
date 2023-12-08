#ifndef ARRAY_H
#define ARRAY_H

#include <type_traits>
#include <cstring> // memcpy & memmove
#include <initializer_list>
#include <stdint.h>
#include <vector>

/**
 * @brief Array Klasse mit statischer Größe. Für Geschwindigkeit optimiert.
 * Bei Zugriffsoperation wird nicht geprüft, ob der Index gültig ist.
 * 
 * @tparam T Der Typ der Elemente im Array.
 * @tparam s Die Größe des Arrays.
 */
template <typename T, size_t s>
class Array {

    public:
        T array[s];
        size_t count;

        constexpr Array() : count(0) {}

        constexpr Array(const Array<T, s>& other) {
            count = other.count;
            if constexpr(!std::is_constant_evaluated())
                memcpy(array, other.array, count * sizeof(T));
            else {
                for(size_t i = 0; i < count; i++) {
                    array[i] = other.array[i];
                }

                for(size_t i = count; i < s; i++) {
                    array[i] = T();
                }
            }
        }

        constexpr Array(const std::initializer_list<T>& list) {
            count = 0;
            for(T elem : list) {
                array[count++] = elem;
            }

            for(size_t i = count; i < s; i++)
                array[i] = T();
        }

        constexpr ~Array() {}

        constexpr Array& operator=(const Array& other) {
            count = other.count;
            memcpy(array, other.array, count * sizeof(T));

            return *this;
        }

        constexpr T& operator[](size_t index) { return array[index]; };
        constexpr operator T*() { return array; };

        /**
         * @brief Fügt ein Element hinten an den Array an.
         */
        inline void push_back(T elem) {
            array[count++] = elem;
        }

        /**
         * @brief Fügt ein Array hinten an den Array an.
         */
        template <size_t s2>
        inline void push_back(Array<T, s2>& other) {
            memmove(array + count, other.array, other.count * sizeof(T));
            count += other.count;
        }

        /**
         * @brief Fügt ein Element an der angegebenen Position ein.
         */
        inline void insert(size_t index, T elem) {
            memmove(array + index + 1, array + index, (count - index) * sizeof(T));
            array[index] = elem;
            count++;
        }

        /**
         * @brief Entfernt das erste Vorkommen des Elements aus dem Array.
         */
        inline void remove(T elem) {
            for(size_t i = 0; i < count; i++) {
                if(array[i] == elem) {
                    memmove(array + i, array + i + 1, (count - i - 1) * sizeof(T));
                    count--;
                    return;
                }
            }
        }

        /**
         * @brief Entfernt das Element an der angegebenen Position aus dem Array.
         */
        inline void remove(size_t index) {
            memmove(array + index, array + index + 1, (count - index - 1) * sizeof(T));

            count--;
        }

        /**
         * @brief Ersetzt das Element an der angegebenen Position durch das angegebene Element.
         */
        inline void replace(size_t index, T elem) {
            array[index] = elem;
        }

        /**
         * @brief Verschiebt alle Elemente ab der angegebenen Position eins nach links.
         * Das erste Element wird überschrieben.
         */
        inline void shiftLeft(size_t index) {
            memmove(array + index, array + index + 1, (count - index - 1) * sizeof(T));
            count--;
        }

        /**
         * @brief Verschiebt alle Elemente ab der angegebenen Position eins nach rechts.
         * Wenn das Array voll ist, wird das letzte Element überschrieben.
         */
        inline void shiftRight(size_t index) {
            if(count == s)
                memmove(array + index + 1, array + index, (s - index - 1) * sizeof(T));
            else {
                memmove(array + index + 1, array + index, (count - index) * sizeof(T));
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
         * @brief Gibt das erste Element des Arrays zurück.
         */
        inline T front() const {
            return array[0];
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