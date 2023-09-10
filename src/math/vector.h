#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

typedef struct vector2d_t
{
    float x, y;
} vector2d_t;

typedef struct ivector2d_t
{
    int x, y;
} ivector2d_t;

typedef struct vector3d_t
{
    float x, y, z;
} vector3d_t;

typedef struct rect_t
{
    float x, y, w, h;
} rect_t;

typedef struct irect_t
{
    int x, y, w, h;
} irect_t;

typedef struct usrect_t
{
    unsigned short x, y, w, h;
} usrect_t;

static float squared_length(vector2d_t vec)
{
    return vec.x*vec.x + vec.y*vec.y;
}

static float length(vector2d_t vec)
{
    return sqrt(squared_length(vec));
}

static vector2d_t normalize(vector2d_t vec)
{
    float norm = length(vec);
    vector2d_t unit;
    unit.x = vec.x/norm;
    unit.y = vec.y/norm;

    return unit;
}

static vector2d_t ortho(vector2d_t vec)
{
    vector2d_t result;
    result.x = -vec.y;
    result.y = vec.x;

    return result;
}

static float dot(vector2d_t lhs, vector2d_t rhs)
{
    return lhs.x*rhs.x + lhs.y*rhs.y;
}

static inline vector2d_t rotate_point(vector2d_t point, float angle, vector2d_t pivot)
{
    void sincosf(float x, float *sin, float *cos);
    float s;
    float c;
    sincosf(angle, &s, &c);

    // translate point back to origin:
    point.x -= pivot.x;
    point.y -= pivot.y;

    // rotate point
    float xnew = point.x * c - point.y * s;
    float ynew = point.x * s + point.y * c;

    // translate point back:
    point.x = xnew + pivot.x;
    point.y = ynew + pivot.y;
    return point;
}

#endif // VECTOR_H
