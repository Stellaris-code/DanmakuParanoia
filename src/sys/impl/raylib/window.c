#include "sys/window.h"

#include <raylib.h>

ivector2d_t get_window_size()
{
    ivector2d_t vec;
    vec.x = GetScreenWidth();
    vec.y = GetScreenHeight();

    return vec;
}
