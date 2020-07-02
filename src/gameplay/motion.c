#include "motion.h"

#include <math.h>
#include <assert.h>

#include "sys/log.h"
#include "utils/json.h"
#include "utils/utils.h"
#include "entity.h"

#include <xmmintrin.h>

static const float twopi=6.283185f;
static const float threehalfpi=4.7123889f;
static const float pi=3.141593f;
static const float halfpi=1.570796f;

static float fast_cos_32s(float x)
{
    const float c1= 0.99940307f;
    const float c2=-0.49558072f;
    const float c3= 0.03679168f;
    float x2;      // The input argument squared
    x2=x * x;
    return (c1 + x2*(c2 + c3 * x2));
}
static void fast_sincos(float angle, float *sin, float *cos){
    //clamp to the range 0..2pi
    //angle=angle-floorf(angle*invtwopi)*twopi;
    float sinmultiplier=angle<pi?1.f:-1.f;
    //angle=angle>0.f?angle:-angle;

    if(angle<halfpi) {
        *cos=fast_cos_32s(angle);
        *sin=sinmultiplier*sqrtf(1.f-*cos**cos);
        return;
    }
    if(angle<pi) {
        *cos=-fast_cos_32s(pi-angle);
        *sin=sinmultiplier*sqrtf(1.f-*cos**cos);
        return;
    }
    if(angle<threehalfpi) {
        *cos=-fast_cos_32s(angle-pi);
        *sin=sinmultiplier*sqrtf(1.f-*cos**cos);
        return;
    }
    *cos=fast_cos_32s(twopi-angle);
    *sin=sinmultiplier*sqrtf(1.f-*cos**cos);
    return;
}

void update_motion(float dt, motion_data_t* data)
{
    data->angular_velocity = MIN(data->max_angular, data->angular_velocity);
    data->acceleration     = MIN(data->max_accel, data->acceleration);
    data->rotational_speed = MIN(data->max_rot, data->rotational_speed + data->rotational_acceleration*dt);
    data->velocity         = MIN(data->max_speed, (data->velocity + data->acceleration*dt)*(1.0f-data->dampening));

    data->direction_angle  = data->direction_angle + data->angular_velocity*dt;
    while (data->direction_angle < 0)
        data->direction_angle = M_PI*2 + data->direction_angle;
    // clamp the direction angle range
    while (data->direction_angle > M_PI*2)
        data->direction_angle -= M_PI*2;

    data->rotation += data->rotational_speed*dt;

    float movement_x, movement_y;
    fast_sincos(data->direction_angle, &movement_y, &movement_x);
    //sincosf(data->direction_angle, &movement_y, &movement_x);
    movement_x *= data->velocity*dt;
    movement_y *= data->velocity*dt;

    data->relative_x += movement_x;
    data->relative_y += movement_y;
}

static vector2d_t impl_absolute_pos_depth_check(const motion_data_t* data, int call_depth)
{
    vector2d_t vec;
    vec.x = data->relative_x;
    vec.y = data->relative_y;

    if (!data->root)
    {
        return vec;
    }
    // hierarchized motion; first ensure the root is up-to-date then adjust our absolute position accordingly
    else
    {
        if (call_depth > MAX_MOTION_HIERARCHY_DEPTH)
        {
            trace_log(LOG_WARNING, "possible motion hierarchy cycle detected");
            return vec;
        }
        vector2d_t root_absolute = impl_absolute_pos_depth_check(data->root, call_depth+1);
        vec.x += root_absolute.x;
        vec.y += root_absolute.y;

        return vec;
    }
}

vector2d_t absolute_pos(const motion_data_t *data)
{
    return impl_absolute_pos_depth_check(data, 0);
}

void load_motion_from_json(motion_data_t *data, json_value_t *json, json_context_t *context)
{
    data->relative_x = json_float_or(0.0, "/x", json, context);
    data->relative_y = json_float_or(0.0, "/y", json, context);

    data->velocity = json_float_or(0.0, "/velocity", json, context);
    data->acceleration = json_float_or(0.0, "/acceleration", json, context);
    data->direction_angle = json_float_or(0.0, "/direction", json, context);
    data->angular_velocity = json_float_or(0.0, "/angular", json, context);
    data->rotation = json_float_or(0.0, "/rotation", json, context);
    data->rotational_speed = json_float_or(0.0, "/rotation-speed", json, context);
    data->rotational_acceleration = json_float_or(0.0, "/rotation-accel", json, context);

    data->dampening = json_float_or(0.0, "/dampening", json, context);

    data->max_speed = json_float_or(+INFINITY, "/max-speed", json, context);
    data->max_accel = json_float_or(+INFINITY, "/max-accel", json, context);
    data->max_rot = json_float_or(+INFINITY, "/max-rot", json, context);
    data->max_angular = json_float_or(+INFINITY, "/max-angular", json, context);

    const char* parent = json_string_or("", "/parent", json, context);
    entity_value_t* parent_motion = get_entity_value(parent, "motion");
    if (parent[0] != '\0' && parent_motion)
        data->root = &parent_motion->motion;
    else
        data->root = NULL;
}
