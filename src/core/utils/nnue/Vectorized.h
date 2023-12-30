#ifndef VECTORIZED_H
#define VECTORIZED_H

#include <stdint.h>

inline void assign32i8(const int8_t* src, int8_t* dst);
inline void assign16i16(const int16_t* src, int16_t* dst);
inline void assign8i32(const int32_t* src, int32_t* dst);
inline void add32i8(const int8_t* src, int8_t* dst);
inline void add16i16(const int16_t* src, int16_t* dst);
inline void add8i32(const int32_t* src, int32_t* dst);
inline void sub32i8(const int8_t* src, int8_t* dst);
inline void sub16i16(const int16_t* src, int16_t* dst);
inline void sub8i32(const int32_t* src, int32_t* dst);
inline void clippedReLU32i16(const int16_t* src, int8_t* dst);
inline void scaledClippedReLU32i32(const int32_t* src, int8_t* dst);

#if defined(__AVX2__)

#include <immintrin.h>

inline void assign32i8(const int8_t* src, int8_t* dst) {
    __m256i srcVec = _mm256_load_si256((__m256i*)src);
    _mm256_store_si256((__m256i*)dst, srcVec);
}

inline void assign16i16(const int16_t* src, int16_t* dst) {
    __m256i srcVec = _mm256_load_si256((__m256i*)src);
    _mm256_store_si256((__m256i*)dst, srcVec);
}

inline void assign8i32(const int32_t* src, int32_t* dst) {
    __m256i srcVec = _mm256_load_si256((__m256i*)src);
    _mm256_store_si256((__m256i*)dst, srcVec);
}

inline void add32i8(const int8_t* src, int8_t* dst) {
    __m256i srcVec = _mm256_load_si256((__m256i*)src);
    __m256i dstVec = _mm256_load_si256((__m256i*)dst);
    __m256i resVec = _mm256_add_epi8(srcVec, dstVec);
    _mm256_store_si256((__m256i*)dst, resVec);
}

inline void add16i16(const int16_t* src, int16_t* dst) {
    __m256i srcVec = _mm256_load_si256((__m256i*)src);
    __m256i dstVec = _mm256_load_si256((__m256i*)dst);
    __m256i resVec = _mm256_add_epi16(srcVec, dstVec);
    _mm256_store_si256((__m256i*)dst, resVec);
}

inline void add8i32(const int32_t* src, int32_t* dst) {
    __m256i srcVec = _mm256_load_si256((__m256i*)src);
    __m256i dstVec = _mm256_load_si256((__m256i*)dst);
    __m256i resVec = _mm256_add_epi32(srcVec, dstVec);
    _mm256_store_si256((__m256i*)dst, resVec);
}

inline void sub32i8(const int8_t* src, int8_t* dst) {
    __m256i srcVec = _mm256_load_si256((__m256i*)src);
    __m256i dstVec = _mm256_load_si256((__m256i*)dst);
    __m256i resVec = _mm256_sub_epi8(dstVec, srcVec);
    _mm256_store_si256((__m256i*)dst, resVec);
}

inline void sub16i16(const int16_t* src, int16_t* dst) {
    __m256i srcVec = _mm256_load_si256((__m256i*)src);
    __m256i dstVec = _mm256_load_si256((__m256i*)dst);
    __m256i resVec = _mm256_sub_epi16(dstVec, srcVec);
    _mm256_store_si256((__m256i*)dst, resVec);
}

inline void sub8i32(const int32_t* src, int32_t* dst) {
    __m256i srcVec = _mm256_load_si256((__m256i*)src);
    __m256i dstVec = _mm256_load_si256((__m256i*)dst);
    __m256i resVec = _mm256_sub_epi32(dstVec, srcVec);
    _mm256_store_si256((__m256i*)dst, resVec);
}

