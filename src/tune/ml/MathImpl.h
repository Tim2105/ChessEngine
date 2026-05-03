#ifndef ML_MATH_IMPL_H
#define ML_MATH_IMPL_H

#include <algorithm>
#include <cstdint>
#include <immintrin.h>
#include <memory>
#include <new>

namespace ML {

    constexpr size_t REQUIRED_ALIGNMENT = 64;

    constexpr void __unsafe_add(float* __restrict dest, const float* __restrict src1, const float* __restrict src2, size_t n) {
        for(size_t i = 0; i < n; i++)
            dest[i] = src1[i] + src2[i];
    }

    constexpr void __unsafe_add_self(float* __restrict dest, const float* __restrict src, size_t n) {
        for(size_t i = 0; i < n; i++)
            dest[i] += src[i];
    }

    constexpr void __unsafe_sub(float* __restrict dest, const float* __restrict src1, const float* __restrict src2, size_t n) {
        for(size_t i = 0; i < n; i++)
            dest[i] = src1[i] - src2[i];
    }

    constexpr void __unsafe_sub_self(float* __restrict dest, const float* __restrict src, size_t n) {
        for(size_t i = 0; i < n; i++)
            dest[i] -= src[i];
    }

    constexpr void __unsafe_mul(float* __restrict dest, const float* __restrict src, float scalar, size_t n) {
        for(size_t i = 0; i < n; i++)
            dest[i] = src[i] * scalar;
    }

    constexpr void __unsafe_mul_self(float* __restrict dest, float scalar, size_t n) {
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
        std::fill(dest, dest + n, 0.0f);
    }

    constexpr void __unsafe_set_constant(float* __restrict dest, float value, size_t n) {
        std::fill(dest, dest + n, value);
    }

    constexpr void __unsafe_copy(float* __restrict dest, const float* __restrict src, size_t n) {
        std::copy(src, src + n, dest);
    }

    inline float __unsafe_dot(const float* __restrict src1, const float* __restrict src2, size_t n) {
        float result = 0.0f;
        for(size_t i = 0; i < n; i++)
            result += src1[i] * src2[i];

        return result;
    }

    constexpr float __unsafe_norm_sq(const float* __restrict src, size_t n) {
        float result = 0.0f;
        for(size_t i = 0; i < n; i++)
            result += src[i] * src[i];
        return result;
    }

    inline void __unsafe_gemv(float* __restrict dest, const float* __restrict m, const float* __restrict v, size_t rows, size_t cols) {
        #ifdef __AVX512F__

        // AVX512-optimierte Implementierung
        // Betrachte gleichzeitig 4 Zeilen * 16 Spalten

        size_t avx512Rows = rows - 4, avx512Cols = cols - 16;
        size_t r, c;
        for(r = 0; r <= avx512Rows; r += 4) {
            __m512 sum1 = _mm512_setzero_ps();
            __m512 sum2 = _mm512_setzero_ps();
            __m512 sum3 = _mm512_setzero_ps();
            __m512 sum4 = _mm512_setzero_ps();

            for(c = 0; c <= avx512Cols; c += 16) {
                __m512 m_vec = _mm512_loadu_ps(&m[r * cols + c]);
                __m512 v_vec = _mm512_loadu_ps(&v[c]);
                sum1 = _mm512_fmadd_ps(m_vec, v_vec, sum1);
                m_vec = _mm512_loadu_ps(&m[(r + 1) * cols + c]);
                sum2 = _mm512_fmadd_ps(m_vec, v_vec, sum2);
                m_vec = _mm512_loadu_ps(&m[(r + 2) * cols + c]);
                sum3 = _mm512_fmadd_ps(m_vec, v_vec, sum3);
                m_vec = _mm512_loadu_ps(&m[(r + 3) * cols + c]);
                sum4 = _mm512_fmadd_ps(m_vec, v_vec, sum4);
            }

            dest[r] = _mm512_reduce_add_ps(sum1);
            dest[r + 1] = _mm512_reduce_add_ps(sum2);
            dest[r + 2] = _mm512_reduce_add_ps(sum3);
            dest[r + 3] = _mm512_reduce_add_ps(sum4);

            // Restliche Spalten
            for(; c < cols; c++) {
                dest[r] += m[r * cols + c] * v[c];
                dest[r + 1] += m[(r + 1) * cols + c] * v[c];
                dest[r + 2] += m[(r + 2) * cols + c] * v[c];
                dest[r + 3] += m[(r + 3) * cols + c] * v[c];
            }
        }

        // Restliche Zeilen
        for(; r < rows; r++) {
            __m512 sum = _mm512_setzero_ps();
            for(c = 0; c <= avx512Cols; c += 16) {
                __m512 m_vec = _mm512_loadu_ps(&m[r * cols + c]);
                __m512 v_vec = _mm512_loadu_ps(&v[c]);
                sum = _mm512_fmadd_ps(m_vec, v_vec, sum);
            }
            dest[r] = _mm512_reduce_add_ps(sum);

            // Restliche Spalten
            for(; c < cols; c++)
                dest[r] += m[r * cols + c] * v[c];
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

    inline float __unsafe_spectral_radius(const float* __restrict m, size_t size, size_t maxIterations = 400, float tol = 1e-5f) {
        // Power-Iteration zur Berechnung der Spektralradius
        float* __restrict b_k = new (std::align_val_t(REQUIRED_ALIGNMENT)) float[size];
        for(size_t i = 0; i < size; i++)
            b_k[i] = 1.0f; // Startvektor

        float* __restrict b_k1 = new (std::align_val_t(REQUIRED_ALIGNMENT)) float[size];
        float eigenvalue = 0.0f;
        for(size_t iter = 0; iter < maxIterations; iter++) {
            // Matrix-Vektor-Produkt: b_k1 = m * b_k
            __unsafe_gemv(b_k1, m, b_k, size, size);

            // Normalisieren
            float invNorm = 1.0f / std::sqrt(__unsafe_dot(b_k1, b_k1, size));
            for(size_t i = 0; i < size; i++)
                b_k[i] = b_k1[i] * invNorm;

            // Rayleigh-Quotient als Eigenwertschätzung
            float new_eigenvalue = __unsafe_dot(b_k, b_k1, size);

            // Konvergenz prüfen
            if(std::abs(new_eigenvalue - eigenvalue) < tol) {
                eigenvalue = new_eigenvalue;
                break;
            }

            eigenvalue = new_eigenvalue;
        }

        delete[] b_k;
        delete[] b_k1;

        return std::abs(eigenvalue);
    }

}

#endif