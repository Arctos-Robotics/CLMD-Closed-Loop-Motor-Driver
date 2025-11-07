#ifndef MOTION_PLANNER_H
#define MOTION_PLANNER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float start_position;
    float target_position;
    float current_position;
    float current_velocity;
    float max_velocity;
    float acceleration;
    int64_t start_time_us;
    bool is_moving;
} motion_profile_t;

void motion_init(void);
void motion_move_absolute(float target_deg, float max_rpm, float accel_deg_s2);
void motion_move_relative(float delta_deg, float max_rpm, float accel_deg_s2);
void motion_update(void);
float motion_get_target_position(void);
bool motion_is_moving(void);
void motion_stop(void);

#endif
