#include "encoder.h"
#include "gpio_init.h"
#include "driver/spi_master.h"
#include "esp_log.h"

static const char *TAG = "ENCODER";
static spi_device_handle_t spi_handle;
static int32_t zero_offset = 0;
static bool comm_ok = false;

esp_err_t encoder_init(void) {
    ESP_LOGI(TAG, "Initializing TLE5012B encoder");
    
    spi_bus_config_t bus_cfg = {
        .miso_io_num = ENCODER_MISO_PIN,
        .mosi_io_num = -1,  // TLE5012B uses 3-wire SPI
        .sclk_io_num = ENCODER_SCK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    
    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return ret;
    }
    
    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = 1000000,  // 1 MHz
        .mode = 1,  // CPOL=0, CPHA=1
        .spics_io_num = ENCODER_CS_PIN,
        .queue_size = 1,
        .flags = SPI_DEVICE_HALFDUPLEX,
    };
    
    ret = spi_bus_add_device(SPI2_HOST, &dev_cfg, &spi_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add SPI device: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Test communication
    uint16_t test_read = encoder_read_raw();
    if (test_read != 0xFFFF) {
        comm_ok = true;
        ESP_LOGI(TAG, "Encoder initialized, raw value: %u", test_read);
    } else {
        ESP_LOGW(TAG, "Encoder communication test failed");
    }
    
    return ESP_OK;
}

uint16_t encoder_read_raw(void) {
    uint16_t cmd = TLE5012_READ_ANGLE_VALUE;
    uint16_t rx_data[2] = {0};
    
    spi_transaction_t trans = {
        .length = 16,
        .tx_buffer = &cmd,
        .rx_buffer = rx_data,
        .rxlength = 32,
    };
    
    esp_err_t ret = spi_device_transmit(spi_handle, &trans);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI transaction failed");
        comm_ok = false;
        return 0xFFFF;
    }
    
    comm_ok = true;
    // Extract 15-bit angle (0-32767)
    uint16_t angle = rx_data[1] & 0x7FFF;
    return angle;
}

float encoder_read_degrees(void) {
    uint16_t raw = encoder_read_raw();
    if (raw == 0xFFFF) return 0.0f;
    
    // Apply zero offset
    int32_t adjusted = (int32_t)raw - zero_offset;
    
    // Wrap to 0-32767 range
    while (adjusted < 0) adjusted += 32768;
    while (adjusted >= 32768) adjusted -= 32768;
    
    // Convert to degrees (0-360)
    return (float)adjusted * 360.0f / 32768.0f;
}

void encoder_set_zero_offset(void) {
    uint16_t current = encoder_read_raw();
    if (current != 0xFFFF) {
        zero_offset = (int32_t)current;
        ESP_LOGI(TAG, "Zero offset set to: %ld", zero_offset);
    }
}

bool encoder_is_communication_ok(void) {
    return comm_ok;
}
