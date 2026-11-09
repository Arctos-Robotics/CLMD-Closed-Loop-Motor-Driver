#pragma once
#include "esp_err.h"
#include "mt6701_codec.h"

esp_err_t mt6701_begin(void);
esp_err_t mt6701_begin_test(unsigned hz, unsigned address, unsigned method);
typedef struct {
    uint8_t address, reg03, reg04;
    uint16_t raw;
    bool transfer_ok, valid;
    esp_err_t error_code;
    unsigned hz, method;
    uint8_t failed_register;
    bool sda_before, scl_before, sda_after, scl_after;
} mt6701_i2c_sample_t;

esp_err_t mt6701_read(mt6701_i2c_sample_t *sample);
void mt6701_end(void);
