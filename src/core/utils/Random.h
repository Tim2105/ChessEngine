#ifndef RANDOM_H
#define RANDOM_H

#include <atomic>
#include <cstdint>
#include <random>

namespace Random {
    inline constexpr uint32_t BASE_SEED = 42;

    inline uint32_t mix32(uint32_t x) {
        x ^= x >> 16;
        x *= 0x7FEB352Du;
        x ^= x >> 15;
        x *= 0x846CA68Bu;
        x ^= x >> 16;
        return x;
    }

    inline uint32_t threadIndex() {
        static std::atomic<uint32_t> nextIndex = 0;
        static thread_local uint32_t index = nextIndex.fetch_add(1);
        return index;
    }

    template <uint32_t StreamId>
    inline std::mt19937& generator() {
        static thread_local std::mt19937 rng(
            mix32(BASE_SEED ^ (StreamId * 0x9E3779B9u) ^ (threadIndex() * 0x85EBCA6Bu))
        );
        return rng;
    }
}

#endif