inline void clippedReLU32i16(const int16_t* src, int8_t* dst) {
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

inline void scaledClippedReLU32i32(const int32_t* src, int8_t* dst) {
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

#elif defined(__SSE4_1__)

#include <immintrin.h>

inline void assign32i8(const int8_t* src, int8_t* dst) {
    __m128i srcVec1 = _mm_load_si128((__m128i*)src);
    __m128i srcVec2 = _mm_load_si128((__m128i*)(src + 16));
    _mm_store_si128((__m128i*)dst, srcVec1);
    _mm_store_si128((__m128i*)(dst + 16), srcVec2);
}

inline void assign16i16(const int16_t* src, int16_t* dst) {
    __m128i srcVec1 = _mm_load_si128((__m128i*)src);
    __m128i srcVec2 = _mm_load_si128((__m128i*)(src + 8));
    _mm_store_si128((__m128i*)dst, srcVec1);
    _mm_store_si128((__m128i*)(dst + 8), srcVec2);
}

inline void assign8i32(const int32_t* src, int32_t* dst) {
    __m128i srcVec1 = _mm_load_si128((__m128i*)src);
    __m128i srcVec2 = _mm_load_si128((__m128i*)(src + 4));
    _mm_store_si128((__m128i*)dst, srcVec1);
    _mm_store_si128((__m128i*)(dst + 4), srcVec2);
}

inline void add32i8(const int8_t* src, int8_t* dst) {
    __m128i srcVec1 = _mm_load_si128((__m128i*)src);
    __m128i srcVec2 = _mm_load_si128((__m128i*)(src + 16));
    __m128i dstVec1 = _mm_load_si128((__m128i*)dst);
    __m128i dstVec2 = _mm_load_si128((__m128i*)(dst + 16));
    __m128i resVec1 = _mm_add_epi8(srcVec1, dstVec1);
    __m128i resVec2 = _mm_add_epi8(srcVec2, dstVec2);
    _mm_store_si128((__m128i*)dst, resVec1);
    _mm_store_si128((__m128i*)(dst + 16), resVec2);
}

inline void add16i16(const int16_t* src, int16_t* dst) {
    __m128i srcVec1 = _mm_load_si128((__m128i*)src);
    __m128i srcVec2 = _mm_load_si128((__m128i*)(src + 8));
    __m128i dstVec1 = _mm_load_si128((__m128i*)dst);
    __m128i dstVec2 = _mm_load_si128((__m128i*)(dst + 8));
    __m128i resVec1 = _mm_add_epi16(srcVec1, dstVec1);
    __m128i resVec2 = _mm_add_epi16(srcVec2, dstVec2);
    _mm_store_si128((__m128i*)dst, resVec1);
    _mm_store_si128((__m128i*)(dst + 8), resVec2);
}

inline void add8i32(const int32_t* src, int32_t* dst) {
    __m128i srcVec1 = _mm_load_si128((__m128i*)src);
    __m128i srcVec2 = _mm_load_si128((__m128i*)(src + 4));
    __m128i dstVec1 = _mm_load_si128((__m128i*)dst);
    __m128i dstVec2 = _mm_load_si128((__m128i*)(dst + 4));
    __m128i resVec1 = _mm_add_epi32(srcVec1, dstVec1);
    __m128i resVec2 = _mm_add_epi32(srcVec2, dstVec2);
    _mm_store_si128((__m128i*)dst, resVec1);
    _mm_store_si128((__m128i*)(dst + 4), resVec2);
}

inline void sub32i8(const int8_t* src, int8_t* dst) {
    __m128i srcVec1 = _mm_load_si128((__m128i*)src);
    __m128i srcVec2 = _mm_load_si128((__m128i*)(src + 16));
    __m128i dstVec1 = _mm_load_si128((__m128i*)dst);
    __m128i dstVec2 = _mm_load_si128((__m128i*)(dst + 16));
    __m128i resVec1 = _mm_sub_epi8(dstVec1, srcVec1);
    __m128i resVec2 = _mm_sub_epi8(dstVec2, srcVec2);
    _mm_store_si128((__m128i*)dst, resVec1);
    _mm_store_si128((__m128i*)(dst + 16), resVec2);
}

inline void sub16i16(const int16_t* src, int16_t* dst) {
    __m128i srcVec1 = _mm_load_si128((__m128i*)src);
    __m128i srcVec2 = _mm_load_si128((__m128i*)(src + 8));
    __m128i dstVec1 = _mm_load_si128((__m128i*)dst);
    __m128i dstVec2 = _mm_load_si128((__m128i*)(dst + 8));
    __m128i resVec1 = _mm_sub_epi16(dstVec1, srcVec1);
    __m128i resVec2 = _mm_sub_epi16(dstVec2, srcVec2);
    _mm_store_si128((__m128i*)dst, resVec1);
    _mm_store_si128((__m128i*)(dst + 8), resVec2);
}

inline void sub8i32(const int32_t* src, int32_t* dst) {
    __m128i srcVec1 = _mm_load_si128((__m128i*)src);
    __m128i srcVec2 = _mm_load_si128((__m128i*)(src + 4));
    __m128i dstVec1 = _mm_load_si128((__m128i*)dst);
    __m128i dstVec2 = _mm_load_si128((__m128i*)(dst + 4));
    __m128i resVec1 = _mm_sub_epi32(dstVec1, srcVec1);
    __m128i resVec2 = _mm_sub_epi32(dstVec2, srcVec2);
    _mm_store_si128((__m128i*)dst, resVec1);
    _mm_store_si128((__m128i*)(dst + 4), resVec2);
}

inline void clippedReLU32i16(const int16_t* src, int8_t* dst) {
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

inline void scaledClippedReLU32i32(const int32_t* src, int8_t* dst) {
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

#else

inline void assign32i8(const int8_t* src, int8_t* dst) {
    for(size_t i = 0; i < 32; i++)
        dst[i] = src[i];
}

inline void assign16i16(const int16_t* src, int16_t* dst) {
    for(size_t i = 0; i < 16; i++)
        dst[i] = src[i];
}

inline void assign8i32(const int32_t* src, int32_t* dst) {
    for(size_t i = 0; i < 8; i++)
        dst[i] = src[i];
}

inline void add32i8(const int8_t* src, int8_t* dst) {
    for(size_t i = 0; i < 32; i++)
        dst[i] += src[i];
}

inline void add16i16(const int16_t* src, int16_t* dst) {
    for(size_t i = 0; i < 16; i++)
        dst[i] += src[i];
}

inline void add8i32(const int32_t* src, int32_t* dst) {
    for(size_t i = 0; i < 8; i++)
        dst[i] += src[i];
}

inline void sub32i8(const int8_t* src, int8_t* dst) {
    for(size_t i = 0; i < 32; i++)
        dst[i] -= src[i];
}

inline void sub16i16(const int16_t* src, int16_t* dst) {
    for(size_t i = 0; i < 16; i++)
        dst[i] -= src[i];
}

inline void sub8i32(const int32_t* src, int32_t* dst) {
    for(size_t i = 0; i < 8; i++)
        dst[i] -= src[i];
}

inline void clippedReLU32i16(const int16_t* src, int8_t* dst) {
    for(size_t i = 0; i < 32; i++)
        dst[i] = std::clamp(src[i], (int16_t)0, (int16_t)127);
}

inline void scaledClippedReLU32i32(const int32_t* src, int8_t* dst) {
    for(size_t i = 0; i < 32; i++)
        dst[i] = std::clamp(src[i] >> 6, 0, 127);
}

#endif

#endif