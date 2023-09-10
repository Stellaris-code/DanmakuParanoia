/*
draw.c

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

#include "resources/texture_handler.h"
#include "draw/draw.h"
#include "draw/texture.h"

#include <string.h>
#include <limits.h>
#include <raylib.h>
#include <rlgl.h>
#include <external/glad.h>

#include <assert.h>

void draw_textured_rect(texture_t tex, rect_t src_rect, rect_t dest_rect, vector2d_t origin, float angle, color_t tint)
{
    Texture2D rltex;
    rltex.id = tex.id;
    rltex.width = tex.width;
    rltex.height = tex.height;

    Color     rl_col  = {tint.r, tint.g, tint.b, tint.a};

    //printf("draw tex %d, src %f %f %f %f, dst %f %f %f %f %f\n", tex.id, src_rect.x, src_rect.y, src_rect.w, src_rect.h, dest_rect.w, dest_rect.h,
    //       dest_rect.x, dest_rect.y, angle);
    DrawTexturePro(rltex, (Rectangle){.x = src_rect.x, .y = src_rect.y, .width = src_rect.w, .height = src_rect.h},
                   (Rectangle){.x = dest_rect.x, .y = dest_rect.y, .width = dest_rect.w, .height = dest_rect.h},
                   (Vector2){.x = origin.x, .y = origin.y}, RAD2DEG*angle, rl_col);
}

void draw_sprite(sprite_frame_id_t sprite, vector2d_t pos, vector2d_t size, float angle, color_t tint)
{
    texture_t  tex       = get_sprite_frame(sprite).texture;
    rect_t     src_rect  = get_sprite_frame(sprite).spritesheet_rect;

    rect_t  dest_rect = {pos.x, pos.y, size.x, size.y};
    vector2d_t  origin = {size.x/2, size.y/2};
    draw_textured_rect(tex, src_rect, dest_rect, origin, angle, tint);
}
static unsigned spritebatch_tex_id;

#if 0

void begin_draw_sprite_batch(texture_t tex)
{
    assert(tex.id > 0);

    spritebatch_inv_width = 1.f/(float)tex.width;
    spritebatch_inv_height = 1.f/(float)tex.height;

    // assuming the texture_rect entry is sane (no negative components)

    rlSetTexture(tex.id);

    rlBegin(RL_QUADS);
    rlPushMatrix();

    spritebatch_tex_id = tex.id;
}

void draw_sprite_batch_element(vector2d_t size, vector2d_t pos, float angle, color_t tint, rect_t spriteframe)
{
    if (rlCheckRenderBatchLimit(4)) // overflow
        rlSetTexture(spritebatch_tex_id);

    Rectangle  dest_rect = {pos.x, pos.y,
                           size.x, size.y};

    rlgl_draw_quad(dest_rect, (Vector2){size.x/2, size.y/2}, angle, *(Rectangle*)&spriteframe, (Color){tint.r, tint.g, tint.b, tint.a});
}

void end_draw_sprite_batch()
{
    rlPopMatrix();
    rlEnd();

    rlSetTexture(0);
}

#else

static bool batch_vao_init;
static unsigned batch_vao;
static unsigned batch_vbo[4];
static Shader batch_shader;
static texture_t batch_texture;
quad_data_t batch_quad_data[SPRITE_BATCH_ELEMENTS];
static float batch_indexed_texcoords[MAX_SPRITE_FRAMES*2*4];
unsigned batch_elem_count;

#define xstr(a) str(a)
#define str(a) #a

extern int sprite_frame_count;
void begin_draw_sprite_batch(texture_t tex)
{
    assert(tex.id > 0);

    // Force the raylib rendering batch to be flushed first as we will be bypassing this system
    rlDrawRenderBatchActive();

    batch_texture = tex;

    const unsigned bufferElements = SPRITE_BATCH_ELEMENTS;
    if (!batch_vao_init)
    {
        const char* vertex_shader =
            "#version 330                       \n"
            "struct quad_item {\n"
            "   uint world_pos;\n"
            "   uint size;\n"
            "   int spriteid_depth;\n"
            "   float angle;\n"
            "};\n"
            "layout(location = 4) in vec2 sprite_pos;\n"
            "layout (std140) uniform quad_data {\n"
            "   quad_item quad[" xstr(SPRITE_BATCH_ELEMENTS) "];\n"
            "   };\n"
            "layout (std140) uniform frame_data {\n"
            "vec4 texcoords[" xstr(MAX_SPRITE_FRAMES) " * 2];\n"
            "};\n"
            "out vec2 fragTexCoord;             \n"
            "uniform mat4 mvp;                  \n"
            "vec2 uint_to_vec2(in uint pack) {\n"
            " return vec2(float(pack & uint(0xFFFF)),float(pack>>16)); \n"
            "}\n"
            "void main()                        \n"
            "{                                  \n"
            "    int quad_id = gl_VertexID/4;\n"
            "    int sprite_id = quad[quad_id].spriteid_depth & 0xFFFF;\n"
            "    float depth = -float(quad[quad_id].spriteid_depth >> 16)*(1.0f/20000.0f);\n"
            "    vec4 fragTexCoordPacked = texcoords[sprite_id*2 + (gl_VertexID%4)/2]; \n"
            "    fragTexCoord = gl_VertexID%2 == 0 ? fragTexCoordPacked.xy : fragTexCoordPacked.zw;\n"
            "    float denorm_angle = quad[quad_id].angle;\n"
            "    float scale_x = (gl_VertexID%4>1) ? 0.5f : -0.5f;"
            "    float scale_y = (gl_VertexID%4>0 && gl_VertexID%4<3) ? 0.5f : -0.5f;"
            "    vec2 size = uint_to_vec2(quad[quad_id].size);\n"
            "    vec2 local_pos = vec2(size.x*scale_x, size.y*scale_y);"
            "    vec2 world_float = uint_to_vec2(quad[quad_id].world_pos);"
            "vec2 vertexPosition = world_float + vec2(cos(denorm_angle)*local_pos.x - sin(denorm_angle)*local_pos.y, sin(denorm_angle)*local_pos.x + cos(denorm_angle)*local_pos.y);"
            "    gl_Position = mvp*vec4(vertexPosition, 0.0, 1.0); \n"
            "    gl_Position.z = depth;"
            "}                                  \n";

        const char* frag_shader =
            "#version 330       \n"
            "in vec2 fragTexCoord;              \n"
            //            "in vec4 fragColor;                 \n"
            "out vec4 finalColor;               \n"
            "uniform sampler2D texture0;        \n"
            "void main()                        \n"
            "{                                  \n"
            "    vec4 texelColor = texture(texture0, fragTexCoord);   \n"
            "if(texelColor.a < 0.5)"
            "    discard;"
            "    finalColor = texelColor;        \n"
            "}";

        batch_shader = LoadShaderFromMemory(vertex_shader, frag_shader);

        unsigned short *indices = danpa_alloc(SPRITE_BATCH_ELEMENTS*6*sizeof(unsigned short));  // 6 int by quad (indices)

        int k = 0;

        // Indices can be initialized right now
        for (int j = 0; j < (6*bufferElements); j += 6)
        {
            indices[j] = 4*k;
            indices[j + 1] = 4*k + 1;
            indices[j + 2] = 4*k + 2;
            indices[j + 3] = 4*k;
            indices[j + 4] = 4*k + 2;
            indices[j + 5] = 4*k + 3;

            k++;
        }

        //--------------------------------------------------------------------------------------------

        // Upload to GPU (VRAM) vertex data and initialize VAOs/VBOs
        //--------------------------------------------------------------------------------------------
        batch_vao = rlLoadVertexArray();
        rlEnableVertexArray(batch_vao);

        // Fill index buffer
        unsigned indices_vbo =  rlLoadVertexBufferElement(indices, SPRITE_BATCH_ELEMENTS*6*sizeof(unsigned short), false);
        (void)indices_vbo;

        danpa_free(indices);

        unsigned frameGlobalUniformBlockIndex =
            glGetUniformBlockIndex(batch_shader.id, "frame_data");
        glGenBuffers(1,&batch_vbo[2]); // this generates UBO, OK
        glBindBuffer(GL_UNIFORM_BUFFER, batch_vbo[2]); // this binds it, OK
        glBufferData(GL_UNIFORM_BUFFER, sizeof(float)*2*MAX_SPRITE_FRAMES*4, NULL, GL_STATIC_DRAW); // this allocates space for the UBO.
        unsigned bind_buffer_index = 0;
        glUniformBlockBinding(batch_shader.id, frameGlobalUniformBlockIndex,
                              bind_buffer_index);
        glBindBufferRange(GL_UNIFORM_BUFFER, bind_buffer_index, batch_vbo[2], 0, sizeof(float)*2*MAX_SPRITE_FRAMES*4); // this binds UBO to Buffer Index

        unsigned quadGlobalUniformBlockIndex =
            glGetUniformBlockIndex(batch_shader.id, "quad_data");
        glGenBuffers(1,&batch_vbo[3]); // this generates UBO, OK
        glBindBuffer(GL_UNIFORM_BUFFER, batch_vbo[3]); // this binds it, OK
        glBufferData(GL_UNIFORM_BUFFER, sizeof(quad_data_t)*SPRITE_BATCH_ELEMENTS, NULL, GL_STATIC_DRAW); // this allocates space for the UBO.
        bind_buffer_index = 1;
        glUniformBlockBinding(batch_shader.id, quadGlobalUniformBlockIndex,
                              bind_buffer_index);
        glBindBufferRange(GL_UNIFORM_BUFFER, bind_buffer_index, batch_vbo[3], 0, sizeof(quad_data_t)*SPRITE_BATCH_ELEMENTS); // this binds UBO to Buffer Index

        rlDisableVertexArray();

        batch_vao_init = true;
    }

    batch_elem_count = 0;

    spritebatch_tex_id = tex.id;

    rlEnableShader(batch_shader.id);
    Matrix matMVP = MatrixMultiply(rlGetMatrixModelview(), rlGetMatrixProjection());
    rlSetUniformMatrix(batch_shader.locs[SHADER_LOC_MATRIX_MVP], matMVP);

    int texid = 0;
    rlSetUniform(batch_shader.locs[SHADER_LOC_MAP_DIFFUSE], &texid, SHADER_UNIFORM_INT, 1);

    for (int i = 0; i < sprite_frame_count; ++i)
    {
        sprite_frame_t frame = get_sprite_frame(i);
        frame.spritesheet_rect.x /= frame.texture.width;
        frame.spritesheet_rect.w /= frame.texture.width;
        frame.spritesheet_rect.y /= frame.texture.height;
        frame.spritesheet_rect.h /= frame.texture.height;

        batch_indexed_texcoords[i*8+0] = frame.spritesheet_rect.x;
        batch_indexed_texcoords[i*8+1] = frame.spritesheet_rect.y;
        batch_indexed_texcoords[i*8+2] = frame.spritesheet_rect.x;
        batch_indexed_texcoords[i*8+3] = (frame.spritesheet_rect.y + frame.spritesheet_rect.h);
        batch_indexed_texcoords[i*8+4] = (frame.spritesheet_rect.x + frame.spritesheet_rect.w);
        batch_indexed_texcoords[i*8+5] = (frame.spritesheet_rect.y + frame.spritesheet_rect.h);
        batch_indexed_texcoords[i*8+6] = (frame.spritesheet_rect.x + frame.spritesheet_rect.w);
        batch_indexed_texcoords[i*8+7] = frame.spritesheet_rect.y;
    }

    glBindBuffer(GL_UNIFORM_BUFFER, batch_vbo[2]); // this binds it, OK
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float)*sprite_frame_count*8, batch_indexed_texcoords);

    rlActiveTextureSlot(0);
    rlEnableTexture(batch_texture.id);

    glDisable(GL_BLEND);
    rlEnableDepthTest();
    glDepthFunc(GL_LESS);
}



void commit_sprite_batch()
{
    //glEnable(GL_RASTERIZER_DISCARD);
    rlEnableVertexArray(batch_vao);

    glBindBuffer(GL_UNIFORM_BUFFER, batch_vbo[3]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(quad_data_t)*batch_elem_count, batch_quad_data);

    rlDrawVertexArrayElements(0, batch_elem_count*6, 0);

    batch_elem_count = 0;
}

void end_draw_sprite_batch()
{
    commit_sprite_batch();
    rlSetTexture(0);
    glEnable(GL_BLEND);
    glDepthFunc(GL_LEQUAL);
    rlDisableDepthTest();
    //glDisable(GL_RASTERIZER_DISCARD);
}

#endif

void draw_rect(rect_t rect, vector2d_t origin, float angle, color_t color)
{
    Rectangle rl_rect = {rect.x, rect.y, rect.w, rect.h};
    Vector2   rl_vec  = {origin.x, origin.y};
    Color     rl_col  = {color.r, color.g, color.b, color.a};

    DrawRectanglePro(rl_rect, rl_vec, RAD2DEG*angle, rl_col);
}

void draw_sheared_quad(float shearing, vector2d_t vertices[], vector2d_t pos, float angle, color_t color)
{
    rlPushMatrix();
    float m[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        shearing, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    rlMultMatrixf(m);
    rlTranslatef(pos.x, pos.y, 0.0f);
    rlRotatef(RAD2DEG*angle, 0.0f, 0.0f, 1.0f);

    Vector2 rl_vert[4];
    for (int i = 0; i < 4; ++i)
    {
        rl_vert[i].x = vertices[i].x;
        rl_vert[i].y = vertices[i].y;
    }

    Color rl_col;
    rl_col.r = color.r;
    rl_col.g = color.g;
    rl_col.b = color.b;
    rl_col.a = color.a;
    DrawTriangleStrip(rl_vert, 4, rl_col);
    rlPopMatrix();
}

void draw_circle(vector2d_t pos, float radius, color_t color)
{
    Color rl_col;
    rl_col.r = color.r;
    rl_col.g = color.g;
    rl_col.b = color.b;
    rl_col.a = color.a;
    Vector2 rl_vec;
    rl_vec.x = pos.x;
    rl_vec.y = pos.y;
    int segments = 36;
    if (radius <= 5.0f) // avoid rendering issues
        segments = 8;
    DrawCircleSector(rl_vec, radius, 0, 360, segments, rl_col);
}

void translate_viewport(vector3d_t offset)
{
    rlTranslatef(offset.x, offset.y, offset.z);
}

void reset_viewport()
{
    rlDrawRenderBatchActive();
    //rlglDraw();
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity(); // force the texture to render over the viewport, ignoring transforms
}

void ignore_viewport_transformations()
{
    rlDrawRenderBatchActive();
    //rlglDraw();
    rlPushMatrix();
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
}

void restore_viewport_transformations()
{
    rlDrawRenderBatchActive();
    //rlglDraw();
    rlPopMatrix();
}
