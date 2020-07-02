#include "animation.h"

#include "draw/draw.h"
#include "gameplay/gamestate.h"

void draw_animation(const animation_t *anim, vector2d_t pos, vector2d_t size, float angle)
{
    unsigned frame_time = global_state.current_frame - anim->start_frame;
    frame_time %= anim->total_frametime;
    int frame = 0;
    for (unsigned i = 0; i < anim->anim_frame_count; ++i)
    {
        if (frame_time < anim->frames[i].frame_duration)
        {
            frame = i;
            break;
        }
        else
        {
            frame_time -= anim->frames[i].frame_duration;
        }
    }

    texture_t  tex       = get_spritesheet_texture(anim->sheet);
    rect_t src_rect = anim->frames[frame].spritesheet_rect;

    rect_t  dest_rect = {pos.x, pos.y, size.x, size.y};
    vector2d_t  origin = {size.x/2, size.y/2};
    draw_textured_rect(tex, src_rect, dest_rect, origin, angle, COL_WHITE);
}
