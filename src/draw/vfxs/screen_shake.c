#include "draw/vfx.h"

#include <stdlib.h>
#include <string.h>

#include "draw/draw.h"
#include "utils/utils.h"

bool vfx_screen_shake_draw(void *databuf, float time)
{
    vfx_screen_shake_info_t info;
    memcpy(&info, databuf, sizeof(info)); // standard-compliant type-punning

    if (time > info.duration) // reset the effect
    {
        return true;
    }

    int current_sample_idx = (int)(time*info.frequency);

    // generate a new sample
    if (current_sample_idx > info.last_sample_idx)
    {
        info.last_sample = info.current_sample;
        info.last_sample_idx = current_sample_idx;

        info.current_sample = (vector2d_t){randf(2.f)-1.0f, randf(2.f)-1.0f};
    }

    vector2d_t interpolated_offset;
    // interpolate
    float sample_progress = (time*info.frequency - current_sample_idx)/info.frequency;
    interpolated_offset.x = lerp(info.last_sample.x, info.current_sample.x, sample_progress);
    interpolated_offset.y = lerp(info.last_sample.y, info.current_sample.y, sample_progress);

    float decay = 1.0 - time/info.duration;

    translate_viewport((vector3d_t){interpolated_offset.x*info.x_amplitude*decay, interpolated_offset.y*info.y_amplitude*decay, 0.f});

    memcpy(databuf, &info, sizeof(info));

    return false;
}
