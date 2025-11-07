#include "pid_controller.h"
#include "esp_timer.h"
#include <math.h>

void pid_init(pid_controller_t *pid, float kp, float ki, float kd, float output_limit) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->output_limit = output_limit;
    pid->last_time_us = esp_timer_get_time();
}

float pid_compute(pid_controller_t *pid, float error) {
    int64_t now = esp_timer_get_time();
    float dt = (now - pid->last_time_us) / 1000000.0f;
    pid->last_time_us = now;
    
    if (dt <= 0.0f || dt > 1.0f) dt = 0.001f;  // Sanity check
    
    // Proportional
    float p_term = pid->kp * error;
    
    // Integral with anti-windup
    pid->integral += pid->ki * error * dt;
    if (pid->integral > pid->output_limit) pid->integral = pid->output_limit;
    if (pid->integral < -pid->output_limit) pid->integral = -pid->output_limit;
    
    // Derivative
    float d_term = pid->kd * (error - pid->prev_error) / dt;
    pid->prev_error = error;
    
    // Output
    float output = p_term + pid->integral + d_term;
    
    // Limit output
    if (output > pid->output_limit) output = pid->output_limit;
    if (output < -pid->output_limit) output = -pid->output_limit;
    
    return output;
}

void pid_reset(pid_controller_t *pid) {
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->last_time_us = esp_timer_get_time();
}
