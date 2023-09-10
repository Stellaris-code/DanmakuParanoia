#include "motion.h"

#include "simd/simd.h"

static inline simd_vf vec256_sin_garrett_c11_(simd_vf x)
{
    simd_vf x2 = x * x;
    return (((((-2.05342856289746600727e-08f*x2 + 2.70405218307799040084e-06f)*x2
               - 1.98125763417806681909e-04f)*x2 + 8.33255814755188010464e-03f)*x2
             - 1.66665772196961623983e-01f)*x2 + 9.99999707044156546685e-01f)*x;
}

static inline void vec_fast_sincos(simd_vf angle, simd_vf *sin, simd_vf *cos)
{
    angle -= 3.1415926535f;
    simd_vf cosmultiplier=-vf_copysign(vf_abs(angle)-3.1415926535f/2.0f, vf_set1(1.0f));

    *sin=-vec256_sin_garrett_c11_(angle);
    *cos=-cosmultiplier*vf_sqrt(1.f-*sin**sin);
}

static inline void update_motion_simd(float dt, packed_motion_t* motion)
{
#if 0
    for (int i = 0; i < SIMD_FLT_PER_REG; ++i)
    {
        motion->velocity[i]         = MIN(motion->max_speed[i], (motion->velocity[i] + motion->acceleration[i]*dt)*(1.0f-motion->dampening[i]));

        motion->direction_angle[i] += motion->angular_velocity[i]*dt;
        motion->rotation[i] += motion->rotational_speed[i]*dt;
        motion->direction_angle[i] -= floorf(motion->direction_angle[i]/(2*M_PI))*(2*M_PI);
        motion->rotation[i] -= floorf(motion->rotation[i]/(2*M_PI))*(2*M_PI);

        float movement_x, movement_y;
        fast_sincos_(motion->direction_angle[i], &movement_y, &movement_x);
        //sincosf(motion->direction_angle[i], &movement_y, &movement_x);
        movement_x *= motion->velocity[i];
        movement_y *= motion->velocity[i];

        motion->relative_x[i] += movement_x*dt;
        motion->relative_y[i] += -movement_y*dt; // coordinate system correction
    }
    return;
#else

    simd_vf vec_dt = vf_set1(dt);

    simd_vf ang_vel = vf_load(motion->angular_velocity);
    simd_vf rot_speed = vf_load(motion->rotational_speed);
    simd_vf accel = vf_load(motion->acceleration);


    simd_vf velocity = vf_load(motion->velocity);
    velocity = vf_min(vf_load(motion->max_speed),
                      (velocity + vec_dt * accel) * (vf_set1(1.0f) - vf_load(motion->dampening)));
    vf_store(motion->velocity, velocity);

    const simd_vf vec_2pi = vf_set1(2.0f*M_PI);
    const simd_vf vec_rec_2pi = vf_set1(1.0f/(2.0f*M_PI));
    simd_vf dir_angle = vf_load(motion->direction_angle);
    dir_angle += vec_dt * ang_vel;
    simd_vf rotation = vf_load(motion->rotation);
    rotation += vec_dt * rot_speed;
    dir_angle = dir_angle - vec_2pi * vf_floor(dir_angle * vec_rec_2pi);
    rotation = rotation - vec_2pi * vf_floor(rotation * vec_rec_2pi);
    vf_store(motion->rotation, rotation);

    simd_vf mov_x, mov_y;
    vec_fast_sincos(dir_angle, &mov_y, &mov_x);
    vf_store(motion->direction_angle, dir_angle);

    mov_x *= velocity;
    mov_y *= velocity;

    vf_store(motion->relative_x, vf_load(motion->relative_x) + mov_x * vec_dt);
    vf_store(motion->relative_y, vf_load(motion->relative_y) + mov_y * vec_dt);
#endif
}
