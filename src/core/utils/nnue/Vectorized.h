#ifndef VECTORIZED_H
#define VECTORIZED_H

/**
 * Diese Datei enthält Hilfsfunktionen für die
 * vektorisierte Inferenz des NNUE-Netzwerks.
 * 
 * Jede Funktion ist dreifach implementiert:
 * - Mit AVX2-Instruktionen (moderne x86-CPUs)
 * - Mit SSE4.1-Instruktionen (ältere x86-CPUs und WebAssembly)
 * - Mit C++-Code, d.h. ohne manuelle Vektorisierung (alle Plattformen)
 * 
 * Außerdem wird in dieser Datei die Konstante REQUIRED_ALIGNMENT
 * definiert, die die Ausrichtung der Daten für die vektorisierten
 * Funktionen angibt. Diese ist abhängig von der verwendeten
 * Instruktionssatz-Erweiterung.
 * Ein Aufruf der Funktionen mit nicht-ausgerichteten Daten
 * führt zu undefiniertem Verhalten (in der Regel zu einem
 * Segmentation Fault!).
 */

#include <stdint.h>

constexpr size_t CACHE_LINE_ALIGNMENT = 64;

inline void add32i8(const int8_t* src, int8_t* dst) noexcept;
inline void add16i16(const int16_t* src, int16_t* dst) noexcept;
inline void add8i32(const int32_t* src, int32_t* dst) noexcept;
inline void sub32i8(const int8_t* src, int8_t* dst) noexcept;
inline void sub16i16(const int16_t* src, int16_t* dst) noexcept;
inline void sub8i32(const int32_t* src, int32_t* dst) noexcept;
inline void clippedReLU32i16(const int16_t* src, int8_t* dst) noexcept;
inline void scaledClippedReLU32i32(const int32_t* src, int8_t* dst) noexcept;

template <size_t IN_SIZE, size_t OUT_SIZE>
inline void linearI8ToI32(const int8_t* in, const int8_t* weights, const int32_t* biases, int32_t* out) noexcept;

#if defined (__AVX2__)

/**
 * Beginn der AVX2-Implementierungen.
 */

#include <immintrin.h>

constexpr size_t REQUIRED_ALIGNMENT = 32;

inline void copy32i8(const int8_t* src, int8_t* dst) noexcept {
    __m256i srcVec = _mm256_load_si256((__m256i*)src);
    _mm256_store_si256((__m256i*)dst, srcVec);
}

inline void copy16i16(const int16_t* src, int16_t* dst) noexcept {
    __m256i srcVec = _mm256_load_si256((__m256i*)src);
    _mm256_store_si256((__m256i*)dst, srcVec);
}

inline void copy8i32(const int32_t* src, int32_t* dst) noexcept {
    __m256i srcVec = _mm256_load_si256((__m256i*)src);
    _mm256_store_si256((__m256i*)dst, srcVec);
}

inline void add32i8(const int8_t* src, int8_t* dst) noexcept {
    __m256i srcVec = _mm256_load_si256((__m256i*)src);
    __m256i dstVec = _mm256_load_si256((__m256i*)dst);
    __m256i resVec = _mm256_add_epi8(srcVec, dstVec);
    _mm256_store_si256((__m256i*)dst, resVec);
}

inline void add16i16(const int16_t* src, int16_t* dst) noexcept {
    __m256i srcVec = _mm256_load_si256((__m256i*)src);
    __m256i dstVec = _mm256_load_si256((__m256i*)dst);
    __m256i resVec = _mm256_add_epi16(srcVec, dstVec);
    _mm256_store_si256((__m256i*)dst, resVec);
}

inline void add8i32(const int32_t* src, int32_t* dst) noexcept {
    __m256i srcVec = _mm256_load_si256((__m256i*)src);
    __m256i dstVec = _mm256_load_si256((__m256i*)dst);
    __m256i resVec = _mm256_add_epi32(srcVec, dstVec);
    _mm256_store_si256((__m256i*)dst, resVec);
}

inline void sub32i8(const int8_t* src, int8_t* dst) noexcept {
    __m256i srcVec = _mm256_load_si256((__m256i*)src);
    __m256i dstVec = _mm256_load_si256((__m256i*)dst);
    __m256i resVec = _mm256_sub_epi8(dstVec, srcVec);
    _mm256_store_si256((__m256i*)dst, resVec);
}

