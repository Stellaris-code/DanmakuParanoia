#include "bckgs/skybox.h"

#include "sys/cleanup.h"

#include <raylib.h>
#include <rlgl.h>

static Mesh cube;
static Model skybox_model;
static TextureCubemap skybox_cubemap;

void load_skybox(const char *cubemap_path)
{
    reset_skybox();

    // init skybox models
    cube = GenMeshCube(1.0f, 1.0f, 1.0f);
    skybox_model = LoadModelFromMesh(cube);

    skybox_model.materials[0].shader = LoadShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    SetShaderValue(skybox_model.materials[0].shader, GetShaderLocation(skybox_model.materials[0].shader, "environmentMap"), (int[1]){ MAP_CUBEMAP }, UNIFORM_INT);

    Image skybox_image = LoadImage("resources/skybox.png");
    skybox_cubemap = LoadTextureCubemap(skybox_image, CUBEMAP_AUTO_DETECT, 0);
    skybox_model.materials[0].maps[MAP_CUBEMAP].texture = skybox_cubemap;

    UnloadImage(skybox_image);
}

void reset_skybox()
{
    if (cube.vertexCount != 0)
    {
        UnloadShader(skybox_model.materials[0].shader);
        UnloadTexture(skybox_cubemap);
        UnloadModel(skybox_model);
    }
}

void draw_skybox()
{
    rlDisableDepthTest();
    rlDisableBackfaceCulling();

    DrawModel(skybox_model, (Vector3){0, 0, 0}, 1.0f, WHITE);

    rlEnableDepthTest();
    rlEnableBackfaceCulling();
}

void init_skybox()
{
    register_cleanup(reset_skybox);
}
