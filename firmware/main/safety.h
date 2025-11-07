#ifndef SAFETY_H
#define SAFETY_H

#include <stdbool.h>

typedef enum {
    ERROR_NONE = 0x00,
    ERROR_POSITION_ERROR = 0x01,
    ERROR_ENCODER_COMM = 0x02,
    ERROR_TMC2209 = 0x03,
    ERROR_LIMIT_SWITCH = 0x04,
    ERROR_OVERCURRENT = 0x05,
    ERROR_WATCHDOG_TIMEOUT = 0x06,
    ERROR_UNKNOWN = 0xFF
} error_code_t;

typedef struct {
    bool is_moving;
    bool has_error;
    bool is_homed;
    bool is_enabled;
    bool at_limit;
    error_code_t error_code;
} system_status_t;

extern system_status_t g_status;

void safety_init(void);
void safety_check_position_error(float error);
void safety_check_limit_switches(void);
void safety_check_encoder(void);
void safety_update_can_watchdog(void);
void safety_check_watchdog(void);

#endif
