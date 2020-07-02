#ifndef MOTION_H
#define MOTION_H

#include <stdbool.h>

#include "math/vector.h"

#define MAX_MOTION_HIERARCHY_DEPTH 8

// 64 Bytes
typedef struct motion_data_t
{
    float relative_x;
    float relative_y;

    float velocity;
    float acceleration;
    float direction_angle;
    float angular_velocity;
    float rotation;
    float rotational_speed;
    float rotational_acceleration;

    float dampening;

    float max_speed;
    float max_accel;
    float max_rot;
    float max_angular;

    struct motion_data_t* root; // NULL if absolute movement
} motion_data_t;

void update_motion(float dt, motion_data_t* data);

vector2d_t absolute_pos(const motion_data_t* data);

typedef struct json_value_t json_value_t;
typedef struct json_context_t json_context_t;
void load_motion_from_json(motion_data_t* data, json_value_t* json, json_context_t* context);

#endif // MOTION_H
