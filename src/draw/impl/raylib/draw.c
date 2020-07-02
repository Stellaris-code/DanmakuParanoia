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

#include <raylib.h>
#include <rlgl.h>

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

void draw_sprite_batch(texture_t texture, sprite_list_entry_t *list, unsigned list_size)
{
#if 0
    for (unsigned i = 0; i < list_size; ++i)
    {
        Vector2  origin = {list[i].size.x/2, list[i].size.y/2};
        Rectangle  dest_rect = {list[i].pos.x, list[i].pos.y,
                               list[i].size.x, list[i].size.y};
        Rectangle  src_rect;
        src_rect.x = list[i].spriteframe.x; src_rect.y = list[i].spriteframe.y;
        src_rect.width = list[i].spriteframe.w; src_rect.height = list[i].spriteframe.h;

        Color rlcol;
        rlcol.a = list[i].tint.a;
        rlcol.r = list[i].tint.r;
        rlcol.g = list[i].tint.g;
        rlcol.b = list[i].tint.b;

        Texture rltex;
        rltex.id = texture.id;
        rltex.width = texture.width;
        rltex.height = texture.height;
        DrawTexturePro(rltex, src_rect, dest_rect, origin, RAD2DEG*list[i].angle, rlcol);
    }
#else // buggy path right now
          if (texture.id <= 0)
        return;

    float inv_width = 1.f/(float)texture.width;
    float inv_height = 1.f/(float)texture.height;

    // assuming the texture_rect entry is sane (no negative components)

    rlEnableTexture(texture.id);

    rlBegin(RL_QUADS);
    rlPushMatrix();
    for (unsigned i = 0; i < list_size; ++i)
    {
        if (i % 1024 == 0)
        {
            // clear the element buffers
            //rlPopMatrix();
            rlEnd();
            rlDisableTexture();
            rlglDraw();
            rlEnableTexture(texture.id);
            rlBegin(RL_QUADS);
            //rlPushMatrix();
        }

        Vector2  origin = {list[i].size.x/2, list[i].size.y/2};
        Rectangle  dest_rect = {list[i].pos.x, list[i].pos.y,
                               list[i].size.x, list[i].size.y};
        Rectangle  src_rect;
        src_rect.x = list[i].spriteframe.x*inv_width; src_rect.width = list[i].spriteframe.w*inv_width;
        src_rect.y = list[i].spriteframe.y*inv_height; src_rect.height = list[i].spriteframe.h*inv_height;

        rlLoadIdentity();
        rlResetNoTransform();

        if (list[i].angle != 0.0f)
        {
            rlRotatef(RAD2DEG*list[i].angle, 0.0f, 0.0f, 1.0f);
            rlTranslatef(-origin.x, -origin.y, 0.0f);
        }
        else
        {
            dest_rect.x -= origin.x;
            dest_rect.y -= origin.y;

            rlSetNoTransform(); // transform is identity matrix, don't bother transforming vertices
        }

        rlgl_draw_quad(dest_rect, src_rect, (Color){list[i].tint.r, list[i].tint.g, list[i].tint.b, list[i].tint.a});
    }
    rlPopMatrix();
    rlEnd();

    rlDisableTexture();
#endif
}

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
    rlglDraw();
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity(); // force the texture to render over the viewport, ignoring transforms
}

void ignore_viewport_transformations()
{
    rlglDraw();
    rlPushMatrix();
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
}

void restore_viewport_transformations()
{
    rlglDraw();
    rlPopMatrix();
}
