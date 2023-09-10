/*
bullet.h

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
#ifndef BULLET_H
#define BULLET_H

#include <stdint.h>
#include <stdbool.h>

#include "resources/texture_handler.h"
#include "collision/collision.h"
#include "math/vector.h"
#include "gameplay/motion.h"
#include "utils/resource_pool.h"
#include "bullet_id.h"

typedef enum BulletType
{
    Circle = 0,
    Rect,

    MAX_BULLET_TYPES
} BulletType;

#define MAX_BULLETS_PER_TYPE MAX_HITBOX_COUNT/MAX_BULLET_TYPES

extern texture_t sprite_tex_atlas;

typedef struct bullet_info_t
{
    uint8_t type;
    hitbox_id_t hitbox;
    sprite_frame_id_t sprite;
} bullet_info_t;

typedef struct cicle_info_t
{
    float radius;
    float hitbox_radius;
} circle_info_t;

typedef struct rect_info_t
{
    uint16_t width;
    uint16_t height;
    float hitbox_width;
    float hitbox_height;
} rect_info_t;

void init_bullet_manager();
void cleanup_bullet_manager();

bullet_id_t register_bullet(bullet_info_t bullet_info, motion_data_t motion, void* specific_data);
void update_bullets(float dt);
void draw_bullets();
void clear_bullets();

int total_bullet_count();

#endif // BULLET_H
