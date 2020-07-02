#include "draw/vfx.h"

#include <string.h>

#include "math/easing.h"
#include "draw/draw.h"

bool vfx_sheared_rectangle_draw(void* databuf, float time)
{
    vfx_sheared_rectangle_info_t info;
    memcpy(&info, databuf, sizeof(info)); // standard-compliant type-punning

    if (info.persist)
    {
        if (time > info.effect_duration*2.f)
            return true;
    }
    else
    {
        if (time > info.effect_duration)
            return true;
    }

    bool invert = time > info.effect_duration/2.f;
    if (info.persist)
        invert = false;

    const float progression = easeInOutCubic(time/(info.effect_duration/2.f));
    float actual_len;
    if (!invert)
        actual_len = info.max_length * progression;
    else
        actual_len = info.max_length * (progression - 1);

    vector2d_t vert[4] =
        {
            {0.0f, invert ? actual_len : 0.0f},
            {0.0f, invert ? info.max_length :actual_len},
            {info.width, invert ? actual_len : 0.0f},
            {info.width, invert ? info.max_length : actual_len}
        };
    if (actual_len < 0)
        SWAP(vert[0], vert[3]);

    draw_sheared_quad(info.shearing, vert, info.pos, info.angle, info.col);

    return false; // effect is not finished yet
}
