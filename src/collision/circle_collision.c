/*
circle_collision.c

Copyright (c) 11 Yann BOUCHER (yann)

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

#include "collision.h"
#include <stdint.h>

#ifdef __SSE2__
#include <xmmintrin.h>
#endif

#ifdef __SSE4_1__
#include <smmintrin.h>
#endif

static inline uint32_t sse_circle_test(const packed_circle_col_info* pck, float pointx, float pointy, float point_radius)
{
    __m128i bullet_ids = _mm_load_si128((const __m128i*)pck->ids);
    // no actives hitboxes, early bail out
#ifdef __SSE4_1__
    if (_mm_test_all_zeros(bullet_ids, _mm_set1_epi8(0xFF)))
        return 0;
#else
    if (_mm_movemask_epi8(_mm_cmpeq_epi32(bullet_ids,_mm_setzero_si128())) == 0xFFFF)
        return 0;
#endif

    __m128 x_vector   = _mm_set_ps1(pointx);
    __m128 y_vector   = _mm_set_ps1(pointy);
    __m128 center_x   = _mm_load_ps(pck->center_x);
    __m128 center_y   = _mm_load_ps(pck->center_y);
    __m128 radius     = _mm_load_ps(pck->radius);

    radius = _mm_add_ps(radius, _mm_set_ps1(point_radius));
    radius = _mm_mul_ps(radius, radius);

    __m128 dx = _mm_sub_ps(center_x, x_vector);
    __m128 dy = _mm_sub_ps(center_y, y_vector);

    dx = _mm_mul_ps(dx, dx);
    dy = _mm_mul_ps(dy, dy);
    __m128 dist_sqr = _mm_add_ps(dx, dy);

    __m128i comp_mask = _mm_castps_si128(_mm_cmple_ps(dist_sqr, radius));

    bullet_ids = _mm_and_si128(bullet_ids, comp_mask);

#ifndef __SSE4_1__ // _mm_max_epu32 isn't available before SSE 4.1
    __m128i result_ids = (__m128i)_mm_max_ps((__m128)bullet_ids, (__m128)_mm_shuffle_epi32(bullet_ids, _MM_SHUFFLE(2, 1, 0, 3)));
    result_ids         = (__m128i)_mm_max_ps((__m128)result_ids, (__m128)_mm_shuffle_epi32(result_ids, _MM_SHUFFLE(1, 0, 3, 2)));
#else
    __m128i result_ids = _mm_max_epu32(bullet_ids, _mm_shuffle_epi32(bullet_ids, _MM_SHUFFLE(2, 1, 0, 3)));
    result_ids         = _mm_max_epu32(result_ids, _mm_shuffle_epi32(result_ids, _MM_SHUFFLE(1, 0, 3, 2)));
#endif

    return _mm_cvtsi128_si32(result_ids);
}

// Compiler auto-vectorizer friendly
static inline uint32_t naive_circle_test(const packed_circle_col_info* pck, float pointx, float pointy, float point_radius)
{
    // early bail
    if (pck->ids[0] == 0 && pck->ids[1] == 0 &&
        pck->ids[2] == 0 && pck->ids[3] == 0)
        return 0;

    int chosen[SIMD_FLT_PER_REG];

    for (unsigned i = 0; i < SIMD_FLT_PER_REG; ++i)
    {
        float test_distance_squared = (point_radius+pck->radius[i]) * (point_radius+pck->radius[i]);

        float dx = pck->center_x[i] - pointx;
        float dy = pck->center_y[i] - pointy;

        float actual_distance_squared = dx*dx + dy*dy;

        chosen[i] = actual_distance_squared <= test_distance_squared;
    }

    for (unsigned i = 0; i < SIMD_FLT_PER_REG; ++i)
    {
        if (chosen[i])
            return pck->ids[i];
    }

    return INVALID_BULLET_ID;
}

static inline uint32_t circle_test(const packed_circle_col_info* pck, float pointx, float pointy, float point_radius)
{
#if defined(__SSE2__)
    return sse_circle_test(pck, pointx, pointy, point_radius);
#else
    return naive_circle_test(pck, pointx, pointy, point_radius);
#endif
}
