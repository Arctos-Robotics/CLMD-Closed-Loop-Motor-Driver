#pragma once
#include <stdint.h>
// R22/R23 = 0.12 ohm, internal comparator resistance 0.02 ohm, VSENSE=0.
// Nominal full scale = 0.325 / (0.12 + 0.02) / sqrt(2) = 1.641497 A RMS.
#define CLMD_MAX_CURRENT_MA 1641
uint8_t current_scale(uint16_t requested_ma);
double current_rms_ma(uint16_t requested_ma);
