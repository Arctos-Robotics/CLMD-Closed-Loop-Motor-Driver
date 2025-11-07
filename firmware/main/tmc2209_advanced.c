#include "tmc2209_advanced.h"
#include "gpio_init.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "TMC2209";

#define TMC_UART_NUM        UART_NUM_0
#define TMC_UART_BAUD       115200
#define TMC_UART_BUF_SIZE   256
#define TMC_NODE_ADDR       0x00

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

esp_err_t tmc2209_init_uart(void) {
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
    
    ESP_LOGI(TAG, "TMC2209 UART initialized");
    return ESP_OK;
}

esp_err_t tmc2209_write_register(uint8_t reg, uint32_t value) {
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
    
    uart_write_bytes(TMC_UART_NUM, data, 8);
    uart_wait_tx_done(TMC_UART_NUM, pdMS_TO_TICKS(100));
    
    return ESP_OK;
}


esp_err_t tmc2209_read_register(uint8_t reg, uint32_t *value) {
    uint8_t tx_data[4];
    uint8_t rx_data[12];
    
    // Send read request
    tx_data[0] = 0x05;
    tx_data[1] = TMC_NODE_ADDR;
    tx_data[2] = reg;
    tx_data[3] = tmc2209_calc_crc(tx_data, 3);
    
    uart_flush(TMC_UART_NUM);
    uart_write_bytes(TMC_UART_NUM, tx_data, 4);
    
    // Wait for response
    int len = uart_read_bytes(TMC_UART_NUM, rx_data, 12, pdMS_TO_TICKS(100));
    
    if (len != 12) {
        ESP_LOGW(TAG, "Read timeout or wrong length: %d", len);
        return ESP_ERR_TIMEOUT;
    }
    
    // Verify CRC
    uint8_t crc = tmc2209_calc_crc(rx_data, 11);
    if (crc != rx_data[11]) {
        ESP_LOGW(TAG, "CRC mismatch");
        return ESP_ERR_INVALID_CRC;
    }
    
    // Extract value
    *value = ((uint32_t)rx_data[7] << 24) |
             ((uint32_t)rx_data[8] << 16) |
             ((uint32_t)rx_data[9] << 8) |
             rx_data[10];
    
    return ESP_OK;
}

esp_err_t tmc2209_enable_coolstep(uint8_t semin, uint8_t semax, uint8_t sedn) {
    // CoolStep configuration
    // semin: minimum StallGuard value (0-15)
    // semax: StallGuard hysteresis (0-15)
    // sedn: current decrement speed (0-3)
    
    uint32_t coolconf = 
        (semin << 0) |      // semin
        (0 << 5) |          // seup (current increment)
        (semax << 8) |      // semax
        (sedn << 13) |      // sedn
        (0 << 15);          // seimin (minimum current = 1/2)
    
    esp_err_t ret = tmc2209_write_register(TMC_COOLCONF, coolconf);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "CoolStep enabled: semin=%d, semax=%d, sedn=%d", 
                 semin, semax, sedn);
    }
    
    return ret;
}

esp_err_t tmc2209_enable_stallguard(uint8_t threshold) {
    // StallGuard threshold (0-255)
    // Lower = more sensitive
    // Typical: 10-30 for most applications
    
    esp_err_t ret = tmc2209_write_register(TMC_SGTHRS, threshold);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "StallGuard enabled: threshold=%d", threshold);
    }
    
    return ret;
}

esp_err_t tmc2209_set_microstepping(uint8_t mres) {
    // mres: 0=256, 1=128, 2=64, 3=32, 4=16, 5=8, 6=4, 7=2, 8=1
    
    uint32_t chopconf;
    esp_err_t ret = tmc2209_read_register(TMC_CHOPCONF, &chopconf);
    if (ret != ESP_OK) return ret;
    
    // Clear mres bits and set new value
    chopconf &= ~(0x0F << 24);
    chopconf |= (mres << 24);
    
    ret = tmc2209_write_register(TMC_CHOPCONF, chopconf);
    if (ret == ESP_OK) {
        uint16_t steps = 256 >> mres;
        ESP_LOGI(TAG, "Microstepping set to 1/%d", steps);
    }
    
    return ret;
}

esp_err_t tmc2209_set_current(uint16_t run_ma, uint16_t hold_ma) {
    // Convert mA to register values
    // Formula: I_RMS = (CS + 1) / 32 * 1.77A (with 120mΩ sense resistors)
    // CS = (I_RMS * 32 / 1.77) - 1
    
    uint8_t irun = (uint8_t)((run_ma * 32.0f / 1770.0f) - 1);
    uint8_t ihold = (uint8_t)((hold_ma * 32.0f / 1770.0f) - 1);
    
    if (irun > 31) irun = 31;
    if (ihold > 31) ihold = 31;
    
    uint32_t ihold_irun = (ihold << 0) | (irun << 8) | (5 << 16);  // IHOLDDELAY=5
    
    esp_err_t ret = tmc2209_write_register(TMC_IHOLD_IRUN, ihold_irun);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Current set: run=%dmA, hold=%dmA", run_ma, hold_ma);
    }
    
    return ret;
}

esp_err_t tmc2209_enable_stealthchop(uint16_t threshold) {
    // Enable StealthChop below this velocity threshold
    // threshold in steps/s (0 = always StealthChop)
    
    esp_err_t ret = tmc2209_write_register(TMC_TPWMTHRS, threshold);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "StealthChop enabled: threshold=%d steps/s", threshold);
    }
    
    return ret;
}

esp_err_t tmc2209_get_temperature(uint8_t *temp) {
    uint32_t drv_status;
    esp_err_t ret = tmc2209_read_register(TMC_DRV_STATUS, &drv_status);
    
    if (ret == ESP_OK) {
        // Temperature is not directly available, but we can detect overtemp
        bool otpw = (drv_status & TMC_DRV_OTPW) != 0;
        bool ot = (drv_status & TMC_DRV_OT) != 0;
        
        if (ot) {
            *temp = 150;  // Shutdown temperature
        } else if (otpw) {
            *temp = 120;  // Pre-warning temperature
        } else {
            *temp = 50;   // Estimated normal temp
        }
    }
    
    return ret;
}

esp_err_t tmc2209_get_stallguard_result(int16_t *result) {
    uint32_t sg_result;
    esp_err_t ret = tmc2209_read_register(TMC_SG_RESULT, &sg_result);
    
    if (ret == ESP_OK) {
        *result = (int16_t)(sg_result & 0x3FF);  // 10-bit value
    }
    
    return ret;
}

esp_err_t tmc2209_get_driver_status(uint32_t *status) {
    return tmc2209_read_register(TMC_DRV_STATUS, status);
}

bool tmc2209_is_overtemperature(void) {
    uint32_t status;
    if (tmc2209_get_driver_status(&status) == ESP_OK) {
        return (status & (TMC_DRV_OTPW | TMC_DRV_OT)) != 0;
    }
    return false;
}

bool tmc2209_is_short_circuit(void) {
    uint32_t status;
    if (tmc2209_get_driver_status(&status) == ESP_OK) {
        return (status & (TMC_DRV_S2GA | TMC_DRV_S2GB | 
                         TMC_DRV_S2VSA | TMC_DRV_S2VSB)) != 0;
    }
    return false;
}

bool tmc2209_is_open_load(void) {
    uint32_t status;
    if (tmc2209_get_driver_status(&status) == ESP_OK) {
        return (status & (TMC_DRV_OLA | TMC_DRV_OLB)) != 0;
    }
    return false;
}
