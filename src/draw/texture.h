#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>

typedef struct texture_t
{
    uint32_t id;
    int width, height;
    int mipmaps;
    int format;
} texture_t;

void init_textures();
texture_t load_texture(const char* filepath);
void unload_texture(texture_t tex);
// returns a *reference* to a texture containing the active framebuffer
texture_t capture_framebuffer();

#endif // TEXTURE_H
