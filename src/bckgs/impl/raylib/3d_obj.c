#include "bckgs/3d_obj.h"

#include <assert.h>
#include <string.h>

#include <raylib.h>

#include "sys/cleanup.h"

static Model   raylib_models[MAX_3DOBJS];
static obj3d_t obj3d_entries[MAX_3DOBJS];

obj3d_id_t load_obj3d(const char* mdl_path)
{
    for (int i = 0; i < MAX_3DOBJS; ++i)
    {
        if (!obj3d_entries[i].model_data) // free entry
        {
            raylib_models[i] = LoadModel(mdl_path);

            memset(&obj3d_entries[i], 0, sizeof(obj3d_t));
            obj3d_entries[i].scale = (vector3d_t){1.0f, 1.0f, 1.0f};
            obj3d_entries[i].model_data = &raylib_models[i];
            return i;
        }
    }

    return INVALID_3DOBJ_ID;
}

obj3d_t *get_obj3d_ref(obj3d_id_t id)
{
    if (id >= MAX_3DOBJS || id == INVALID_3DOBJ_ID)
        return 0;
    return &obj3d_entries[id];
}

void delete_obj3d(obj3d_id_t id)
{
    assert(id < MAX_3DOBJS && id != INVALID_3DOBJ_ID);

    UnloadModel(raylib_models[id]);
    obj3d_entries[id].model_data = 0;
}

void draw_obj3d(obj3d_id_t id)
{
    if (id >= MAX_3DOBJS || id == INVALID_3DOBJ_ID)
        return;

    Vector3 pos = (Vector3){obj3d_entries[id].pos.x, obj3d_entries[id].pos.y, obj3d_entries[id].pos.z};
    Vector3 angles = (Vector3){obj3d_entries[id].orientation.x, obj3d_entries[id].orientation.y, obj3d_entries[id].orientation.z};
    Vector3 scale = (Vector3){obj3d_entries[id].scale.x, obj3d_entries[id].scale.y, obj3d_entries[id].scale.z};

    texture_t* ptr = retrieve_texture(obj3d_entries[id].texture);

    if (ptr)
    {
        Texture tex;
        tex.id = ptr->id;
        tex.width = ptr->width;
        tex.height = ptr->height;
        tex.format = ptr->format;
        tex.mipmaps = ptr->mipmaps;
        SetMaterialTexture(&raylib_models[id].materials[0], MAP_DIFFUSE, tex);  // Set model material map texture
    }

    DrawModelExXYZ(raylib_models[id], pos, angles, scale, WHITE);
}

void cleanup_obj3ds()
{
    for (int i = 0; i < MAX_3DOBJS; ++i)
    {
        if (obj3d_entries[i].model_data != NULL) // free entry
        {
            delete_obj3d(i);
        }
    }
}

void init_obj3ds()
{
    register_cleanup(cleanup_obj3ds);
}

void draw_all_obj3d()
{
    for (int i = 0; i < MAX_3DOBJS; ++i)
    {
        if (obj3d_entries[i].model_data) // loaded
        {
            draw_obj3d(i);
        }
    }
}