inline void sub16i16(const int16_t* src, int16_t* dst) noexcept {
    __m256i srcVec = _mm256_load_si256((__m256i*)src);
    __m256i dstVec = _mm256_load_si256((__m256i*)dst);
    __m256i resVec = _mm256_sub_epi16(dstVec, srcVec);
    _mm256_store_si256((__m256i*)dst, resVec);
}

inline void sub8i32(const int32_t* src, int32_t* dst) noexcept {
    __m256i srcVec = _mm256_load_si256((__m256i*)src);
    __m256i dstVec = _mm256_load_si256((__m256i*)dst);
    __m256i resVec = _mm256_sub_epi32(dstVec, srcVec);
    _mm256_store_si256((__m256i*)dst, resVec);
}

inline void clippedReLU32i16(const int16_t* src, int8_t* dst) noexcept {
    __m256i srcVec1 = _mm256_load_si256((__m256i*)src);
    __m256i srcVec2 = _mm256_load_si256((__m256i*)(src + 16));
    __m256i resVec = _mm256_permute4x64_epi64(
        _mm256_max_epi8(
            _mm256_packs_epi16(srcVec1, srcVec2),
            _mm256_setzero_si256()
        ),
        0b11011000
    );

    _mm256_store_si256((__m256i*)dst, resVec);
}

inline void scaledClippedReLU32i32(const int32_t* src, int8_t* dst) noexcept {
    __m256i srcVec1 = _mm256_load_si256((__m256i*)src);
    __m256i srcVec2 = _mm256_load_si256((__m256i*)(src + 8));
    __m256i srcVec3 = _mm256_load_si256((__m256i*)(src + 16));
    __m256i srcVec4 = _mm256_load_si256((__m256i*)(src + 24));

    srcVec1 = _mm256_srai_epi32(srcVec1, 6);
    srcVec2 = _mm256_srai_epi32(srcVec2, 6);
    srcVec3 = _mm256_srai_epi32(srcVec3, 6);
    srcVec4 = _mm256_srai_epi32(srcVec4, 6);

    __m256i inVec1 = _mm256_packs_epi32(
        srcVec1,
        srcVec2
    );

    __m256i inVec2 = _mm256_packs_epi32(
        srcVec3,
        srcVec4
    );

    __m256i resVec = _mm256_permutevar8x32_epi32(
        _mm256_max_epi8(
            _mm256_packs_epi16(inVec1, inVec2),
            _mm256_setzero_si256()
        ),
        _mm256_set_epi32(7, 3, 6, 2, 5, 1, 4, 0)
    );

    _mm256_store_si256((__m256i*)dst, resVec);
}

/**
 * Matrixmultiplikation mit Ansatz aus https://gcc.gnu.org/bugzilla/attachment.cgi?id=51238
 */

inline void m256_add_dpbusd_epi32(__m256i& outVec, __m256i a, __m256i b) {
    #if defined (USE_VNNI)
        acc = _mm256_dpbusd_epi32(acc, a, b);
    #else
        __m256i resVec = _mm256_maddubs_epi16(a, b);
        __m256i oneVec = _mm256_set1_epi16(1);
        resVec = _mm256_madd_epi16(resVec, oneVec);
        outVec = _mm256_add_epi32(outVec, resVec);
    #endif
}

inline __m128i m256_haddx4(__m256i a, __m256i b, __m256i c, __m256i d, __m128i biasVec) {
    a = _mm256_hadd_epi32(a, b);
    c = _mm256_hadd_epi32(c, d);

    a = _mm256_hadd_epi32(a, c);

    __m128i sum128lo = _mm256_castsi256_si128(a);
    __m128i sum128hi = _mm256_extracti128_si256(a, 1);

    return _mm_add_epi32(_mm_add_epi32(sum128lo, sum128hi), biasVec);
}

template <size_t IN_SIZE, size_t OUT_SIZE>
inline void linearI8ToI32(const int8_t* in, const int8_t* weights,
                           const int32_t* biases, int32_t* out)
                           noexcept requires(IN_SIZE % 32 == 0 && OUT_SIZE % 4 == 0) {

    for(size_t i = 0; i < OUT_SIZE; i += 4) {
        __m256i a = _mm256_setzero_si256();
        __m256i b = _mm256_setzero_si256();
        __m256i c = _mm256_setzero_si256();
        __m256i d = _mm256_setzero_si256();

        for(size_t j = 0; j < IN_SIZE; j += 32) {
            __m256i inVec = _mm256_load_si256((__m256i*)(in + j));

            m256_add_dpbusd_epi32(a, inVec, _mm256_load_si256((__m256i*)(weights + (i + 0) * IN_SIZE + j)));
            m256_add_dpbusd_epi32(b, inVec, _mm256_load_si256((__m256i*)(weights + (i + 1) * IN_SIZE + j)));
            m256_add_dpbusd_epi32(c, inVec, _mm256_load_si256((__m256i*)(weights + (i + 2) * IN_SIZE + j)));
            m256_add_dpbusd_epi32(d, inVec, _mm256_load_si256((__m256i*)(weights + (i + 3) * IN_SIZE + j)));
        }

        __m128i biasVec = _mm_load_si128((__m128i*)(biases + i));

        __m128i resVec = m256_haddx4(a, b, c, d, biasVec);

        _mm_store_si128((__m128i*)(out + i), resVec);
    }
}

