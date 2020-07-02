#ifndef VM_DRAWLIST_H
#define VM_DRAWLIST_H

#include "draw/animation.h"

#define MAX_VM_DRAWLIST_SIZE 65536
#define MAX_VM_ANIM_DRAWLIST_SIZE 4096

void vm_drawlist_reset();
void vm_drawlist_register_anim(const animation_t* anim, vector2d_t pos, vector2d_t size, float angle, int zorder);
void vm_drawlist_register_sprite(sprite_frame_id_t id, vector2d_t pos, vector2d_t size, float angle, int zorder);

#endif // VM_DRAWLIST_H
