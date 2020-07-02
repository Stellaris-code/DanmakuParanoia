#include "drawlist.h"

#include <stdlib.h>

#include "math/vector.h"

typedef struct draw_element_t
{
    void* data_ptr;
    void (*draw_callback)(void*);
    int z_order;
} draw_element_t;

static draw_element_t drawlist[MAX_DRAWLIST_SIZE];
static int drawlist_size;

void register_draw_element(void *data_ptr, void (*draw_callback)(void *), int z_order)
{
    if (drawlist_size >= MAX_DRAWLIST_SIZE)
        return;

    drawlist[drawlist_size].data_ptr = data_ptr;
    drawlist[drawlist_size].draw_callback = draw_callback;
    drawlist[drawlist_size].z_order = z_order;

    ++drawlist_size;
}

int drawlist_compare(const void *l_, const void *r_)
{
    draw_element_t l = *(const draw_element_t *)l_;
    draw_element_t r = *(const draw_element_t *)r_;

    return l.z_order - r.z_order;
}

void sort_drawlist()
{
    qsort(drawlist, drawlist_size, sizeof(draw_element_t), drawlist_compare);
}

void commit_drawlist()
{
    // drawlist is sorted from back to front
    for (int i = 0; i < drawlist_size; ++i)
    {
        drawlist[i].draw_callback(drawlist[i].data_ptr);
    }

    drawlist_size = 0;
}