template <size_t IN_SIZE, size_t OUT_SIZE>
inline void linearI8ToI32(const int8_t* in, const int8_t* weights,
                           const int32_t* biases, int32_t* out)
                           noexcept requires(IN_SIZE % 32 == 0 && OUT_SIZE % 4 != 0) {

    for(size_t i = 0; i < OUT_SIZE; i++) {
        __m256i a = _mm256_setzero_si256();

        for(size_t j = 0; j < IN_SIZE; j += 32) {
            __m256i inVec = _mm256_load_si256((__m256i*)(in + j));

            m256_add_dpbusd_epi32(a, inVec, _mm256_load_si256((__m256i*)(weights + i * IN_SIZE + j)));
        }

        __m128i biasVec = _mm_load_si128((__m128i*)(biases + i));

        __m128i resVec = m256_haddx4(a, a, a, a, biasVec);

        _mm_store_si128((__m128i*)(out + i), resVec);
    }
}

#elif defined (__SSE4_1__)

/**
 * Beginn der SSE4.1-Implementierungen.
 * Die verwendeten Instruktionen können (bis auf 2 Ausnahmen) alle
 * auf WebAssembly SIMD-Instruktionen abgebildet werden:
 * 
 * _mm_load_si128 -> wasm_v128_load
 * _mm_store_si128 -> wasm_v128_store
 * _mm_add_epi8 -> wasm_i8x16_add
 * _mm_add_epi16 -> wasm_i16x8_add
 * _mm_add_epi32 -> wasm_i32x4_add
 * _mm_sub_epi8 -> wasm_i8x16_sub
 * _mm_sub_epi16 -> wasm_i16x8_sub
 * _mm_sub_epi32 -> wasm_i32x4_sub
 * _mm_max_epi8 -> wasm_i8x16_max
 * _mm_packs_epi16 -> wasm_i8x16_narrow_i16x8
 * _mm_packs_epi32 -> wasm_i16x8_narrow_i32x4
 * _mm_setzero_si128 -> wasm_i64x2_const
 * _mm_srai_epi32 -> wasm_i32x4_shr
 * _mm_maddubs_epi16 -> 4 shifts, 2 multiplies, 1 and, 1 const (muss emuliert werden)
 * _mm_set1_epi16 -> wasm_i16x8_splat
 * _mm_madd_epi16 -> wasm_i32x4_dot_i16x8
 * _mm_hadd_epi32 -> 1 add, 2 shuffles (muss emuliert werden)
 */

#include <smmintrin.h>

constexpr size_t REQUIRED_ALIGNMENT = 16;

inline void copy32i8(const int8_t* src, int8_t* dst) noexcept {
    __m128i srcVec1 = _mm_load_si128((__m128i*)src);
    __m128i srcVec2 = _mm_load_si128((__m128i*)(src + 16));
    _mm_store_si128((__m128i*)dst, srcVec1);
    _mm_store_si128((__m128i*)(dst + 16), srcVec2);
}

inline void copy16i16(const int16_t* src, int16_t* dst) noexcept {
    __m128i srcVec1 = _mm_load_si128((__m128i*)src);
    __m128i srcVec2 = _mm_load_si128((__m128i*)(src + 8));
    _mm_store_si128((__m128i*)dst, srcVec1);
    _mm_store_si128((__m128i*)(dst + 8), srcVec2);
}

inline void copy8i32(const int32_t* src, int32_t* dst) noexcept {
    __m128i srcVec1 = _mm_load_si128((__m128i*)src);
    __m128i srcVec2 = _mm_load_si128((__m128i*)(src + 4));
    _mm_store_si128((__m128i*)dst, srcVec1);
    _mm_store_si128((__m128i*)(dst + 4), srcVec2);
}

