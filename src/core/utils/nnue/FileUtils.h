#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <istream>
#include <ostream>
#include <stdint.h>

namespace NNUE {
    template <typename T>
    void readLittleEndian(std::istream& is, T& t) {
        #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            is.read(reinterpret_cast<char*>(&t), sizeof(T));
        #else
            char* ptr = reinterpret_cast<char*>(&t);
            for(size_t i = 0; i < sizeof(T); i++)
                is.read(ptr + sizeof(T) - i - 1, 1);
        #endif
    }

    template <typename T>
    void readLittleEndian(std::istream& is, T* buffer, size_t count) {
        #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            is.read(reinterpret_cast<char*>(buffer), sizeof(T) * count);
        #else
            for(size_t i = 0; i < count; i++)
                readLittleEndian(is, buffer[i]);
        #endif
    }

    template <typename T>
    void writeLittleEndian(std::ostream& os, const T& t) {
        #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            os.write(reinterpret_cast<const char*>(&t), sizeof(T));
        #else
            const char* ptr = reinterpret_cast<const char*>(&t);
            for(size_t i = 0; i < sizeof(T); i++)
                os.write(ptr + sizeof(T) - i - 1, 1);
        #endif 
    }

    template <typename T>
    void writeLittleEndian(std::ostream& os, const T* buffer, size_t count) {
        #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            os.write(reinterpret_cast<const char*>(buffer), sizeof(T) * count);
        #else
            for(size_t i = 0; i < count; i++)
                writeLittleEndian(os, buffer[i]);
        #endif
    }
}

#endif