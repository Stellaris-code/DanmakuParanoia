/*
simd_info.h

Copyright (c) 05 Yann BOUCHER (yann)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
                                                              copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

       THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
        SOFTWARE.

            */
#ifndef SIMD_INFO_H
#define SIMD_INFO_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "utils/utils.h"

// register width in bytes
#if  defined(__AVX2__) && !defined(NO_SIMD)
#define SIMD_AVX2
#include <immintrin.h>
#define VECTOR_WIDTH 32
        typedef __m256  simd_vf;
typedef __m256i simd_vi;
#elif defined(__SSE2__) && !defined(NO_SIMD)
#include <emmintrin.h>
#define SIMD_SSE2
#define VECTOR_WIDTH 16
        typedef __m128  simd_vf;
typedef __m128i simd_vi;
#else
#define SIMD_NONE
#define VECTOR_WIDTH sizeof(float)
        typedef float simd_vf;
typedef int32_t   simd_vi;
#endif

#define SIMD_FLT_PER_REG (VECTOR_WIDTH/sizeof(float))
#define SIMD_ID_PER_REG  (VECTOR_WIDTH/sizeof(uint32_t))

static inline simd_vf vf_set1(float val)
{
#ifdef SIMD_AVX2
    return _mm256_set1_ps(val);
#elif defined(SIMD_SSE2)
    return _mm_set1_ps(val);
#else
    return val;
#endif
}
static inline simd_vi vi_set1(int val)
{
#ifdef SIMD_AVX2
    return _mm256_set1_epi32(val);
#elif defined(SIMD_SSE2)
    return _mm_set1_epi32(val);
#else
    return val;
#endif
}

static inline simd_vf vf_load(float* ptr)
{
#ifdef SIMD_AVX2
    return _mm256_load_ps((const __m256*)ptr);
#elif defined(SIMD_SSE2)
    return _mm_load_ps((const __m128*)ptr);
#else
    return *ptr;
#endif
}
static inline simd_vi vi_load(int* ptr)
{
#ifdef SIMD_AVX2
    return _mm256_load_si256((const __m256i*)ptr);
#elif defined(SIMD_SSE2)
    return _mm_load_si128((const __m128i*)ptr);
#else
    return *ptr;
#endif
}

static inline void vf_store(float* ptr, simd_vf val)
{
#ifdef SIMD_AVX2
    _mm256_store_ps((__m256*)ptr, val);
#elif defined(SIMD_SSE2)
    _mm_store_ps((__m128*)ptr, val);
#else
    *ptr = val;
#endif
}
static inline void vi_store(int* ptr, simd_vi val)
{
#ifdef SIMD_AVX2
    _mm256_store_si256((__m256i*)ptr, val);
#elif defined(SIMD_SSE2)
    _mm_store_si128((__m128i*)ptr, val);
#else
    *ptr = val;
#endif
}

static inline simd_vi vf_or(simd_vf lhs, simd_vf rhs)
{
#ifdef SIMD_AVX2
    return _mm256_castps_si256(_mm256_or_ps(lhs, rhs));
#elif defined(SIMD_SSE2)
    return _mm_castps_si128(_mm_or_ps(lhs, rhs));
#else
    return *(int*)&lhs | *(int*)&rhs;
#endif
}

static inline simd_vi vi_or(simd_vi lhs, simd_vi rhs)
{
#ifdef SIMD_AVX2
    return _mm256_or_si256(lhs, rhs);
#elif defined(SIMD_SSE2)
    return _mm_or_si128(lhs, rhs);
#else
    return lhs | rhs;
#endif
}

static inline simd_vi vi_andnot(simd_vi lhs, simd_vi rhs)
{
#ifdef SIMD_AVX2
    return _mm256_andnot_si256(lhs, rhs);
#elif defined(SIMD_SSE2)
    return _mm_andnot_si128(lhs, rhs);
#else
    return (~lhs) & rhs;
#endif
}

static inline simd_vi vf_cmpgt(simd_vf lhs, simd_vf rhs)
{
#ifdef SIMD_AVX2
    return _mm256_castps_si256(_mm256_cmp_ps(lhs, rhs, _CMP_GT_OQ));
#elif defined(SIMD_SSE2)
    return _mm_castps_si128(_mm_cmpgt_ps(lhs, rhs));
#else
    return (lhs > rhs) ? ~0 : 0;
#endif
}