inline void add32i8(const int8_t* src, int8_t* dst) noexcept {
    __m128i srcVec1 = _mm_load_si128((__m128i*)src);
    __m128i srcVec2 = _mm_load_si128((__m128i*)(src + 16));
    __m128i dstVec1 = _mm_load_si128((__m128i*)dst);
    __m128i dstVec2 = _mm_load_si128((__m128i*)(dst + 16));
    __m128i resVec1 = _mm_add_epi8(srcVec1, dstVec1);
    __m128i resVec2 = _mm_add_epi8(srcVec2, dstVec2);
    _mm_store_si128((__m128i*)dst, resVec1);
    _mm_store_si128((__m128i*)(dst + 16), resVec2);
}

inline void add16i16(const int16_t* src, int16_t* dst) noexcept {
    __m128i srcVec1 = _mm_load_si128((__m128i*)src);
    __m128i srcVec2 = _mm_load_si128((__m128i*)(src + 8));
    __m128i dstVec1 = _mm_load_si128((__m128i*)dst);
    __m128i dstVec2 = _mm_load_si128((__m128i*)(dst + 8));
    __m128i resVec1 = _mm_add_epi16(srcVec1, dstVec1);
    __m128i resVec2 = _mm_add_epi16(srcVec2, dstVec2);
    _mm_store_si128((__m128i*)dst, resVec1);
    _mm_store_si128((__m128i*)(dst + 8), resVec2);
}

inline void add8i32(const int32_t* src, int32_t* dst) noexcept {
    __m128i srcVec1 = _mm_load_si128((__m128i*)src);
    __m128i srcVec2 = _mm_load_si128((__m128i*)(src + 4));
    __m128i dstVec1 = _mm_load_si128((__m128i*)dst);
    __m128i dstVec2 = _mm_load_si128((__m128i*)(dst + 4));
    __m128i resVec1 = _mm_add_epi32(srcVec1, dstVec1);
    __m128i resVec2 = _mm_add_epi32(srcVec2, dstVec2);
    _mm_store_si128((__m128i*)dst, resVec1);
    _mm_store_si128((__m128i*)(dst + 4), resVec2);
}

inline void sub32i8(const int8_t* src, int8_t* dst) noexcept {
    __m128i srcVec1 = _mm_load_si128((__m128i*)src);
    __m128i srcVec2 = _mm_load_si128((__m128i*)(src + 16));
    __m128i dstVec1 = _mm_load_si128((__m128i*)dst);
    __m128i dstVec2 = _mm_load_si128((__m128i*)(dst + 16));
    __m128i resVec1 = _mm_sub_epi8(dstVec1, srcVec1);
    __m128i resVec2 = _mm_sub_epi8(dstVec2, srcVec2);
    _mm_store_si128((__m128i*)dst, resVec1);
    _mm_store_si128((__m128i*)(dst + 16), resVec2);
}

inline void sub16i16(const int16_t* src, int16_t* dst) noexcept {
    __m128i srcVec1 = _mm_load_si128((__m128i*)src);
    __m128i srcVec2 = _mm_load_si128((__m128i*)(src + 8));
    __m128i dstVec1 = _mm_load_si128((__m128i*)dst);
    __m128i dstVec2 = _mm_load_si128((__m128i*)(dst + 8));
    __m128i resVec1 = _mm_sub_epi16(dstVec1, srcVec1);
    __m128i resVec2 = _mm_sub_epi16(dstVec2, srcVec2);
    _mm_store_si128((__m128i*)dst, resVec1);
    _mm_store_si128((__m128i*)(dst + 8), resVec2);
}

inline void sub8i32(const int32_t* src, int32_t* dst) noexcept {
    __m128i srcVec1 = _mm_load_si128((__m128i*)src);
    __m128i srcVec2 = _mm_load_si128((__m128i*)(src + 4));
    __m128i dstVec1 = _mm_load_si128((__m128i*)dst);
    __m128i dstVec2 = _mm_load_si128((__m128i*)(dst + 4));
    __m128i resVec1 = _mm_sub_epi32(dstVec1, srcVec1);
    __m128i resVec2 = _mm_sub_epi32(dstVec2, srcVec2);
    _mm_store_si128((__m128i*)dst, resVec1);
    _mm_store_si128((__m128i*)(dst + 4), resVec2);
}

