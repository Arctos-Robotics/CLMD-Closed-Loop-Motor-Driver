#include "safety.h"
#include "config.h"
#include "motor_control.h"
#include "encoder.h"
#include "gpio_init.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <math.h>

static const char *TAG = "SAFETY";
static int64_t last_can_message_time = 0;

system_status_t g_status = {0};

void safety_init(void) {
    g_status.is_moving = false;
    g_status.has_error = false;
    g_status.is_homed = false;
    g_status.is_enabled = false;
    g_status.at_limit = false;
    g_status.error_code = ERROR_NONE;
    last_can_message_time = esp_timer_get_time() / 1000;
    ESP_LOGI(TAG, "Safety system initialized");
}

void safety_check_position_error(float error) {
    if (fabsf(error) > g_config.position_error_limit) {
        motor_emergency_stop();
        g_status.has_error = true;
        g_status.error_code = ERROR_POSITION_ERROR;
        ESP_LOGE(TAG, "Position error too large: %.2f deg", error);
    }
}

void safety_check_limit_switches(void) {
    bool j_min = (gpio_get_level(J_MIN_PIN) == 0);
    bool j_max = (gpio_get_level(J_MAX_PIN) == 0);
    
    if (j_min || j_max) {
        motor_emergency_stop();
        g_status.has_error = true;
        g_status.at_limit = true;
        g_status.error_code = ERROR_LIMIT_SWITCH;
        ESP_LOGW(TAG, "Limit switch triggered: MIN=%d MAX=%d", j_min, j_max);
    } else {
        g_status.at_limit = false;
    }
}

void safety_check_encoder(void) {
    if (!encoder_is_communication_ok()) {
        motor_emergency_stop();
        g_status.has_error = true;
        g_status.error_code = ERROR_ENCODER_COMM;
        ESP_LOGE(TAG, "Encoder communication error");
    }
}

void safety_update_can_watchdog(void) {
    last_can_message_time = esp_timer_get_time() / 1000;
}

void safety_check_watchdog(void) {
    int64_t now = esp_timer_get_time() / 1000;
    int64_t elapsed = now - last_can_message_time;
    
    if (elapsed > g_config.watchdog_timeout_ms && g_status.is_moving) {
        motor_emergency_stop();
        g_status.has_error = true;
        g_status.error_code = ERROR_WATCHDOG_TIMEOUT;
        ESP_LOGW(TAG, "CAN watchdog timeout");
    }
}
