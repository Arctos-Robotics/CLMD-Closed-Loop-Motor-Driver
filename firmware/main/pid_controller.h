#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <stdint.h>

typedef struct {
    float kp, ki, kd;
    float integral;
    float prev_error;
    float output_limit;
    int64_t last_time_us;
} pid_controller_t;

void pid_init(pid_controller_t *pid, float kp, float ki, float kd, float output_limit);
float pid_compute(pid_controller_t *pid, float error);
void pid_reset(pid_controller_t *pid);

#endif
