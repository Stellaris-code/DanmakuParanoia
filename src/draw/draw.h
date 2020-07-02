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

typedef struct sprite_list_entry_t
{
    vector2d_t pos;
    vector2d_t size;
    float angle;
    color_t tint;
    rect_t spriteframe;
} sprite_list_entry_t;

void translate_viewport(vector3d_t offset);
void ignore_viewport_transformations();
void restore_viewport_transformations();
void reset_viewport();

void draw_textured_rect(texture_t tex, rect_t src_rect, rect_t dest_rect, vector2d_t origin, float angle, color_t tint);
void draw_rect(rect_t rect, vector2d_t origin, float angle, color_t color);
void draw_sprite(sprite_frame_id_t sprite, vector2d_t pos, vector2d_t size, float angle, color_t tint);
void draw_sprite_batch(texture_t texture, sprite_list_entry_t* list, unsigned list_size);
void draw_circle(vector2d_t pos, float radius, color_t color);

void draw_sheared_quad(float shearing, vector2d_t vertices[4], vector2d_t pos, float angle, color_t color);

#endif // DRAW_H
