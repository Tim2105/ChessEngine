#ifndef ATOMIC_H
#define ATOMIC_H

#if not defined(DISABLE_THREADS)
    #include <atomic>
#endif

#include <stdint.h>

#if not defined(DISABLE_THREADS)
    template <typename T>
    using Atomic = std::atomic<T>;

    using AtomicBool = std::atomic<bool>;
    using AtomicU64 = std::atomic<uint64_t>;
    using AtomicSize = std::atomic<size_t>;
#else
    /**
     * Eine Definition der Klasse Atomic, die die API
     * von std::atomic nachahmt, aber keine atomaren
     * Operationen durchführt. Diese Klasse ist für
     * die DISABLE_THREADS Präprozessor-Direktive
     * gedacht, die das Programm ohne Multithreading kompiliert.
     */
    template <typename T>
    class Atomic {
        private:
            T value;

        public:
            constexpr Atomic() : value() {}
            constexpr Atomic(T value) : value(value) {}

            constexpr T load() const {
                return value;
            }

            constexpr void store(T desired) {
                value = desired;
            }

            constexpr T exchange(T desired) {
                T old = value;
                value = desired;
                return old;
            }

            constexpr bool compare_exchange_strong(T& expected, T desired) {
                if (value == expected) {
                    value = desired;
                    return true;
                }

                expected = value;
                return false;
            }

            constexpr bool compare_exchange_weak(T& expected, T desired) {
                return compare_exchange_strong(expected, desired);
            }

            constexpr T fetch_add(T arg) {
                T old = value;
                value += arg;
                return old;
            }

            constexpr T fetch_sub(T arg) {
                T old = value;
                value -= arg;
                return old;
            }

            constexpr T fetch_and(T arg) {
                T old = value;
                value &= arg;
                return old;
            }

            constexpr T fetch_or(T arg) {
                T old = value;
                value |= arg;
                return old;
            }

            constexpr T fetch_xor(T arg) {
                T old = value;
                value ^= arg;
                return old;
            }

            constexpr T operator++() {
                return fetch_add(1) + 1;
            }

            constexpr T operator++(int) {
                return fetch_add(1);
            }

            constexpr T operator--() {
                return fetch_sub(1) - 1;
            }

            constexpr T operator--(int) {
                return fetch_sub(1);
            }

            constexpr T operator+=(T arg) {
                return fetch_add(arg) + arg;
            }

            constexpr T operator-=(T arg) {
                return fetch_sub(arg) - arg;
            }

            constexpr T operator&=(T arg) {
                return fetch_and(arg) & arg;
            }

            constexpr T operator|=(T arg) {
                return fetch_or(arg) | arg;
            }

            constexpr T operator^=(T arg) {
                return fetch_xor(arg) ^ arg;
            }

            constexpr operator T() const {
                return load();
            }

            constexpr T operator=(T desired) {
                store(desired);
                return desired;
            }

            constexpr T operator=(const Atomic<T>& desired) {
                store(desired.load());
                return desired.load();
            }
    };

    using AtomicBool = Atomic<bool>;
    using AtomicU64 = Atomic<uint64_t>;
    using AtomicSize = Atomic<size_t>;

#endif

#endif