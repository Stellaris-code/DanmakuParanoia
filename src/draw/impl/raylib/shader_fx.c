#include "draw/shader_fx.h"

#include <rlgl.h>
#include <stdbool.h>
#include <assert.h>

#include "gameplay/gamestate.h"
#include "sys/cleanup.h"
#include "draw/drawlist.h"
#include "draw/draw.h"
#include "draw/drawlist.h"

static Texture2D blank_texture;

typedef struct shader_entry_t
{
    Shader rlgl_shader;
    bool loaded;
    bool enabled;

    ShaderType type;
    bool ignore_global_transformations;
    int zorder;
    float time_acc;
    float duration;
    // shader locations
    int time_loc;
    int ratio_loc;
    int player_pos_loc;
} shader_entry_t;

static shader_entry_t shader_fxs[MAX_SHADER_FXS];

void cleanup_shader_fx();

void init_shader_fx()
{
    register_cleanup(&cleanup_shader_fx);

    Image imBlank = GenImageColor(global_state.game_area_size.x, global_state.game_area_size.y, BLANK);
    blank_texture = LoadTextureFromImage(imBlank);  // Load blank texture to fill on shader
    UnloadImage(imBlank);
}

void cleanup_shader_fx()
{
    for (int i = 0; i < MAX_SHADER_FXS; ++i)
    {
        if (shader_fxs[i].loaded)
        {
            unload_shader(i);
        }
    }
    UnloadTexture(blank_texture);
}

void update_shader_fx(float dt)
{
    for (int i = 0; i < MAX_SHADER_FXS; ++i)
    {
        if (shader_fxs[i].loaded && shader_fxs[i].enabled)
        {
            shader_fxs[i].time_acc += dt;
            shader_fxs[i].duration -= dt;
            vector2d_t norm_pos = global_state.player.pos;
            norm_pos.x /= global_state.game_area_size.x; norm_pos.y /= global_state.game_area_size.y;
            float ratio = (float)(global_state.game_area_size.x)/(float)(global_state.game_area_size.y);
            SetShaderValue(shader_fxs[i].rlgl_shader, shader_fxs[i].time_loc, &shader_fxs[i].time_acc, UNIFORM_FLOAT);
            SetShaderValue(shader_fxs[i].rlgl_shader, shader_fxs[i].ratio_loc, &ratio, UNIFORM_FLOAT);
            SetShaderValue(shader_fxs[i].rlgl_shader, shader_fxs[i].player_pos_loc, &norm_pos, UNIFORM_VEC2);

            if (shader_fxs[i].duration <= 0.0f)
                disable_shader(i);
        }
    }
}

void enable_shader(shader_fx_id_t id, float duration, int zorder, bool ignore_global_transformations)
{
    assert(id != INVALID_SHADER_FX_ID);
    assert(id < MAX_SHADER_FXS);

    shader_fxs[id].enabled = true;
    shader_fxs[id].time_acc = 0.0f;
    shader_fxs[id].duration = duration;
    shader_fxs[id].zorder = zorder;
    shader_fxs[id].ignore_global_transformations = ignore_global_transformations;

    vector2d_t norm_pos = global_state.player.pos;
    norm_pos.x /= global_state.game_area_size.x; norm_pos.y /= global_state.game_area_size.y;
    float ratio = (float)(global_state.game_area_size.x)/(float)(global_state.game_area_size.y);
    SetShaderValue(shader_fxs[id].rlgl_shader, shader_fxs[id].time_loc, &shader_fxs[id].time_acc, UNIFORM_FLOAT);
    SetShaderValue(shader_fxs[id].rlgl_shader, shader_fxs[id].ratio_loc, &ratio, UNIFORM_FLOAT);
    SetShaderValue(shader_fxs[id].rlgl_shader, shader_fxs[id].player_pos_loc, &norm_pos, UNIFORM_VEC2);
}

void disable_shader(shader_fx_id_t id)
{
    assert(id != INVALID_SHADER_FX_ID);
    assert(id < MAX_SHADER_FXS);

    shader_fxs[id].enabled = false;
}

shader_fx_id_t load_shader(const char *shader_path, ShaderType type)
{
    for (int i = 0; i < MAX_SHADER_FXS; ++i)
    {
        if (!shader_fxs[i].loaded)
        {
            shader_fxs[i].rlgl_shader = LoadShader(NULL, shader_path);
            shader_fxs[i].time_loc = GetShaderLocation(shader_fxs[i].rlgl_shader, "uTime");
            shader_fxs[i].ratio_loc = GetShaderLocation(shader_fxs[i].rlgl_shader, "uAspectRatio");
            shader_fxs[i].player_pos_loc = GetShaderLocation(shader_fxs[i].rlgl_shader, "uPos");
            shader_fxs[i].type = type;

            shader_fxs[i].loaded  = true;
            shader_fxs[i].enabled = false;

            return i;
        }
    }

    return INVALID_SHADER_FX_ID;
}

void draw_shader_callback(void* shader_ptr)
{
    shader_entry_t* shader = shader_ptr;
    assert(shader->loaded && shader->enabled);

    rect_t game_area_rect;
    game_area_rect.x = game_area_rect.y = 0;
    game_area_rect.w = global_state.game_area_size.x;
    game_area_rect.h = global_state.game_area_size.y;

    texture_t postfx_texture;
    if (shader->type == PostFXShader)
    {
        postfx_texture = capture_framebuffer();
    }

    BeginShaderMode(shader->rlgl_shader);    // Enable our custom shader for next shapes/textures drawings

    // PostFX shaders aren't affected by transformations by design
    if (shader->type != PostFXShader && shader->ignore_global_transformations)
        ignore_viewport_transformations();

    if (shader->type == PostFXShader)
        draw_textured_rect(postfx_texture, game_area_rect, game_area_rect, (vector2d_t){0, 0}, 0.0f, COL_WHITE);
    else
        DrawTexture(blank_texture, 0, 0, WHITE);  // Drawing BLANK texture, all magic happens on shader

    if (shader->type != PostFXShader && shader->ignore_global_transformations)
        restore_viewport_transformations();

    EndShaderMode();            // Disable our custom shader, return to default shader
}

void draw_shader_fx()
{
    for (int i = 0; i < MAX_SHADER_FXS; ++i)
    {
        if (shader_fxs[i].loaded && shader_fxs[i].enabled)
        {
            register_draw_element(&shader_fxs[i], draw_shader_callback, shader_fxs[i].zorder);
        }
    }
}

void unload_shader(shader_fx_id_t id)
{
    assert(id != INVALID_SHADER_FX_ID);
    assert(id < MAX_SHADER_FXS);

    if (!shader_fxs[id].loaded)
        return;
    shader_fxs[id].loaded = false;
    UnloadShader(shader_fxs[id].rlgl_shader);
}
