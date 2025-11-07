#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#define CONFIG_MAGIC 0xDEADBEEF
#define CONFIG_VERSION 1

typedef struct {
    uint32_t magic;
    uint32_t version;
    
    uint8_t can_id;
    char name[16];
    
    uint16_t motor_current_ma;
    uint8_t microsteps;
    float steps_per_degree;
    bool invert_direction;
    
    uint16_t max_rpm;
    uint16_t max_accel;
    float min_position_deg;
    float max_position_deg;
    
    uint8_t home_direction;
    uint16_t home_speed_rpm;
    float home_offset_deg;
    bool auto_home_on_boot;
    
    float pid_kp;
    float pid_ki;
    float pid_kd;
    float pid_output_limit;
    
    int32_t encoder_offset;
    bool encoder_invert;
    
    float position_error_limit;
    uint16_t watchdog_timeout_ms;
} motor_config_t;

extern motor_config_t g_config;

esp_err_t config_init(void);
esp_err_t config_save(void);
esp_err_t config_load(void);
void config_set_defaults(void);

#endif
