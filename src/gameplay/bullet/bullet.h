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

#define MAX_BULLETS MAX_HITBOX_COUNT

typedef enum BulletType
{
    Circle,
    Rect
} BulletType;

typedef struct bullet_t
{
    hitbox_id_t hitbox;
    BulletType type;
    sprite_frame_id_t sprite;
    motion_data_t motion;

    // internal data
    float* hitbox_center_x; // pointer to the hitbox's position field
    float* hitbox_center_y; // pointer to the hitbox's position field
    float* hitbox_angle;    // pointer to the hitbox's angle field
    float  visible_radius; // computed for each update
    bool to_be_removed;
} bullet_t;

typedef struct cicle_info_t
{
    float radius;
    float hitbox_radius;
} circle_info_t;

typedef struct rect_info_t
{
    float width;
    float height;
    float hitbox_width;
    float hitbox_height;
} rect_info_t;

// returns a pointer to the actual registered bullet
bullet_t *register_bullet(bullet_t bullet_info, void* specific_data);
void update_bullets(float dt);
void draw_bullets();
void clear_bullets();

int total_bullet_count();

#endif // BULLET_H
