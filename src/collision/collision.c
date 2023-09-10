/*
collision.c

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

#include "collision.h"

#include <assert.h>
#include <string.h>

_Alignas(VECTOR_WIDTH) packed_rect_col_info   rect_hitboxes  [MAX_HITBOX_COUNT / SIMD_FLT_PER_REG];
_Alignas(VECTOR_WIDTH) packed_circle_col_info circle_hitboxes[MAX_HITBOX_COUNT / SIMD_FLT_PER_REG];
int rect_hitbox_index = 0;
int circle_hitbox_index = 0;

// used to skip checking a whole bunch of hitboxes if we know that they are empty
unsigned hitbox_chunk_count[(MAX_HITBOX_COUNT/SIMD_FLT_PER_REG) / HITBOX_CHUNK_SIZE];

#include "circle_collision.c"
#include "rect_collision.c"

hitbox_id_t register_hitbox(void* hitbox_list, unsigned hitbox_len, int *hitbox_index, bullet_id_t id)
{
    for (size_t counter = 0; counter < MAX_HITBOX_COUNT/SIMD_FLT_PER_REG; ++counter)
    {
        int i = (*hitbox_index + counter) % (MAX_HITBOX_COUNT/SIMD_FLT_PER_REG);

        void* packed_hitbox_ptr = ((uint8_t*)hitbox_list + i*hitbox_len);
        for (size_t j = 0; j < SIMD_FLT_PER_REG; ++j)
        {
            bullet_id_t* id_field = (bullet_id_t*)packed_hitbox_ptr + j;

            if (*id_field == INVALID_BULLET_ID)
            {
                *id_field = id;

                *hitbox_index = i;

                unsigned chunk_id = i / HITBOX_CHUNK_SIZE;
                ++hitbox_chunk_count[chunk_id];

                hitbox_id_t id = i*SIMD_FLT_PER_REG + j;
                return id;
            }
        }
    }
    if (*hitbox_index != 0) // retry scanning the entire array this time
    {
        *hitbox_index = 0;
        return register_hitbox(hitbox_list, hitbox_len, hitbox_index, id);
    }

    return INVALID_HITBOX_ID; // no hitbox of this type could be allocated
}

bullet_id_t test_collision(float pos_x, float pos_y, float hit_radius)
{
    for (size_t i = 0; i < MAX_HITBOX_COUNT/SIMD_FLT_PER_REG; ++i)
    {
        unsigned chunk_id = i / HITBOX_CHUNK_SIZE;
        // no active hitboxes here, skip
        if (hitbox_chunk_count[chunk_id] == 0)
        {
            i += HITBOX_CHUNK_SIZE;
            continue;
        }

        int id = circle_test(circle_hitboxes + i, pos_x, pos_y, hit_radius);
        if (id)
        {
            return id;
        }
    }

    for (size_t i = 0; i < MAX_HITBOX_COUNT/SIMD_FLT_PER_REG; ++i)
    {
        unsigned chunk_id = i / HITBOX_CHUNK_SIZE;
        // no active hitboxes here, skip
        if (hitbox_chunk_count[chunk_id] == 0)
        {
            i += HITBOX_CHUNK_SIZE;
            continue;
        }

        int id = rect_test(rect_hitboxes + i, pos_x, pos_y, hit_radius);
        if (id)
        {
            return id;
        }
    }

    return INVALID_BULLET_ID;
}
