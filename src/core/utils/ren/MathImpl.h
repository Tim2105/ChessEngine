#ifndef REN_MATH_IMPL_H
#define REN_MATH_IMPL_H

#include <algorithm>
#include <cstdint>
#include <immintrin.h>
#include <memory>

namespace REN {

    constexpr size_t REQUIRED_ALIGNMENT = 64;

    constexpr void __unsafe_add(float* __restrict dest, const float* __restrict src1, const float* __restrict src2, size_t n) {
        dest = std::assume_aligned<REQUIRED_ALIGNMENT>(dest);
        src1 = std::assume_aligned<REQUIRED_ALIGNMENT>(src1);
        src2 = std::assume_aligned<REQUIRED_ALIGNMENT>(src2);
        for(size_t i = 0; i < n; i++)
            dest[i] = src1[i] + src2[i];
    }

    constexpr void __unsafe_add_self(float* __restrict dest, const float* __restrict src, size_t n) {
        dest = std::assume_aligned<REQUIRED_ALIGNMENT>(dest);
        src = std::assume_aligned<REQUIRED_ALIGNMENT>(src);
        for(size_t i = 0; i < n; i++)
            dest[i] += src[i];
    }

    constexpr void __unsafe_sub(float* __restrict dest, const float* __restrict src1, const float* __restrict src2, size_t n) {
        dest = std::assume_aligned<REQUIRED_ALIGNMENT>(dest);
        src1 = std::assume_aligned<REQUIRED_ALIGNMENT>(src1);
        src2 = std::assume_aligned<REQUIRED_ALIGNMENT>(src2);
        for(size_t i = 0; i < n; i++)
            dest[i] = src1[i] - src2[i];
    }

    constexpr void __unsafe_sub_self(float* __restrict dest, const float* __restrict src, size_t n) {
        dest = std::assume_aligned<REQUIRED_ALIGNMENT>(dest);
        src = std::assume_aligned<REQUIRED_ALIGNMENT>(src);
        for(size_t i = 0; i < n; i++)
            dest[i] -= src[i];
    }

    constexpr void __unsafe_mul(float* __restrict dest, const float* __restrict src, float scalar, size_t n) {
        dest = std::assume_aligned<REQUIRED_ALIGNMENT>(dest);
        src = std::assume_aligned<REQUIRED_ALIGNMENT>(src);
        for(size_t i = 0; i < n; i++)
            dest[i] = src[i] * scalar;
    }

    constexpr void __unsafe_mul_self(float* __restrict dest, float scalar, size_t n) {
        dest = std::assume_aligned<REQUIRED_ALIGNMENT>(dest);
        for(size_t i = 0; i < n; i++)
            dest[i] *= scalar;
    }

    constexpr void __unsafe_div(float* __restrict dest, const float* __restrict src, float scalar, size_t n) {
        __unsafe_mul(dest, src, 1.0f / scalar, n);
    }

    constexpr void __unsafe_div_self(float* __restrict dest, float scalar, size_t n) {
        __unsafe_mul_self(dest, 1.0f / scalar, n);
    }

    constexpr void __unsafe_set_zero(float* __restrict dest, size_t n) {
        dest = std::assume_aligned<REQUIRED_ALIGNMENT>(dest);
        std::fill(dest, dest + n, 0.0f);
    }

    constexpr void __unsafe_set_constant(float* __restrict dest, float value, size_t n) {
        dest = std::assume_aligned<REQUIRED_ALIGNMENT>(dest);
        std::fill(dest, dest + n, value);
    }

    constexpr void __unsafe_copy(float* __restrict dest, const float* __restrict src, size_t n) {
        dest = std::assume_aligned<REQUIRED_ALIGNMENT>(dest);
        src = std::assume_aligned<REQUIRED_ALIGNMENT>(src);
        std::copy(src, src + n, dest);
    }

    inline float __unsafe_dot(const float* __restrict src1, const float* __restrict src2, size_t n) {
        src1 = std::assume_aligned<REQUIRED_ALIGNMENT>(src1);
        src2 = std::assume_aligned<REQUIRED_ALIGNMENT>(src2);

        float result = 0.0f;
        for(size_t i = 0; i < n; i++)
            result += src1[i] * src2[i];

        return result;
    }

    constexpr float __unsafe_norm_sq(const float* __restrict src, size_t n) {
        src = std::assume_aligned<REQUIRED_ALIGNMENT>(src);
        float result = 0.0f;
        for(size_t i = 0; i < n; i++)
            result += src[i] * src[i];
        return result;
    }

    inline void __unsafe_gemv(float* __restrict dest, const float* __restrict m, const float* __restrict v, size_t rows, size_t cols) {
        dest = std::assume_aligned<REQUIRED_ALIGNMENT>(dest);
        m = std::assume_aligned<REQUIRED_ALIGNMENT>(m);
        v = std::assume_aligned<REQUIRED_ALIGNMENT>(v);

        #ifdef __AVX512F__

        // AVX512-optimierte Implementierung
        for(size_t r = 0; r < rows; r++) {
            __m512 sum = _mm512_setzero_ps();
            for(size_t c = 0; c < cols; c += 16) {
                __m512 m_vec = _mm512_load_ps(&m[r * cols + c]);
                __m512 v_vec = _mm512_load_ps(&v[c]);
                sum = _mm512_fmadd_ps(m_vec, v_vec, sum);
            }
            dest[r] = _mm512_reduce_add_ps(sum);
        }

        #else

        for(size_t r = 0; r < rows; r++) {
            float sum = 0.0f;
            for(size_t c = 0; c < cols; c++)
                sum += m[r * cols + c] * v[c];
            dest[r] = sum;
        }

        #endif
    }

}

#endif