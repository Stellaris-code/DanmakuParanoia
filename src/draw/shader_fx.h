#ifndef SHADER_FX_H
#define SHADER_FX_H

#include <stdbool.h>
#include "utils/resource_pool.h"

#define INVALID_SHADER_FX_ID INVALID_RES_ID
#define MAX_SHADER_FXS 256

typedef res_id_t shader_fx_id_t;

typedef enum ShaderType
{
    NormalShader,
    PostFXShader
} ShaderType;

void init_shader_fx();
void update_shader_fx(float dt);
void draw_shader_fx();
shader_fx_id_t load_shader(const char* shader_path, ShaderType type);
// load the shader asynchronously, ensuring it's loaded and ready when needed
shader_fx_id_t load_shader_defer(const char* shader_path, ShaderType type);
void enable_shader(shader_fx_id_t id, float duration, int zorder, bool ignore_global_transformations);
void disable_shader(shader_fx_id_t id);
void unload_shader(shader_fx_id_t id);

#endif // SHADER_FX_H
