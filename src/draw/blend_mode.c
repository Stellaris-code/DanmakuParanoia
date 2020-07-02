#include "blend_mode.h"

#include <raylib.h>

void enable_blend_mode(blend_mode mode)
{
    switch (mode)
    {
        case BlendAdditive:
            BeginBlendMode(BLEND_ADDITIVE);
            break;
        case BlendMultiplied:
            BeginBlendMode(BLEND_MULTIPLIED);
            break;
        case BlendAlpha:
        default:
            BeginBlendMode(BLEND_ALPHA);
            break;
    }
}

void reset_blend_mode()
{
    EndBlendMode();
}
