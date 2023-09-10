/*
draw.h

Copyright (c) 13 Yann BOUCHER (yann)

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
#ifndef DRAW_H
#define DRAW_H

#include "spriteframe.h"
#include "color.h"
#include "math/vector.h"

// (4096/2)*16bytes = 32k
// 16kb instead as guaranteed by standard
#define SPRITE_BATCH_ELEMENTS (4096/2)

void translate_viewport(vector3d_t offset);
void ignore_viewport_transformations();
void restore_viewport_transformations();
void reset_viewport();

void draw_textured_rect(texture_t tex, rect_t src_rect, rect_t dest_rect, vector2d_t origin, float angle, color_t tint);
void draw_rect(rect_t rect, vector2d_t origin, float angle, color_t color);
void draw_sprite(sprite_frame_id_t sprite, vector2d_t pos, vector2d_t size, float angle, color_t tint);
void draw_circle(vector2d_t pos, float radius, color_t color);

// expects an angle normalized to the range 0..2pi
// expects texture coordinates as 16-bit normalized unsigned short values
typedef struct quad_data_t
{
    unsigned short world_pos[2];
    unsigned short size[2];
    uint16_t sprite_id;
    uint16_t zorder;
    float angle;
} quad_data_t;

void begin_draw_sprite_batch(texture_t tex);
// returns a pointer to 'amount' quad elements to be configured for the sprite batch
static inline quad_data_t* prepare_batch_elements(unsigned amount)
{
    extern unsigned batch_elem_count;
    extern quad_data_t batch_quad_data[];
    void commit_sprite_batch();

    if (batch_elem_count + amount >= SPRITE_BATCH_ELEMENTS)
    {
        commit_sprite_batch();
    }

    quad_data_t* quad_ptr = &batch_quad_data[batch_elem_count];
    batch_elem_count += amount;

    return quad_ptr;
}
static inline void draw_sprite_batch_element(vector2d_t size, vector2d_t pos, float angle, int zorder, sprite_frame_id_t frame_id)
{
    quad_data_t* quad_ptr = prepare_batch_elements(1);
    quad_ptr[0].sprite_id = frame_id;
    quad_ptr[0].zorder = zorder;
    quad_ptr[0].angle = angle;
    quad_ptr[0].world_pos[0] = pos.x;
    quad_ptr[0].world_pos[1] = pos.y;
    quad_ptr[0].size[0] = size.x;
    quad_ptr[0].size[1] = size.y;
}
void end_draw_sprite_batch();

void draw_sheared_quad(float shearing, vector2d_t vertices[4], vector2d_t pos, float angle, color_t color);

#endif // DRAW_H
