#ifndef REN_ALIGNED_ALLOCATOR_H
#define REN_ALIGNED_ALLOCATOR_H

#include <cstddef>
#include <limits>
#include <new>
#include <type_traits>

template <typename T, std::size_t Alignment>
class AlignedAllocator {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using is_always_equal = std::true_type;
    using propagate_on_container_move_assignment = std::true_type;

    constexpr AlignedAllocator() noexcept = default;

    template <typename U>
    constexpr AlignedAllocator(const AlignedAllocator<U, Alignment>&) noexcept {}

    T* allocate(std::size_t n) {
        static_assert((Alignment & (Alignment - 1)) == 0, "Alignment must be a power of two");
        static_assert(Alignment >= alignof(T), "Alignment must be >= alignof(T)");

        if (n > max_size()) {
            throw std::bad_array_new_length();
        }

        return static_cast<T*>(
            ::operator new(n * sizeof(T), std::align_val_t{Alignment})
        );
    }

    void deallocate(T* p, std::size_t) noexcept {
        ::operator delete(p, std::align_val_t{Alignment});
    }

    constexpr std::size_t max_size() const noexcept {
        return std::numeric_limits<std::size_t>::max() / sizeof(T);
    }

    template <typename U>
    struct rebind {
        using other = AlignedAllocator<U, Alignment>;
    };
};

template <typename T1, std::size_t A1, typename T2, std::size_t A2>
constexpr bool operator==(const AlignedAllocator<T1, A1>&, const AlignedAllocator<T2, A2>&) noexcept {
    return A1 == A2;
}

template <typename T1, std::size_t A1, typename T2, std::size_t A2>
constexpr bool operator!=(const AlignedAllocator<T1, A1>& a, const AlignedAllocator<T2, A2>& b) noexcept {
    return !(a == b);
}

#endif