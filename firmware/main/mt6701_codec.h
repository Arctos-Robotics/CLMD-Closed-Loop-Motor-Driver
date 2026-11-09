#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t frame;
    uint16_t raw;
    uint8_t status, crc_received, crc_expected;
    bool crc_ok, idle_high, valid;
} mt6701_sample_t;

uint16_t mt6701_i2c_angle(uint8_t reg03, uint8_t reg04);
uint8_t mt6701_crc6(uint32_t data18);
mt6701_sample_t mt6701_decode(uint32_t frame, bool idle_high);
bool mt6701_test_allows(bool active, bool native, uint8_t opcode, uint8_t parameter);