static inline simd_vi vf_cmplt(simd_vf lhs, simd_vf rhs)
{
#ifdef SIMD_AVX2
    return _mm256_castps_si256(_mm256_cmp_ps(lhs, rhs, _CMP_LT_OQ));
#elif defined(SIMD_SSE2)
    return _mm_castps_si128(_mm_cmplt_ps(lhs, rhs));
#else
    return (lhs < rhs) ? ~0 : 0;
#endif
}

static inline simd_vf vf_min(simd_vf lhs, simd_vf rhs)
{
#ifdef SIMD_AVX2
    return _mm256_min_ps(lhs, rhs);
#elif defined(SIMD_SSE2)
    return _mm_min_ps(lhs, rhs);
#else
    return MIN(lhs, rhs);
#endif
}

static inline simd_vf vf_max(simd_vf lhs, simd_vf rhs)
{
#ifdef SIMD_AVX2
    return _mm256_max_ps(lhs, rhs);
#elif defined(SIMD_SSE2)
    return _mm_max_ps(lhs, rhs);
#else
    return MAX(lhs, rhs);
#endif
}

static inline bool vi_vector_eq(simd_vi lhs, simd_vi rhs)
{
#ifdef SIMD_AVX2
    return _mm256_movemask_epi8(_mm256_cmpeq_epi8(lhs, rhs)) == 0xFFFFFFFFU;
#elif defined(SIMD_SSE2)
    return _mm_movemask_epi8(_mm_cmpeq_epi8(lhs, rhs)) == 0xFFFFU;
#else
    return lhs == rhs;
#endif
}

static inline simd_vf vf_rsqrt(simd_vf val)
{
#ifdef SIMD_AVX2
    return _mm256_rsqrt_ps(val);
#elif defined(SIMD_SSE2)
    return _mm_rsqrt_ps(val);
#else
    return 1.0/sqrtf(val);
#endif
}
static inline simd_vf vf_sqrt(simd_vf val)
{
#ifdef SIMD_AVX2
    return _mm256_sqrt_ps(val);
#elif defined(SIMD_SSE2)
    return _mm_sqrt_ps(val);
#else
    return sqrtf(val);
#endif
}

static inline simd_vf vf_floor(simd_vf val)
{
#ifdef SIMD_AVX2
    return _mm256_floor_ps(val);
#elif defined(SIMD_SSE2)
    __m128 one = _mm_set1_ps(1.0f);

    __m128 t = _mm_cvtepi32_ps(_mm_cvttps_epi32(val));
    __m128 r = _mm_sub_ps(t, _mm_and_ps(_mm_cmplt_ps(val, t), one));

    return r;
#else
    return floorf(val);
#endif
}

static inline simd_vf vf_abs(simd_vf val)
{
#ifdef SIMD_AVX2
    const __m256 sign_mask = _mm256_set1_ps(-0.f); // -0.f = 1 << 31
    return _mm256_andnot_ps(sign_mask, val);
#elif defined(SIMD_SSE2)
    const __m128 sign_mask = _mm_set1_ps(-0.f); // -0.f = 1 << 31
    return _mm_andnot_ps(sign_mask, val);
#else
    return fabsf(val);
#endif
}

static inline simd_vf vf_copysign(simd_vf from, simd_vf to)
{
#ifdef SIMD_AVX2
    const float signbit = -0.f;
    __m256 const avx_signbit = _mm256_set1_ps(signbit);
    return _mm256_or_ps(_mm256_and_ps(avx_signbit, from), _mm256_andnot_ps(avx_signbit, to)); // (avx_signbit & from) | (~avx_signbit & to)

#elif defined(SIMD_SSE2)
    const float signbit = -0.f;
    __m128 const avx_signbit = _mm_set1_ps(signbit);
    return _mm_or_ps(_mm_and_ps(avx_signbit, from), _mm_andnot_ps(avx_signbit, to)); // (avx_signbit & from) | (~avx_signbit & to)

#else
    return copysignf(to, from);
#endif
}

#endif // SIMD_INFO_H
