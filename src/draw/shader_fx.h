#ifndef SHADER_FX_H
#define SHADER_FX_H

#include <stdbool.h>

#define INVALID_SHADER_FX_ID (shader_fx_id_t)(-1)
#define MAX_SHADER_FXS 256

typedef int shader_fx_id_t;

typedef enum ShaderType
{
    NormalShader,
    PostFXShader
} ShaderType;

void init_shader_fx();
void update_shader_fx(float dt);
void draw_shader_fx();
shader_fx_id_t load_shader(const char* shader_path, ShaderType type);
void enable_shader(shader_fx_id_t id, float duration, int zorder, bool ignore_global_transformations);
void disable_shader(shader_fx_id_t id);
void unload_shader(shader_fx_id_t id);

#endif // SHADER_FX_H
