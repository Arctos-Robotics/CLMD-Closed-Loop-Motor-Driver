#pragma once
#include <stdint.h>
#include "esp_err.h"
#define TMC_GCONF 0x00
#define TMC_IHOLD_IRUN 0x10
#define TMC_DRV_STATUS 0x6F
#define TMC_CHOPCONF 0x6C
esp_err_t tmc2209_init_uart(void);
esp_err_t tmc2209_write_register(uint8_t reg,uint32_t value);
esp_err_t tmc2209_read_register(uint8_t reg,uint32_t *value);
esp_err_t tmc2209_set_microstepping(uint8_t mres);
esp_err_t tmc2209_set_current(uint16_t run_ma,uint16_t hold_ma);
esp_err_t tmc2209_set_velocity(int32_t velocity);
