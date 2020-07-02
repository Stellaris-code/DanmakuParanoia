#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>

#include "math/vector.h"

#define SWAP(x, y) \
    do { \
    __typeof__(x) tmp = x; \
    x = y; \
    y = tmp; \
    } while (0)

#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define MIN(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

static inline int ilerp(int startValue, int endValue, int stepNumber, int lastStepNumber)
{
    return (endValue - startValue) * stepNumber / lastStepNumber + startValue;
}
static inline float lerp(float a, float b, float t)
{
    return b*t + a*(1-t);
}
static inline vector2d_t vec2d_lerp(vector2d_t a, vector2d_t b, float t)
{
    vector2d_t vec;
    vec.x = lerp(a.x, b.x, t);
    vec.y = lerp(a.y, b.y, t);
    return vec;
}
static inline float randf(float upper)
{
    return (float)rand()/(float)(RAND_MAX/upper);
}

static inline float randf_bounds( float min, float max )
{
    float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
    return min + scale * ( max - min );      /* [min, max] */
}



#endif // UTILS_H
