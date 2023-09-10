#include "draw/shader_fx.h"

#include <rlgl.h>
#include <stdbool.h>
#include <assert.h>

#include "thread_support.h"

#include "gameplay/gamestate.h"
#include "sys/cleanup.h"
#include "sys/log.h"
#include "sys/alloc.h"
#include "draw/drawlist.h"
#include "draw/draw.h"
#include "draw/drawlist.h"

#include "utils/resource_pool.h"

static Texture2D blank_texture;

typedef struct shader_entry_t
{
    Shader rlgl_shader;

    bool loaded;
    bool joined;
    thrd_t loading_thrd;

    bool enabled;
    bool ignore_global_transformations;
    ShaderType type;

    float time_acc;
    float duration;
    // shader locations
    int time_loc;
    int ratio_loc;
    int player_pos_loc;
    int zorder;
} shader_entry_t;

DEFINE_RESOURCE_POOL(shader_pool, shader_entry_t, MAX_SHADER_FXS);

void cleanup_shader_fx();

void init_shader_fx()
{
    register_cleanup(&cleanup_shader_fx, GamestateEnd);

    Image imBlank = GenImageColor(global_state.game_area_size.x, global_state.game_area_size.y, BLANK);
    blank_texture = LoadTextureFromImage(imBlank);  // Load blank texture to fill on shader
    UnloadImage(imBlank);
}

void cleanup_shader_fx()
{
    FOR_EACH_RES(shader_pool, id, shader_entry_t*, entry)
    {
        unload_shader(id);
    }
    res_pool_clear(&shader_pool);
    UnloadTexture(blank_texture);
}

void update_shader_fx(float dt)
{
    FOR_EACH_RES(shader_pool, id, shader_entry_t*, entry)
    {
        if (entry->enabled)
        {
            entry->time_acc += dt;
            entry->duration -= dt;
            vector2d_t norm_pos = global_state.player.pos;
            norm_pos.x /= global_state.game_area_size.x; norm_pos.y /= global_state.game_area_size.y;
            float ratio = (float)(global_state.game_area_size.x)/(float)(global_state.game_area_size.y);
            SetShaderValue(entry->rlgl_shader, entry->time_loc, &entry->time_acc, SHADER_UNIFORM_FLOAT);
            SetShaderValue(entry->rlgl_shader, entry->ratio_loc, &ratio, SHADER_UNIFORM_FLOAT);
            SetShaderValue(entry->rlgl_shader, entry->player_pos_loc, &norm_pos, SHADER_UNIFORM_VEC2);

            if (entry->duration <= 0.0f)
                disable_shader(id);
        }
    }
}

static void join_shader_loading(shader_entry_t* shader)
{
    if (shader->joined)
        return;

    if (!shader->loaded)
        trace_log(LOG_INFO, "Joining on shader loading thread\n");

    thrd_join(shader->loading_thrd, NULL);
    assert(shader->loaded);
    shader->joined = true;
}

void enable_shader(shader_fx_id_t id, float duration, int zorder, bool ignore_global_transformations)
{
    shader_entry_t* entry = res_pool_get(&shader_pool, id);
    if (!entry)
        return;

    join_shader_loading(entry);

    entry->enabled = true;
    entry->time_acc = 0.0f;
    entry->duration = duration;
    entry->zorder = zorder;
    entry->ignore_global_transformations = ignore_global_transformations;

    vector2d_t norm_pos = global_state.player.pos;
    norm_pos.x /= global_state.game_area_size.x; norm_pos.y /= global_state.game_area_size.y;
    float ratio = (float)(global_state.game_area_size.x)/(float)(global_state.game_area_size.y);
    SetShaderValue(entry->rlgl_shader, entry->time_loc, &entry->time_acc, SHADER_UNIFORM_FLOAT);
    SetShaderValue(entry->rlgl_shader, entry->ratio_loc, &ratio, SHADER_UNIFORM_FLOAT);
    SetShaderValue(entry->rlgl_shader, entry->player_pos_loc, &norm_pos, SHADER_UNIFORM_VEC2);
}

void disable_shader(shader_fx_id_t id)
{
    shader_entry_t* entry = res_pool_get(&shader_pool, id);
    assert(entry);

    entry->enabled = false;
}


static void impl_load_shader(shader_entry_t* entry, const char *shader_path)
{
    char* fShaderStr = LoadFileText(shader_path);
    entry->rlgl_shader = LoadShaderFromMemory(NULL, fShaderStr);
    danpa_free(fShaderStr);

    entry->time_loc = GetShaderLocation(entry->rlgl_shader, "uTime");
    entry->ratio_loc = GetShaderLocation(entry->rlgl_shader, "uAspectRatio");
    entry->player_pos_loc = GetShaderLocation(entry->rlgl_shader, "uPos");
    entry->loaded = true;
}

shader_fx_id_t load_shader(const char *shader_path, ShaderType type)
{
    res_id_t id = res_pool_alloc(&shader_pool);
    if (id == INVALID_RES_ID)
        return id;

    shader_entry_t* entry = res_pool_get(&shader_pool, id);
    entry->enabled = false;
    entry->type = type;

    impl_load_shader(entry, shader_path);
    entry->joined = true;

    return id;
}

shader_fx_id_t load_shader_defer(const char *shader_path, ShaderType type)
{
    // FIXME : actually implement deferred shader loading
    return load_shader(shader_path, type);
}

void draw_shader_callback(void* shader_ptr)
{
    shader_entry_t* shader = shader_ptr;
    assert(shader->enabled);

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
    FOR_EACH_RES(shader_pool, id, shader_entry_t*, shader)
    {
        if (shader->enabled)
        {
            register_draw_element(shader, draw_shader_callback, shader->zorder);
        }
    }
}

void unload_shader(shader_fx_id_t id)
{
    shader_entry_t* shader = res_pool_get(&shader_pool, id);
    if (!shader)
        return;

    join_shader_loading(shader);
    UnloadShader(shader->rlgl_shader);
}
