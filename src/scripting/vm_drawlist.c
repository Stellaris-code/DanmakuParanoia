#include "vm_drawlist.h"

#include "draw/drawlist.h"
#include "draw/draw.h"

typedef struct vm_sprite_entry_t
{
    sprite_frame_id_t id;
    vector2d_t pos, size;
    float angle;
} vm_sprite_entry_t;

typedef struct vm_anim_entry_t
{
    animation_t anim;
    vector2d_t pos, size;
    float angle;
} vm_anim_entry_t;

static vm_sprite_entry_t sprites[MAX_VM_DRAWLIST_SIZE];
static vm_anim_entry_t anims[MAX_VM_ANIM_DRAWLIST_SIZE];
static int sprite_count;
static int anim_count;

static void draw_anim_callback(void* ptr)
{
    vm_anim_entry_t* anim = ptr;

    draw_animation(&anim->anim, anim->pos, anim->size, anim->angle);
}

static void draw_sprite_callback(void* ptr)
{
    vm_sprite_entry_t* sprite = ptr;

    draw_sprite(sprite->id, sprite->pos, sprite->size, sprite->angle, COL_WHITE);
}

void vm_drawlist_register_anim(const animation_t *anim, vector2d_t pos, vector2d_t size, float angle, int zorder)
{
    if (anim_count > MAX_VM_ANIM_DRAWLIST_SIZE)
        return;

    anims[anim_count].anim = *anim;
    anims[anim_count].pos = pos;
    anims[anim_count].size = size;
    anims[anim_count].angle = angle;

    register_draw_element(&anims[anim_count], draw_anim_callback, zorder);

    ++anim_count;
}

void vm_drawlist_register_sprite(sprite_frame_id_t id, vector2d_t pos, vector2d_t size, float angle, int zorder)
{
    if (sprite_count > MAX_VM_DRAWLIST_SIZE)
        return;

    sprites[sprite_count].id = id;
    sprites[sprite_count].pos = pos;
    sprites[sprite_count].size = size;
    sprites[sprite_count].angle = angle;

    register_draw_element(&sprites[sprite_count], draw_sprite_callback, zorder);

    ++sprite_count;
}

void vm_drawlist_reset()
{
    anim_count = sprite_count = 0;
}
