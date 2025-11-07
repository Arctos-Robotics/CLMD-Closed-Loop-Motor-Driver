#include "usb_config.h"
#include "config.h"
#include "motor_control.h"
#include "motion_planner.h"
#include "encoder.h"
#include "safety.h"
#include "gpio_init.h"
#include "tmc2209_advanced.h"
#include "pid_controller.h"
#include "esp_log.h"
#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#include "tusb_console.h"
#include <string.h>
#include <stdio.h>

// Forward declaration
void usb_handle_advanced_commands(char *cmd);

static const char *TAG = "USB";
static char rx_buffer[256];
static size_t rx_len = 0;

esp_err_t usb_config_init(void) {
    ESP_LOGI(TAG, "Initializing USB CDC");
    
    tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false,
        .configuration_descriptor = NULL,
    };
    
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    
    tinyusb_config_cdcacm_t acm_cfg = {
        .usb_dev = TINYUSB_USBDEV_0,
        .cdc_port = TINYUSB_CDC_ACM_0,
        .rx_unread_buf_sz = 256,
        .callback_rx = NULL,
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = NULL,
        .callback_line_coding_changed = NULL
    };
    
    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));
    
    ESP_LOGI(TAG, "USB CDC initialized");
    return ESP_OK;
}

void usb_send_response(const char *response) {
    tinyusb_cdcacm_write_queue(TINYUSB_CDC_ACM_0, (uint8_t*)response, strlen(response));
    tinyusb_cdcacm_write_flush(TINYUSB_CDC_ACM_0, 0);
}

void usb_handle_command(char *cmd) {
    char response[128];
    
    if (strncmp(cmd, "GET_POS", 7) == 0) {
        float pos = encoder_read_degrees();
        snprintf(response, sizeof(response), "POS %.2f\r\n", pos);
        usb_send_response(response);
    }
    else if (strncmp(cmd, "ENABLE", 6) == 0) {
        motor_enable();
        g_status.is_enabled = true;
        usb_send_response("OK Motor enabled\r\n");
    }
    else if (strncmp(cmd, "DISABLE", 7) == 0) {
        motor_disable();
        g_status.is_enabled = false;
        usb_send_response("OK Motor disabled\r\n");
    }
    else if (strncmp(cmd, "MOVE_ABS", 8) == 0) {
        float deg;
        int rpm;
        if (sscanf(cmd, "MOVE_ABS %f %d", &deg, &rpm) == 2) {
            motion_move_absolute(deg, rpm, g_config.max_accel);
            g_status.is_moving = true;
            snprintf(response, sizeof(response), "OK Moving to %.2f deg\r\n", deg);
            usb_send_response(response);
        } else {
            usb_send_response("ERROR Invalid parameters\r\n");
        }
    }
    else if (strncmp(cmd, "MOVE_REL", 8) == 0) {
        float deg;
        int rpm;
        if (sscanf(cmd, "MOVE_REL %f %d", &deg, &rpm) == 2) {
            motion_move_relative(deg, rpm, g_config.max_accel);
            g_status.is_moving = true;
            snprintf(response, sizeof(response), "OK Moving %.2f deg\r\n", deg);
            usb_send_response(response);
        } else {
            usb_send_response("ERROR Invalid parameters\r\n");
        }
    }
    else if (strncmp(cmd, "STOP", 4) == 0) {
        motor_emergency_stop();
        motion_stop();
        g_status.is_moving = false;
        usb_send_response("OK Stopped\r\n");
    }
    else if (strncmp(cmd, "SET_ZERO", 8) == 0) {
        encoder_set_zero_offset();
        usb_send_response("OK Zero position set\r\n");
    }
    else if (strncmp(cmd, "SET_ID", 6) == 0) {
        int id;
        if (sscanf(cmd, "SET_ID %d", &id) == 1 && id >= 1 && id <= 6) {
            g_config.can_id = id;
            usb_send_response("OK CAN ID set\r\n");
        } else {
            usb_send_response("ERROR Invalid ID (1-6)\r\n");
        }
    }
    else if (strncmp(cmd, "SAVE", 4) == 0) {
        if (config_save() == ESP_OK) {
            usb_send_response("OK Configuration saved\r\n");
        } else {
            usb_send_response("ERROR Failed to save\r\n");
        }
    }
    else if (strncmp(cmd, "VERSION", 7) == 0) {
        usb_send_response("OK Closed Loop Driver v1.0\r\n");
    }
    else if (strncmp(cmd, "GET_STATUS", 10) == 0) {
        uint8_t flags = 0;
        if (g_status.is_moving) flags |= 0x01;
        if (g_status.has_error) flags |= 0x02;
        if (g_status.is_homed) flags |= 0x04;
        if (g_status.is_enabled) flags |= 0x08;
        snprintf(response, sizeof(response), "STATUS 0x%02X\r\n", flags);
        usb_send_response(response);
    }
    else {
        // Try advanced commands
        usb_handle_advanced_commands(cmd);
    }
}

