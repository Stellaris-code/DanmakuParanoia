/*
texture_handler.h

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
#ifndef TEXTURE_HANDLER_H
#define TEXTURE_HANDLER_H

#include <stdint.h>

#include "draw/spriteframe.h"

#define INVALID_TEXTURE_ID 0xFF

typedef uint8_t spritesheet_id_t;
typedef uint8_t texture_id_t;

static inline sprite_frame_t get_sprite_frame(spritesheet_id_t id)
{
    extern sprite_frame_t sprite_frame_array[];
    return sprite_frame_array[id];
}

void init_texture_handler();

texture_t        get_spritesheet_texture(spritesheet_id_t id);
spritesheet_id_t get_spritesheet(const char* key);
spritesheet_id_t load_spritesheet(const char* filename, const char *key);

texture_id_t register_texture(texture_t tex, const char* key);
texture_t*   retrieve_texture(texture_id_t id);

sprite_frame_id_t load_sprite_frame(spritesheet_id_t sprsht, rect_t tex_rec);

#endif // TEXTURE_HANDLER_H