inline void clippedReLU32i16(const int16_t* src, int8_t* dst) noexcept {
    __m128i srcVec1 = _mm_load_si128((__m128i*)src);
    __m128i srcVec2 = _mm_load_si128((__m128i*)(src + 8));
    __m128i srcVec3 = _mm_load_si128((__m128i*)(src + 16));
    __m128i srcVec4 = _mm_load_si128((__m128i*)(src + 24));

    __m128i resVec1 = _mm_max_epi8(
            _mm_packs_epi16(srcVec1, srcVec2),
            _mm_setzero_si128()
        );

    __m128i resVec2 = _mm_max_epi8(
            _mm_packs_epi16(srcVec3, srcVec4),
            _mm_setzero_si128()
        );

    _mm_store_si128((__m128i*)dst, resVec1);
    _mm_store_si128((__m128i*)(dst + 16), resVec2);
}

inline void scaledClippedReLU32i32(const int32_t* src, int8_t* dst) noexcept {
    __m128i srcVec1 = _mm_load_si128((__m128i*)src);
    __m128i srcVec2 = _mm_load_si128((__m128i*)(src + 4));
    __m128i srcVec3 = _mm_load_si128((__m128i*)(src + 8));
    __m128i srcVec4 = _mm_load_si128((__m128i*)(src + 12));
    __m128i srcVec5 = _mm_load_si128((__m128i*)(src + 16));
    __m128i srcVec6 = _mm_load_si128((__m128i*)(src + 20));
    __m128i srcVec7 = _mm_load_si128((__m128i*)(src + 24));
    __m128i srcVec8 = _mm_load_si128((__m128i*)(src + 28));

    srcVec1 = _mm_srai_epi32(srcVec1, 6);
    srcVec2 = _mm_srai_epi32(srcVec2, 6);
    srcVec3 = _mm_srai_epi32(srcVec3, 6);
    srcVec4 = _mm_srai_epi32(srcVec4, 6);
    srcVec5 = _mm_srai_epi32(srcVec5, 6);
    srcVec6 = _mm_srai_epi32(srcVec6, 6);
    srcVec7 = _mm_srai_epi32(srcVec7, 6);
    srcVec8 = _mm_srai_epi32(srcVec8, 6);

    __m128i inVec1 = _mm_packs_epi32(
        srcVec1,
        srcVec2
    );

    __m128i inVec2 = _mm_packs_epi32(
        srcVec3,
        srcVec4
    );

    __m128i inVec3 = _mm_packs_epi32(
        srcVec5,
        srcVec6
    );

    __m128i inVec4 = _mm_packs_epi32(
        srcVec7,
        srcVec8
    );

    __m128i resVec1 = _mm_max_epi8(
        _mm_packs_epi16(inVec1, inVec2),
        _mm_setzero_si128()
    );

    __m128i resVec2 = _mm_max_epi8(
        _mm_packs_epi16(inVec3, inVec4),
        _mm_setzero_si128()
    );

    _mm_store_si128((__m128i*)dst, resVec1);
    _mm_store_si128((__m128i*)(dst + 16), resVec2);
}

inline void m128_add_dpbusd_epi32(__m128i& outVec, __m128i a, __m128i b) {
    __m128i resVec = _mm_maddubs_epi16(a, b);
    __m128i oneVec = _mm_set1_epi16(1);
    resVec = _mm_madd_epi16(resVec, oneVec);
    outVec = _mm_add_epi32(outVec, resVec);
}

inline __m128i m128_haddx4(__m128i a, __m128i b, __m128i c, __m128i d, __m128i biasVec) {
    a = _mm_hadd_epi32(a, b);
    c = _mm_hadd_epi32(c, d);

    a = _mm_hadd_epi32(a, c);

    return _mm_add_epi32(a, biasVec);
}

template <size_t IN_SIZE, size_t OUT_SIZE>
inline void linearI8ToI32(const int8_t* in, const int8_t* weights,
                           const int32_t* biases, int32_t* out)
                           noexcept requires(IN_SIZE % 16 == 0 && OUT_SIZE % 4 == 0) {

    for(size_t i = 0; i < OUT_SIZE; i += 4) {
        __m128i a = _mm_setzero_si128();
        __m128i b = _mm_setzero_si128();
        __m128i c = _mm_setzero_si128();
        __m128i d = _mm_setzero_si128();

        for(size_t j = 0; j < IN_SIZE; j += 16) {
            __m128i inVec = _mm_load_si128((__m128i*)(in + j));

            m128_add_dpbusd_epi32(a, inVec, _mm_load_si128((__m128i*)(weights + (i + 0) * IN_SIZE + j)));
            m128_add_dpbusd_epi32(b, inVec, _mm_load_si128((__m128i*)(weights + (i + 1) * IN_SIZE + j)));
            m128_add_dpbusd_epi32(c, inVec, _mm_load_si128((__m128i*)(weights + (i + 2) * IN_SIZE + j)));
            m128_add_dpbusd_epi32(d, inVec, _mm_load_si128((__m128i*)(weights + (i + 3) * IN_SIZE + j)));
        }

        __m128i biasVec = _mm_load_si128((__m128i*)(biases + i));

        __m128i resVec = m128_haddx4(a, b, c, d, biasVec);

        _mm_store_si128((__m128i*)(out + i), resVec);
    }
}

