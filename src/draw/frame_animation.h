#ifndef ANIMATION_H
#define ANIMATION_H

#define MAX_ANIM_FRAMES 64

#include "resources/texture_handler.h"

typedef struct anim_frame_t
{
    rect_t  spritesheet_rect;
    unsigned frame_duration;
} anim_frame_t;

typedef struct frame_animation_t
{
    spritesheet_id_t sheet;
    unsigned int anim_frame_count;
    unsigned int start_frame;
    unsigned int total_frametime;
    anim_frame_t frames[MAX_ANIM_FRAMES];
} frame_animation_t;

void start_frame_animation(frame_animation_t* anim);
void reset_frame_animation(frame_animation_t* anim);
void draw_frame_animation (const frame_animation_t* anim, vector2d_t pos, vector2d_t size, float angle);

#endif // ANIMATION_H