void usb_config_task(void *pvParameters) {
    while (1) {
        size_t available;
        esp_err_t ret = tinyusb_cdcacm_read(TINYUSB_CDC_ACM_0, 
                                            (uint8_t*)&rx_buffer[rx_len], 
                                            sizeof(rx_buffer) - rx_len - 1, 
                                            &available);
        
        if (ret == ESP_OK && available > 0) {
            rx_len += available;
            rx_buffer[rx_len] = '\0';
            
            // Look for newline
            char *newline = strchr(rx_buffer, '\n');
            if (newline) {
                *newline = '\0';
                
                // Remove carriage return if present
                char *cr = strchr(rx_buffer, '\r');
                if (cr) *cr = '\0';
                
                // Process command
                if (strlen(rx_buffer) > 0) {
                    usb_handle_command(rx_buffer);
                }
                
                // Clear buffer
                rx_len = 0;
                rx_buffer[0] = '\0';
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Additional USB commands for advanced features
void usb_handle_advanced_commands(char *cmd) {
    char response[128];
    
    if (strncmp(cmd, "SET_PID", 7) == 0) {
        float kp, ki, kd;
        if (sscanf(cmd, "SET_PID %f %f %f", &kp, &ki, &kd) == 3) {
            g_config.pid_kp = kp;
            g_config.pid_ki = ki;
            g_config.pid_kd = kd;
            // Note: PID controller will use new values on next control loop
            // Direct reinitialization not safe from USB task
            snprintf(response, sizeof(response), "OK PID set: Kp=%.2f Ki=%.3f Kd=%.3f\r\n", kp, ki, kd);
            usb_send_response(response);
            ESP_LOGI("USB", "PID parameters updated in config");
        } else {
            usb_send_response("ERROR Invalid PID parameters\r\n");
        }
    }
    else if (strncmp(cmd, "LED", 3) == 0) {
        int r, g, b;
        if (sscanf(cmd, "LED %d %d %d", &r, &g, &b) == 3) {
            gpio_set_level(LED_RED_PIN, r > 0 ? 1 : 0);
            gpio_set_level(LED_GREEN_PIN, g > 0 ? 1 : 0);
            gpio_set_level(LED_BLUE_PIN, b > 0 ? 1 : 0);
            usb_send_response("OK LED set\r\n");
        } else {
            usb_send_response("ERROR Invalid LED parameters\r\n");
        }
    }
    else if (strncmp(cmd, "GET_ENDSTOPS", 12) == 0) {
        int jmin = gpio_get_level(J_MIN_PIN) == 0 ? 1 : 0;
        int jmax = gpio_get_level(J_MAX_PIN) == 0 ? 1 : 0;
        snprintf(response, sizeof(response), "ENDSTOPS %d %d\r\n", jmin, jmax);
        usb_send_response(response);
    }
    else if (strncmp(cmd, "GET_ENCODER", 11) == 0) {
        uint16_t raw = encoder_read_raw();
        snprintf(response, sizeof(response), "ENCODER %u\r\n", raw);
        usb_send_response(response);
    }
    // TMC2209 Advanced Commands
    else if (strncmp(cmd, "ENABLE_COOLSTEP", 15) == 0) {
        tmc2209_enable_coolstep(5, 10, 1);
        usb_send_response("OK CoolStep enabled - expect 50-75% power savings\r\n");
    }
    else if (strncmp(cmd, "DISABLE_COOLSTEP", 16) == 0) {
        tmc2209_enable_coolstep(0, 0, 0);
        usb_send_response("OK CoolStep disabled\r\n");
    }
    else if (strncmp(cmd, "GET_TMC_STATUS", 14) == 0) {
        uint32_t status;
        if (tmc2209_get_driver_status(&status) == ESP_OK) {
            snprintf(response, sizeof(response), "TMC_STATUS 0x%08lX\r\n", status);
            usb_send_response(response);
        } else {
            usb_send_response("ERROR Failed to read TMC status\r\n");
        }
    }
    else if (strncmp(cmd, "GET_TMC_TEMP", 12) == 0) {
        uint8_t temp;
        if (tmc2209_get_temperature(&temp) == ESP_OK) {
            snprintf(response, sizeof(response), "TMC_TEMP %d\r\n", temp);
            usb_send_response(response);
        } else {
            usb_send_response("ERROR Failed to read temperature\r\n");
        }
    }
    else if (strncmp(cmd, "GET_STALLGUARD", 14) == 0) {
        int16_t sg_result;
        if (tmc2209_get_stallguard_result(&sg_result) == ESP_OK) {
            snprintf(response, sizeof(response), "STALLGUARD %d\r\n", sg_result);
            usb_send_response(response);
        } else {
            usb_send_response("ERROR Failed to read StallGuard\r\n");
        }
    }
    else if (strncmp(cmd, "SET_TMC_CURRENT", 15) == 0) {
        int run_ma, hold_ma;
        if (sscanf(cmd, "SET_TMC_CURRENT %d %d", &run_ma, &hold_ma) == 2) {
            tmc2209_set_current(run_ma, hold_ma);
            snprintf(response, sizeof(response), "OK Current set: %dmA run, %dmA hold\r\n", run_ma, hold_ma);
            usb_send_response(response);
        } else {
            usb_send_response("ERROR Invalid current parameters\r\n");
        }
    }
    else {
        usb_send_response("ERROR Unknown command\r\n");
    }
}
