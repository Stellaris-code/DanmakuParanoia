/*
collision_info.h

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
#ifndef COLLISION_INFO_H
#define COLLISION_INFO_H

#include "simd/simd.h"
#include "gameplay/bullet/bullet_id.h"

#define MAX_HITBOX_COUNT 0x400000 // ~4M. yeah, I'm a lunatic. thank god for the .bss section by the way
//#define MAX_HITBOX_COUNT 65536
#define INVALID_HITBOX_ID ((hitbox_id_t)-1)

typedef int32_t hitbox_id_t;

_Static_assert(SIMD_ID_PER_REG == SIMD_FLT_PER_REG, "hitbox_id_t has a different size compared to float");
// AoSoA
typedef struct packed_rect_col_info
{
    _Alignas(VECTOR_WIDTH) bullet_id_t ids        [SIMD_ID_PER_REG ];
    _Alignas(VECTOR_WIDTH) float       center_x   [SIMD_FLT_PER_REG];
    _Alignas(VECTOR_WIDTH) float       center_y   [SIMD_FLT_PER_REG];
    _Alignas(VECTOR_WIDTH) float       half_width [SIMD_FLT_PER_REG];
    _Alignas(VECTOR_WIDTH) float       half_height[SIMD_FLT_PER_REG];
    _Alignas(VECTOR_WIDTH) float       angle      [SIMD_FLT_PER_REG];
} packed_rect_col_info;

typedef struct packed_circle_col_info
{
    _Alignas(VECTOR_WIDTH) bullet_id_t ids [SIMD_ID_PER_REG ];
    _Alignas(VECTOR_WIDTH) float center_x  [SIMD_FLT_PER_REG];
    _Alignas(VECTOR_WIDTH) float center_y  [SIMD_FLT_PER_REG];
    _Alignas(VECTOR_WIDTH) float radius    [SIMD_FLT_PER_REG];
} packed_circle_col_info;

_Alignas(VECTOR_WIDTH) extern packed_rect_col_info   rect_hitboxes  [MAX_HITBOX_COUNT / SIMD_FLT_PER_REG];
_Alignas(VECTOR_WIDTH) extern packed_circle_col_info circle_hitboxes[MAX_HITBOX_COUNT / SIMD_FLT_PER_REG];
extern int rect_hitbox_index;
extern int circle_hitbox_index;

// used to skip checking a whole bunch of hitboxes if we know that they are empty
#define HITBOX_CHUNK_SIZE 1024
extern unsigned hitbox_chunk_count[(MAX_HITBOX_COUNT/SIMD_FLT_PER_REG) / HITBOX_CHUNK_SIZE];

#define GET_HITBOX_FIELD(type, field, idx) \
    &type##_hitboxes[(idx) / SIMD_FLT_PER_REG].field[(idx) % SIMD_FLT_PER_REG]

#define REGISTER_HITBOX(type, id) \
    register_hitbox(type##_hitboxes, sizeof(type##_hitboxes[0]), &type##_hitbox_index, id)

#define FREE_HITBOX(type, id) \
    do { \
    bullet_id_t* id_field = &type##_hitboxes[id/SIMD_FLT_PER_REG].ids[id%SIMD_FLT_PER_REG]; \
assert(*id_field != INVALID_BULLET_ID); \
*id_field = INVALID_BULLET_ID; \
unsigned chunk_id = (id/SIMD_FLT_PER_REG) / HITBOX_CHUNK_SIZE; \
--hitbox_chunk_count[chunk_id]; \
    } while (0)


hitbox_id_t register_hitbox(void* hitbox_list, unsigned hitbox_len, int* hitbox_index, bullet_id_t id);

bullet_id_t test_collision(float pos_x, float pos_y, float hit_radius);

#endif // COLLISION_INFO_H
