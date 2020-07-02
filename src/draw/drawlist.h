#ifndef DRAWLIST_H
#define DRAWLIST_H

#define MAX_DRAWLIST_SIZE 65536

void register_draw_element(void* data_ptr, void (*draw_callback)(void*), int z_order);
void sort_drawlist();
void commit_drawlist();

#endif // DRAWLIST_H
