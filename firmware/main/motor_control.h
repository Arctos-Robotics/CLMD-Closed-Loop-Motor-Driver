#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <stdbool.h>
#include "esp_err.h"

esp_err_t motor_init(void);
void motor_enable(void);
void motor_disable(void);
void motor_set_direction(bool forward);
void motor_set_step_rate(float steps_per_sec);
void motor_emergency_stop(void);
bool motor_is_enabled(void);

#endif
