#ifndef TMC2209_ADVANCED_H
#define TMC2209_ADVANCED_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

// TMC2209 Register Addresses
#define TMC_GCONF       0x00
#define TMC_GSTAT       0x01
#define TMC_IOIN        0x06
#define TMC_IHOLD_IRUN  0x10
#define TMC_TPOWERDOWN  0x11
#define TMC_TSTEP       0x12
#define TMC_TPWMTHRS    0x13
#define TMC_TCOOLTHRS   0x14
#define TMC_SGTHRS      0x40
#define TMC_SG_RESULT   0x41
#define TMC_COOLCONF    0x42
#define TMC_DRV_STATUS  0x6F
#define TMC_CHOPCONF    0x6C

// GCONF Register Bits
#define TMC_GCONF_I_SCALE_ANALOG    (1 << 0)
#define TMC_GCONF_INTERNAL_RSENSE   (1 << 1)
#define TMC_GCONF_EN_SPREADCYCLE    (1 << 2)
#define TMC_GCONF_SHAFT             (1 << 3)
#define TMC_GCONF_INDEX_OTPW        (1 << 4)
#define TMC_GCONF_INDEX_STEP        (1 << 5)
#define TMC_GCONF_PDN_DISABLE       (1 << 6)
#define TMC_GCONF_MSTEP_REG_SELECT  (1 << 7)
#define TMC_GCONF_MULTISTEP_FILT    (1 << 8)

// DRV_STATUS Register Bits
#define TMC_DRV_OTPW        (1 << 0)   // Overtemperature pre-warning
#define TMC_DRV_OT          (1 << 1)   // Overtemperature shutdown
#define TMC_DRV_S2GA        (1 << 2)   // Short to ground A
#define TMC_DRV_S2GB        (1 << 3)   // Short to ground B
#define TMC_DRV_S2VSA       (1 << 4)   // Short to VS A
#define TMC_DRV_S2VSB       (1 << 5)   // Short to VS B
#define TMC_DRV_OLA         (1 << 6)   // Open load A
#define TMC_DRV_OLB         (1 << 7)   // Open load B
#define TMC_DRV_STST        (1 << 31)  // Standstill detected

// Function prototypes
esp_err_t tmc2209_init_uart(void);
esp_err_t tmc2209_write_register(uint8_t reg, uint32_t value);
esp_err_t tmc2209_read_register(uint8_t reg, uint32_t *value);

// Advanced configuration functions
esp_err_t tmc2209_enable_coolstep(uint8_t semin, uint8_t semax, uint8_t sedn);
esp_err_t tmc2209_enable_stallguard(uint8_t threshold);
esp_err_t tmc2209_set_microstepping(uint8_t mres);
esp_err_t tmc2209_set_current(uint16_t run_ma, uint16_t hold_ma);
esp_err_t tmc2209_enable_stealthchop(uint16_t threshold);

// Diagnostic functions
esp_err_t tmc2209_get_temperature(uint8_t *temp);
esp_err_t tmc2209_get_stallguard_result(int16_t *result);
esp_err_t tmc2209_get_driver_status(uint32_t *status);
bool tmc2209_is_overtemperature(void);
bool tmc2209_is_short_circuit(void);
bool tmc2209_is_open_load(void);

#endif // TMC2209_ADVANCED_H
