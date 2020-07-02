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
#include <math.h>
#include "math/vector.h"

#ifdef __SSE2__
#include <xmmintrin.h>
#define USE_SSE2
#include "sse_trig.h"
#endif

#ifdef __SSE4_1__
#include <smmintrin.h>
#endif

static inline __m128 abs_mask(void)
{
    // with clang, this turns into a 16B load,
    // with every calling function getting its own copy of the mask
    __m128i minus1 = _mm_set1_epi32(-1);
    return _mm_castsi128_ps(_mm_srli_epi32(minus1, 1));
}
// MSVC is BAD when inlining this into loops
static inline __m128 vecabs_and(__m128 v)
{
    return _mm_and_ps(abs_mask(), v);
}

// algorithm taken from here : https://stackoverflow.com/questions/401847/circle-rectangle-collision-detection-intersection
static inline uint32_t sse_rect_test(const packed_rect_col_info* pck, float pointx, float pointy, float point_radius)
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
    __m128 radius_vector   = _mm_set_ps1(point_radius);
    __m128 radius_squared  = _mm_mul_ps(radius_vector, radius_vector);
    __m128 rect_x   = _mm_load_ps(pck->center_x);
    __m128 rect_y   = _mm_load_ps(pck->center_y);
    __m128 rect_half_width    = _mm_load_ps(pck->half_width);
    __m128 rect_half_height   = _mm_load_ps(pck->half_height);
    __m128 angles = _mm_load_ps(pck->angle);

    // transfer the circle coordinate's from world space to rotated world space relative to the rectangle
    __m128 sins, coss;
    sincos_ps(angles, &sins, &coss);
    x_vector = _mm_sub_ps(x_vector, rect_x);
    y_vector = _mm_sub_ps(y_vector, rect_y);
    // apply the rotation matrix
    __m128 newx = _mm_sub_ps(_mm_mul_ps(x_vector, coss), _mm_mul_ps(y_vector, sins));
    __m128 newy = _mm_add_ps(_mm_mul_ps(x_vector, sins), _mm_mul_ps(y_vector, coss));
    // translate point back
    x_vector = _mm_add_ps(newx, rect_x);
    y_vector = _mm_add_ps(newy, rect_y);

    __m128 edge_distance_x = vecabs_and(x_vector - rect_x) - rect_half_width;
    __m128 edge_distance_y = vecabs_and(y_vector - rect_y) - rect_half_height;

    __m128i close_enough_mask = _mm_castps_si128(_mm_cmple_ps(edge_distance_x, radius_vector));
    close_enough_mask = _mm_and_si128(close_enough_mask, _mm_castps_si128(_mm_cmple_ps(edge_distance_y, radius_vector)));

    __m128i collision_mask    = _mm_castps_si128(_mm_cmple_ps(edge_distance_x, _mm_setzero_ps()));
    collision_mask    = _mm_or_si128(collision_mask, _mm_castps_si128(_mm_cmple_ps(edge_distance_y, _mm_setzero_ps())));

    __m128 corner_distance_squared = _mm_mul_ps(edge_distance_x, edge_distance_x) + _mm_mul_ps(edge_distance_y, edge_distance_y);
    collision_mask    = _mm_or_si128(collision_mask, _mm_castps_si128(_mm_cmple_ps(corner_distance_squared, radius_squared)));

    collision_mask    = _mm_and_si128(collision_mask, close_enough_mask);

    bullet_ids = _mm_and_si128(bullet_ids, collision_mask);

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
static inline uint32_t naive_rect_test(const packed_rect_col_info* pck, float pointx, float pointy, float point_radius)
{
    // early bail
    if (pck->ids[0] == 0 && pck->ids[1] == 0 &&
        pck->ids[2] == 0 && pck->ids[3] == 0)
        return 0;

    int chosen[SIMD_FLT_PER_REG];

    for (unsigned i = 0; i < SIMD_FLT_PER_REG; ++i)
    {
        vector2d_t rotated = rotate_point((vector2d_t){pointx, pointy}, pck->angle[i], (vector2d_t){pck->center_x[i], pck->center_y[i]});
        float rotated_pointx = rotated.x;
        float rotated_pointy = rotated.y;

        float circle_dist_x = fabsf(rotated_pointx - pck->center_x[i]);
        float circle_dist_y = fabsf(rotated_pointy - pck->center_y[i]);

        if (circle_dist_x > (pck->half_width[i] + point_radius))
        {
            chosen[i] = 0;
            continue;
        }
        if (circle_dist_y > (pck->half_height[i] + point_radius))
        {
            chosen[i] = 0;
            continue;
        }

        if (circle_dist_x <= pck->half_width[i])
        {
            chosen[i] = 1;
            continue;
        }
        if (circle_dist_y <= pck->half_height[i])
        {
            chosen[i] = 1;
            continue;
        }

        float cornerDistance_sq = (circle_dist_x - pck->half_width[i])*(circle_dist_x - pck->half_width[i])
                                      +
                                  (circle_dist_y - pck->half_height[i])*(circle_dist_y - pck->half_height[i]);


        chosen[i] = cornerDistance_sq <= (point_radius*point_radius);
    }

    for (unsigned i = 0; i < SIMD_FLT_PER_REG; ++i)
    {
        if (chosen[i])
            return pck->ids[i];
    }

    return INVALID_BULLET_ID;
}

static inline uint32_t rect_test(const packed_rect_col_info* pck, float pointx, float pointy, float point_radius)
{
#if defined(__SSE2__)
    return sse_rect_test(pck, pointx, pointy, point_radius);
#else
    return naive_rect_test(pck, pointx, pointy, point_radius);
#endif
}
