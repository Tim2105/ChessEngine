#ifndef ARRAY_H
#define ARRAY_H

#include <stdint.h>
#include <initializer_list>
#include <cstring> // memcpy & memmove

/**
 * @brief Array Klasse mit statischer Größe. Für Geschwindigkeit optimiert.
 * Bei Zugriffsoperation wird nicht geprüft, ob der Index gültig ist.
 * 
 * @tparam T Der Typ der Elemente im Array.
 * @tparam s Die Größe des Arrays.
 */
template <typename T, size_t s>
class Array {
    private:
        T array[s];
        size_t count;
    
    public:
        Array();
        Array(const Array<T, s>& other);
        Array(std::initializer_list<int32_t> list);
        ~Array();

        Array& operator=(const Array& other);

        T& operator[](size_t index) { return array[index]; };
        operator T*() { return array; };

        /**
         * @brief Fügt ein Element hinten an den Array an.
         */
        void push_back(T elem);

        /**
         * @brief Fügt ein Array hinten an den Array an.
         */
        template <size_t s2>
        void push_back(Array<T, s2>& other);

        /**
         * @brief Fügt ein Element an der angegebenen Position ein.
         */
        void insert(size_t index, T elem);

        /**
         * @brief Entfernt das erste Vorkommen des Elements aus dem Array.
         */
        void remove(T elem);

        /**
         * @brief Entfernt das Element an der angegebenen Position aus dem Array.
         */
        void remove(size_t index);

        /**
         * @brief Ersetzt das Element an der angegebenen Position durch das angegebene Element.
         */
        void replace(size_t index, T elem);

        /**
         * @brief Verschiebt alle Elemente ab der angegebenen Position eins nach links.
         * Das erste Element wird überschrieben.
         */
        void shiftLeft(size_t index);

        /**
         * @brief Verschiebt alle Elemente ab der angegebenen Position eins nach rechts.
         * Wenn das Array voll ist, wird das letzte Element überschrieben.
         */
        void shiftRight(size_t index);

        /**
         * @brief Gibt die Anzahl der Elemente im Array zurück.
         */
        size_t size() const;

        /**
         * @brief Entfernt alle Elemente aus dem Array.
         */
        void clear();

        /**
         * @brief Gibt das erste Element des Arrays zurück.
         */
        T front() const;

        constexpr T* begin() { return array; };
        constexpr T* end() { return array + count; };
};

template <typename T, size_t s>
Array<T, s>::Array() {
    count = 0;
}

template <typename T, size_t s>
Array<T, s>::Array(const Array& other) {
    count = other.count;
    memcpy(array, other.array, count * sizeof(T));
}

template <typename T, size_t s>
Array<T, s>::Array(std::initializer_list<int32_t> elems) {
    count = 0;
    for(T elem : elems) {
        this->array[count++] = elem;
    }
}

template <typename T, size_t s>
Array<T, s>::~Array() {

}

template <typename T, size_t s>
Array<T, s>& Array<T, s>::operator=(const Array& other) {
    count = other.count;
    memcpy(array, other.array, count * sizeof(T));

    return *this;
}

template <typename T, size_t s>
void Array<T, s>::push_back(T elem) {
    array[count++] = elem;
}

template <typename T, size_t s>
template <size_t s2>
void Array<T,s>::push_back(Array<T, s2>& other) {
    memcpy(array + count, other.array, other.count * sizeof(T));
    count += other.count;
}

template <typename T, size_t s>
void Array<T, s>::insert(size_t index, T elem) {
    memmove(array + index + 1, array + index, (count - index) * sizeof(T));
    array[index] = elem;
    count++;
}

template <typename T, size_t s>
void Array<T, s>::remove(T elem) {
    for(int i = 0; i < count; i++) {
        if(array[i] == elem) {
            memmove(array + i, array + i + 1, (count - i - 1) * sizeof(T));
            count--;
            return;
        }
    }
}

template <typename T, size_t s>
void Array<T, s>::remove(size_t index) {
    memmove(array + index, array + index + 1, (count - index - 1) * sizeof(T));

    count--;
}

template <typename T, size_t s>
void Array<T, s>::replace(size_t index, T elem) {
    array[index] = elem;
}

template <typename T, size_t s>
void Array<T, s>::shiftLeft(size_t index) {
    memmove(array + index, array + index + 1, (count - index - 1) * sizeof(T));
    count--;
}

template <typename T, size_t s>
void Array<T, s>::shiftRight(size_t index) {
    if(count == s)
        memmove(array + index + 1, array + index, (s - index - 1) * sizeof(T));
    else {
        memmove(array + index + 1, array + index, (count - index) * sizeof(T));
        count++;
    }
}

template <typename T, size_t s>
size_t Array<T, s>::size() const {
    return count;
}

template <typename T, size_t s>
void Array<T, s>::clear() {
    count = 0;
}

template <typename T, size_t s>
T Array<T, s>::front() const {
    return array[0];
}


#endif