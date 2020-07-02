#ifndef BLEND_MODE_H
#define BLEND_MODE_H

typedef enum blend_mode
{
    BlendAlpha = 0,
    BlendAdditive,
    BlendMultiplied
} blend_mode;

void enable_blend_mode(blend_mode mode);
void reset_blend_mode();

#endif // BLEND_MODE_H
