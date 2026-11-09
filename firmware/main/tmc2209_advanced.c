#include "tmc2209_advanced.h"
#include "current.h"
#include "gpio_init.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>
#include <math.h>
#include "freertos/semphr.h"

static const char *TAG = "TMC2209";

// TMC2209 UART Configuration
#define TMC_UART_NUM        UART_NUM_1  // Use UART1 instead of UART0
#define TMC_UART_BAUD       115200
#define TMC_UART_BUF_SIZE   256
#define TMC_NODE_ADDR       0x00
static SemaphoreHandle_t uart_mutex;

esp_err_t tmc2209_init_uart(void) {
    uart_mutex = xSemaphoreCreateMutex();
    if (!uart_mutex) return ESP_ERR_NO_MEM;
    uart_config_t uart_config = {
        .baud_rate = TMC_UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    
    ESP_ERROR_CHECK(uart_param_config(TMC_UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(TMC_UART_NUM, TMC_UART_TX_PIN, 
                                  TMC_UART_RX_PIN, -1, -1));
    ESP_ERROR_CHECK(uart_driver_install(TMC_UART_NUM, TMC_UART_BUF_SIZE, 
                                        TMC_UART_BUF_SIZE, 0, NULL, 0));
    
    ESP_LOGI(TAG, "TMC2209 UART initialized on UART1");
    return ESP_OK;
}

// CRC calculation for TMC2209 UART
static uint8_t tmc2209_calc_crc(uint8_t *data, uint8_t len) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        uint8_t byte = data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if ((crc >> 7) ^ (byte & 0x01)) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc = crc << 1;
            }
            byte >>= 1;
        }
    }
    return crc;
}

esp_err_t tmc2209_write_register(uint8_t reg, uint32_t value) {
    if (!uart_mutex || xSemaphoreTake(uart_mutex, pdMS_TO_TICKS(200)) != pdTRUE)
        return ESP_ERR_TIMEOUT;
    uint8_t data[8];
    
    // Sync + Node address
    data[0] = 0x05;
    data[1] = TMC_NODE_ADDR;
    
    // Register address (write)
    data[2] = reg | 0x80;
    
    // Value (32-bit, MSB first)
    data[3] = (value >> 24) & 0xFF;
    data[4] = (value >> 16) & 0xFF;
    data[5] = (value >> 8) & 0xFF;
    data[6] = value & 0xFF;
    
    // CRC
    data[7] = tmc2209_calc_crc(data, 7);
    
    int sent = uart_write_bytes(TMC_UART_NUM, data, 8);
    esp_err_t result = uart_wait_tx_done(TMC_UART_NUM, pdMS_TO_TICKS(100));
    uart_flush_input(TMC_UART_NUM);  // Writes echo on the single-wire bus; no reply is expected.
    xSemaphoreGive(uart_mutex);
    return sent == 8 ? result : ESP_FAIL;
}


static esp_err_t read_register_unlocked(uint8_t reg, uint32_t *value) {
    uint8_t data[4];
    
    // Sync + Node address
    data[0] = 0x05;
    data[1] = TMC_NODE_ADDR;
    
    // Register address (read)
    data[2] = reg;
    
    // CRC
    data[3] = tmc2209_calc_crc(data, 3);
    
    // Clear RX buffer
    uart_flush_input(TMC_UART_NUM);
    
    // Send read request
    uart_write_bytes(TMC_UART_NUM, data, 4);
    uart_wait_tx_done(TMC_UART_NUM, pdMS_TO_TICKS(100));
    
    // Wait for response (12 bytes: 4 byte request echo + 8 byte response)
    vTaskDelay(pdMS_TO_TICKS(2));
    
    uint8_t response[12];
    int len = uart_read_bytes(TMC_UART_NUM, response, 12, pdMS_TO_TICKS(100));
    
    if (len >= 8) {
        // Response is in last 8 bytes
        uint8_t *resp = &response[len - 8];
        
        // Verify CRC
        uint8_t crc = tmc2209_calc_crc(resp, 7);
        if (crc != resp[7] || resp[0] != 0x05 || resp[1] != 0xFF || resp[2] != reg) {
            ESP_LOGW(TAG, "CRC mismatch in read response");
            *value = 0;
            return ESP_ERR_INVALID_CRC;
        }
        
        // Extract value (bytes 3-6, MSB first)
        *value = ((uint32_t)resp[3] << 24) | ((uint32_t)resp[4] << 16) | 
                 ((uint32_t)resp[5] << 8) | resp[6];
        
        ESP_LOGI(TAG, "Read reg 0x%02X = 0x%08X", reg, *value);
        return ESP_OK;
    }
    
    ESP_LOGW(TAG, "No response from TMC2209 (got %d bytes)", len);
    *value = 0;
    return ESP_ERR_TIMEOUT;
}

esp_err_t tmc2209_read_register(uint8_t reg, uint32_t *value) {
    if (!uart_mutex || xSemaphoreTake(uart_mutex, pdMS_TO_TICKS(200)) != pdTRUE)
        return ESP_ERR_TIMEOUT;
    esp_err_t result = read_register_unlocked(reg, value);
    xSemaphoreGive(uart_mutex);
    return result;
}

esp_err_t tmc2209_set_velocity(int32_t velocity) {
    if (velocity < -8388607 || velocity > 8388607) return ESP_ERR_INVALID_ARG;
    return tmc2209_write_register(0x22, ((uint32_t)velocity) & 0xFFFFFF);
}

esp_err_t tmc2209_set_microstepping(uint8_t mres) {
    // mres: 0=256, 1=128, 2=64, 3=32, 4=16, 5=8, 6=4, 7=2, 8=1
    
    // Read current CHOPCONF
    uint32_t chopconf = 0x10000053;  // Default value
    
    // Clear mres bits and set new value
    chopconf &= ~(0x0F << 24);
    chopconf |= (mres << 24);
    
    esp_err_t ret = tmc2209_write_register(TMC_CHOPCONF, chopconf);
    if (ret == ESP_OK) {
        uint16_t steps = 256 >> mres;
        ESP_LOGI(TAG, "Microstepping set to 1/%d", steps);
    }
    
    return ret;
}

esp_err_t tmc2209_set_current(uint16_t run_ma, uint16_t hold_ma) {
    if (run_ma < 50 || run_ma > CLMD_MAX_CURRENT_MA || hold_ma > run_ma) return ESP_ERR_INVALID_ARG;
    // Datasheet RMS formula, external 0.12-ohm sense resistors, vsense=0.
    // Round down to the supported current scale; CS=0 still supplies ~51 mA.
    uint8_t cs_run = current_scale(run_ma), cs_hold = current_scale(hold_ma);
    
    // IHOLD_IRUN register format:
    // Bits 0-4: IHOLD (standstill current)
    // Bits 8-12: IRUN (run current)
    // Bits 16-19: IHOLDDELAY (delay before reducing to hold current)
    uint32_t ihold_irun = (cs_hold << 0) | (cs_run << 8) | (5 << 16);
    
    esp_err_t ret = tmc2209_write_register(TMC_IHOLD_IRUN, ihold_irun);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Current set: run=%dmA (CS=%d), hold=%dmA (CS=%d)", 
                 run_ma, cs_run, hold_ma, cs_hold);
        ESP_LOGI(TAG, "Estimated RMS: run=%.0fmA hold=%.0fmA", current_rms_ma(run_ma), current_rms_ma(hold_ma));
    } else {
        ESP_LOGE(TAG, "Failed to write IHOLD_IRUN");
    }
    
    return ret;
}

