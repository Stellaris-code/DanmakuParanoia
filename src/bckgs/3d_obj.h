#ifndef OBJ3D_H
#define OBJ3D_H

#include "math/vector.h"
#include "resources/texture_handler.h"

#define MAX_3DOBJS 1024
#define INVALID_3DOBJ_ID 0xffffffff

typedef uint32_t obj3d_id_t;

typedef struct obj3d_t
{
    vector3d_t pos;
    vector3d_t orientation;
    vector3d_t scale;
    texture_id_t  texture;
    void* model_data;
} obj3d_t;

void init_obj3ds();
void cleanup_obj3ds();
obj3d_id_t load_obj3d(const char* mdl_path);
obj3d_t *get_obj3d_ref(obj3d_id_t id);
void draw_obj3d(obj3d_id_t id);
void draw_all_obj3d();
void delete_obj3d(obj3d_id_t id);

#endif // OBJ3D_H
