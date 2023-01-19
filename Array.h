#ifndef ARRAY_H
#define ARRAY_H

#include <stdint.h>
#include <initializer_list>
#include <cstring> // memcpy

template <typename T, size_t s>
class Array {
    private:
        T array[s];
        size_t count;
    
    public:
        Array();
        Array(const Array& other);
        Array(std::initializer_list<int32_t> squares);
        ~Array();

        Array& operator=(const Array& other);

        T& operator[](size_t index) { return array[index]; };
        operator T*() { return array; };

        void push_back(T elem);
        void remove(T elem);
        size_t size() const;
        void clear();
        T front();

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
void Array<T, s>::remove(T elem) {
    for(int i = 0; i < count; i++) {
        if(array[i] == elem) {
            for(int j = i; j < count - 1; j++) {
                array[j] = array[j + 1];
            }

            count--;
            return;
        }
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
T Array<T, s>::front() {
    return array[0];
}


#endif