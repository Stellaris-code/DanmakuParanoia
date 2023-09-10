#include "draw/texture.h"

#include "gameplay/gamestate.h"
#include "sys/cleanup.h"

#include <raylib.h>

static RenderTexture2D render_texture;

void cleanup_textures()
{
    UnloadRenderTexture(render_texture);
}

void init_textures()
{
    // init the FBO used for postfx
    render_texture = LoadRenderTexture(global_state.game_area_size.x, global_state.game_area_size.y);

    register_cleanup(cleanup_textures, AppEnd);
}

texture_t load_texture(const char *filepath)
{
    Texture2D rltex = LoadTexture(filepath);
    texture_t tex;
    tex.id = rltex.id;
    tex.width = rltex.width;
    tex.height = rltex.height;
    tex.mipmaps = rltex.mipmaps;
    tex.format = rltex.format;

    return tex;
}

void unload_texture(texture_t tex)
{
    Texture2D rltex;
    rltex.id = tex.id;
    UnloadTexture(rltex);
}

texture_t capture_framebuffer()
{
    Rectangle game_area_rect;
    game_area_rect.x = game_area_rect.y = 0;
    game_area_rect.width = global_state.game_area_size.x;
    game_area_rect.height = global_state.game_area_size.y;
    //BlitRenderTexture(render_texture, game_area_rect, game_area_rect);

    texture_t tex;
    tex.id = render_texture.texture.id;
    tex.width = render_texture.texture.width;
    tex.height = -render_texture.texture.height; // take in account OpenGL coordinates
    tex.mipmaps = render_texture.texture.mipmaps;
    tex.format = render_texture.texture.format;

    return tex;
}
