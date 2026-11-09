#pragma once
#include "core.h"
#include "esp_err.h"
void hw_pins_init(void);
esp_err_t hw_driver_configure(const settings_t *c);
esp_err_t hw_driver_health(uint32_t *status);
esp_err_t hw_velocity(double rpm, const settings_t *c);
bool hw_encoder(uint16_t *raw, uint16_t *word, uint16_t *safety);
bool hw_inputs(int *minimum, int *maximum);
void hw_led(bool enabled, bool moving, bool fault);
