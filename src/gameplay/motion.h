#ifndef MOTION_H
#define MOTION_H

#include <stdbool.h>

#include "math/vector.h"

#include "simd/simd.h"

typedef int32_t motion_id_t;

#define MAX_MOTION_COUNT 65536
#define INVALID_MOTION_ID ((motion_id_t)-1)
#define MAX_MOTION_HIERARCHY_DEPTH 8

typedef struct motion_data_t
{
    float acceleration;
    float angular_velocity;
    float rotational_speed;
    float velocity;

    float max_speed;
    float max_accel;
    float max_rot;
    float max_angular;

    float direction_angle;
    float rotation;
    float rotational_acceleration;
    float dampening;

    float relative_x;
    float relative_y;

    struct motion_data_t* root; // NULL if absolute movement

    // impl details
    float cached_direction_angle;
    float movement_x;
    float movement_y;
} motion_data_t;

typedef struct packed_motion_t
{
    _Alignas(VECTOR_WIDTH) float acceleration[SIMD_FLT_PER_REG];
    _Alignas(VECTOR_WIDTH) float angular_velocity[SIMD_FLT_PER_REG];
    _Alignas(VECTOR_WIDTH) float rotational_speed[SIMD_FLT_PER_REG];
    _Alignas(VECTOR_WIDTH) float velocity[SIMD_FLT_PER_REG];

    _Alignas(VECTOR_WIDTH) float max_speed[SIMD_FLT_PER_REG];

    _Alignas(VECTOR_WIDTH) float direction_angle[SIMD_FLT_PER_REG];
    _Alignas(VECTOR_WIDTH) float rotation[SIMD_FLT_PER_REG];
    _Alignas(VECTOR_WIDTH) float dampening[SIMD_FLT_PER_REG];

    _Alignas(VECTOR_WIDTH) float relative_x[SIMD_FLT_PER_REG];
    _Alignas(VECTOR_WIDTH) float relative_y[SIMD_FLT_PER_REG];
} packed_motion_t;

void update_motion(float dt, motion_data_t* data);
void motion_simd_load(packed_motion_t* dst, unsigned load_idx, const motion_data_t* src);

vector2d_t absolute_pos(const motion_data_t* data);

typedef struct json_value_t json_value_t;
typedef struct json_context_t json_context_t;
void load_motion_from_json(motion_data_t* data, json_value_t* json, json_context_t* context);

#include "motion_simd.h"

#endif // MOTION_H
