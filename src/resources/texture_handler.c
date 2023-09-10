/*
texture_handler.c

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

#include "texture_handler.h"

#define MAX_SPRITESHEETS  255
#define MAX_REGISTERED_TEXTURES 255

#include <assert.h>
#include <string.h>

#include "draw/texture.h"
#include "sys/log.h"
#include "sys/cleanup.h"

typedef struct registered_texture_t
{
    texture_t tex;
    const char* key;
} registered_texture_t;

static int spritesheet_count = 0;
static registered_texture_t spritesheet_array[MAX_SPRITESHEETS];

int sprite_frame_count = 0;
sprite_frame_t sprite_frame_array[MAX_SPRITE_FRAMES];

static registered_texture_t registered_textures[MAX_REGISTERED_TEXTURES];

spritesheet_id_t load_spritesheet(const char *filename, const char* key)
{
    if (spritesheet_count >= MAX_SPRITESHEETS)
    {
        trace_log(LOG_WARNING, "Spritesheet '%s' could not be loaded (spritesheet array is full)", filename);
        return (spritesheet_id_t)-1;
    }

    texture_t spritesheet = load_texture(filename);
    if (spritesheet.id == 0)
    {
        trace_log(LOG_WARNING, "Spritesheet '%s' could not be loaded", filename);
        return (spritesheet_id_t)-1;
    }

    trace_log(LOG_INFO, "Loaded Spritesheed %d from path '%s'\n", spritesheet_count, filename);
    spritesheet_array[spritesheet_count].key = key;
    spritesheet_array[spritesheet_count].tex = spritesheet;

    return spritesheet_count++;
}

sprite_frame_id_t load_sprite_frame(spritesheet_id_t sprsht, rect_t tex_rec)
{
    texture_t sht_tex = spritesheet_array[sprsht].tex;
    if (sht_tex.id == 0)
    {
        trace_log(LOG_WARNING, "Invalid spritesheet id %d", sprsht);
        return (sprite_frame_id_t)-1;
    }

    if (sprite_frame_count >= MAX_SPRITE_FRAMES)
    {
        trace_log(LOG_WARNING, "Sprite frame could not be loaded (sprite frame array is full)");
        return (spritesheet_id_t)-1;
    }

    // half pixel correction
    //tex_rec.x += 0.5;
    //tex_rec.y += 0.5;
    //tex_rec.w += 0.5;
    //tex_rec.h += 0.5;

    sprite_frame_array[sprite_frame_count].texture = sht_tex;
    sprite_frame_array[sprite_frame_count].spritesheet_rect = tex_rec;

    return sprite_frame_count++;
}

texture_t get_spritesheet_texture(spritesheet_id_t id)
{
    assert(id < MAX_SPRITESHEETS);
    return spritesheet_array[id].tex;
}

void texture_handler_cleanup()
{
    for (int i = 0; i < spritesheet_count; ++i)
    {
        unload_texture(spritesheet_array[i].tex);
    }

    for (int i = 0; i < MAX_REGISTERED_TEXTURES; ++i)
    {
        if (registered_textures[i].tex.id)
        {
            unload_texture(registered_textures[i].tex);
            registered_textures[i].tex.id = 0;
        }
    }

    spritesheet_count = 0;
    sprite_frame_count = 0;
}

void init_texture_handler()
{
    register_cleanup(texture_handler_cleanup, GamestateEnd);
}

texture_id_t register_texture(texture_t tex, const char *key)
{
    for (int i = 0; i < MAX_REGISTERED_TEXTURES; ++i)
    {
        if (registered_textures[i].tex.id == 0)
        {
            registered_textures[i].key = key;
            registered_textures[i].tex = tex;
            return i;
        }
    }

    return INVALID_TEXTURE_ID;
}

texture_t *retrieve_texture(texture_id_t id)
{
    if (id >= MAX_REGISTERED_TEXTURES || id == INVALID_TEXTURE_ID)
        return 0;

    assert(registered_textures[id].tex.id);
    return &registered_textures[id].tex;
}

spritesheet_id_t get_spritesheet(const char *key)
{
    for (int i = 0; i < spritesheet_count; ++i)
    {
        if (strcmp(key, spritesheet_array[i].key) == 0)
            return i;
    }

    trace_log(LOG_WARNING, "Spritesheet '%s' not found", key);

    return (spritesheet_id_t)-1;
}

void release_texture(texture_id_t id)
{
    assert(id < MAX_REGISTERED_TEXTURES && id != INVALID_TEXTURE_ID);

    if (!registered_textures[id].tex.id)
        return;
    unload_texture(registered_textures[id].tex);
    registered_textures[id].tex.id = 0;
}
