#include "can_protocol.h"
#include "config.h"
#include "safety.h"
#include "motor_control.h"
#include "motion_planner.h"
#include "encoder.h"
#include "gpio_init.h"
#include "esp_log.h"

static const char *TAG = "CAN";

esp_err_t can_init(void) {
    ESP_LOGI(TAG, "Initializing CAN bus");
    
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    
    g_config.tx_queue_len = 10;
    g_config.rx_queue_len = 10;
    
    esp_err_t ret = twai_driver_install(&g_config, &t_config, &f_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install TWAI driver: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = twai_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start TWAI: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "CAN bus initialized at 500 kbps");
    return ESP_OK;
}

uint8_t calculate_crc(const uint8_t *data, size_t len) {
    uint8_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum & 0xFF;
}

void can_handle_message(const twai_message_t *msg) {
    if (msg->data_length_code < 8) return;
    
    uint8_t can_id = msg->data[0];
    if (can_id != g_config.can_id) return;
    
    // Note: CRC verification skipped for now since CAN has its own CRC
    // MKS protocol uses 8 bytes of data, CAN frame already has error checking
    
    uint8_t mode = msg->data[1];
    uint16_t speed_rpm = (msg->data[2] << 8) | msg->data[3];
    uint16_t accel = msg->data[4];
    int32_t position = (msg->data[5] << 16) | (msg->data[6] << 8) | msg->data[7];
    
    // Handle signed 24-bit position
    if (position & 0x800000) {
        position |= 0xFF000000;
    }
    
    float position_deg = (float)position / 100.0f;
    
    ESP_LOGI(TAG, "CAN RX: mode=0x%02X, speed=%d RPM, pos=%.2f deg", 
             mode, speed_rpm, position_deg);
    
    safety_update_can_watchdog();
    
    switch (mode) {
        case CAN_MODE_ENABLE:
            motor_enable();
            g_status.is_enabled = true;
            break;
            
        case CAN_MODE_DISABLE:
            motor_disable();
            g_status.is_enabled = false;
            break;
            
        case CAN_MODE_MOVE_ABSOLUTE:
            if (motor_is_enabled()) {
                motion_move_absolute(position_deg, speed_rpm, accel);
                g_status.is_moving = true;
            }
            break;
            
        case CAN_MODE_MOVE_RELATIVE:
            if (motor_is_enabled()) {
                motion_move_relative(position_deg, speed_rpm, accel);
                g_status.is_moving = true;
            }
            break;
            
        case CAN_MODE_EMERGENCY_STOP:
            motor_emergency_stop();
            motion_stop();
            g_status.is_moving = false;
            break;
            
        case CAN_MODE_SET_ZERO:
            encoder_set_zero_offset();
            break;
            
        case CAN_MODE_QUERY_STATUS:
            can_send_status_response();
            break;
    }
}

void can_send_status_response(void) {
    twai_message_t tx_msg = {
        .identifier = 0x100 + g_config.can_id,
        .data_length_code = 8,  // CAN supports max 8 bytes
        .flags = TWAI_MSG_FLAG_NONE,
    };
    
    float current_pos = encoder_read_degrees();
    int32_t pos_encoded = (int32_t)(current_pos * 100.0f);
    
    uint8_t status_flags = 0;
    if (g_status.is_moving) status_flags |= 0x01;
    if (g_status.has_error) status_flags |= 0x02;
    if (g_status.is_homed) status_flags |= 0x04;
    if (g_status.is_enabled) status_flags |= 0x08;
    if (g_status.at_limit) status_flags |= 0x10;
    
    tx_msg.data[0] = 0x10 + g_config.can_id;
    tx_msg.data[1] = status_flags;
    tx_msg.data[2] = (pos_encoded >> 16) & 0xFF;
    tx_msg.data[3] = (pos_encoded >> 8) & 0xFF;
    tx_msg.data[4] = pos_encoded & 0xFF;
    tx_msg.data[5] = 0;  // Speed high byte
    tx_msg.data[6] = 0;  // Speed low byte
    tx_msg.data[7] = g_status.error_code;
    // Note: CRC omitted - CAN protocol has built-in error checking
    
    twai_transmit(&tx_msg, pdMS_TO_TICKS(10));
}

void can_task(void *pvParameters) {
    twai_message_t rx_msg;
    
    while (1) {
        if (twai_receive(&rx_msg, portMAX_DELAY) == ESP_OK) {
            can_handle_message(&rx_msg);
        }
    }
}