template <size_t IN_SIZE, size_t OUT_SIZE>
inline void linearI8ToI32(const int8_t* in, const int8_t* weights,
                           const int32_t* biases, int32_t* out)
                           noexcept requires(IN_SIZE % 16 == 0 && OUT_SIZE % 4 != 0) {

    for(size_t i = 0; i < OUT_SIZE; i++) {
        __m128i a = _mm_setzero_si128();

        for(size_t j = 0; j < IN_SIZE; j += 16) {
            __m128i inVec = _mm_load_si128((__m128i*)(in + j));

            m128_add_dpbusd_epi32(a, inVec, _mm_load_si128((__m128i*)(weights + i * IN_SIZE + j)));
        }

        __m128i biasVec = _mm_load_si128((__m128i*)(biases + i));

        __m128i resVec = m128_haddx4(a, a, a, a, biasVec);

        _mm_store_si128((__m128i*)(out + i), resVec);
    }
}

#else

/**
 * Beginn der C++-Implementierungen.
 * Wenn wir hier landen (z.B. weil wir für ARM kompilieren),
 * hoffen wir, dass die automatische Vektorisierung des Compilers
 * die Geschwindigkeit der Funktionen erhöht.
 */

#include <algorithm>

constexpr size_t REQUIRED_ALIGNMENT = 1;

inline void copy32i8(const int8_t* src, int8_t* dst) noexcept {
    for(size_t i = 0; i < 32; i++)
        dst[i] = src[i];
}

inline void copy16i16(const int16_t* src, int16_t* dst) noexcept {
    for(size_t i = 0; i < 16; i++)
        dst[i] = src[i];
}

inline void copy8i32(const int32_t* src, int32_t* dst) noexcept {
    for(size_t i = 0; i < 8; i++)
        dst[i] = src[i];
}

inline void add32i8(const int8_t* src, int8_t* dst) noexcept {
    for(size_t i = 0; i < 32; i++)
        dst[i] += src[i];
}

inline void add16i16(const int16_t* src, int16_t* dst) noexcept {
    for(size_t i = 0; i < 16; i++)
        dst[i] += src[i];
}

inline void add8i32(const int32_t* src, int32_t* dst) noexcept {
    for(size_t i = 0; i < 8; i++)
        dst[i] += src[i];
}

inline void sub32i8(const int8_t* src, int8_t* dst) noexcept {
    for(size_t i = 0; i < 32; i++)
        dst[i] -= src[i];
}

inline void sub16i16(const int16_t* src, int16_t* dst) noexcept {
    for(size_t i = 0; i < 16; i++)
        dst[i] -= src[i];
}

inline void sub8i32(const int32_t* src, int32_t* dst) noexcept {
    for(size_t i = 0; i < 8; i++)
        dst[i] -= src[i];
}

inline void clippedReLU32i16(const int16_t* src, int8_t* dst) noexcept {
    for(size_t i = 0; i < 32; i++)
        dst[i] = std::clamp(src[i], (int16_t)0, (int16_t)127);
}

inline void scaledClippedReLU32i32(const int32_t* src, int8_t* dst) noexcept {
    for(size_t i = 0; i < 32; i++)
        dst[i] = std::clamp(src[i] >> 6, 0, 127);
}

template <size_t IN_SIZE, size_t OUT_SIZE>
inline void linearI8ToI32(const int8_t* in, const int8_t* weights,
                           const int32_t* biases, int32_t* out) noexcept {

    // Setze Biases
    std::copy(biases, biases + OUT_SIZE, out);

    // Skalarprodukte
    for(size_t i = 0; i < OUT_SIZE; i++) {
        for(size_t j = 0; j < IN_SIZE; j++)
            out[i] += in[j] * weights[i * IN_SIZE + j];
    }
}

#endif

#endif