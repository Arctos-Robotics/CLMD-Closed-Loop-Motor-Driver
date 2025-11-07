#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#define TLE5012_READ_ANGLE_VALUE    0x8021
#define TLE5012_READ_STATUS         0x8001

esp_err_t encoder_init(void);
uint16_t encoder_read_raw(void);
float encoder_read_degrees(void);
void encoder_set_zero_offset(void);
bool encoder_is_communication_ok(void);

#endif
