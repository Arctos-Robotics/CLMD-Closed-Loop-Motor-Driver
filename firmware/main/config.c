#include "config.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "CONFIG";
static const char *NVS_NAMESPACE = "motor_cfg";

motor_config_t g_config;

void config_set_defaults(void) {
    g_config.magic = CONFIG_MAGIC;
    g_config.version = CONFIG_VERSION;
    g_config.can_id = 1;
    strncpy(g_config.name, "Joint 1", sizeof(g_config.name));
    g_config.motor_current_ma = 1400;
    g_config.microsteps = 16;
    g_config.steps_per_degree = (200.0f * 16) / 360.0f;
    g_config.invert_direction = false;
    g_config.max_rpm = 600;
    g_config.max_accel = 500;
    g_config.min_position_deg = -180.0f;
    g_config.max_position_deg = 180.0f;
    g_config.home_direction = 0;
    g_config.home_speed_rpm = 100;
    g_config.home_offset_deg = 0.0f;
    g_config.auto_home_on_boot = false;
    g_config.pid_kp = 2.0f;
    g_config.pid_ki = 0.1f;
    g_config.pid_kd = 0.05f;
    g_config.pid_output_limit = 1000.0f;
    g_config.encoder_offset = 0;
    g_config.encoder_invert = false;
    g_config.position_error_limit = 5.0f;
    g_config.watchdog_timeout_ms = 1000;
}

esp_err_t config_init(void) {
    ESP_LOGI(TAG, "Initializing NVS");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition was truncated, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Try to load config, use defaults if not found
    if (config_load() != ESP_OK) {
        ESP_LOGI(TAG, "No saved config found, using defaults");
        config_set_defaults();
    }
    
    return ESP_OK;
}

esp_err_t config_save(void) {
    nvs_handle_t handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) return ret;
    
    ret = nvs_set_blob(handle, "config", &g_config, sizeof(motor_config_t));
    if (ret == ESP_OK) {
        ret = nvs_commit(handle);
        ESP_LOGI(TAG, "Configuration saved");
    }
    
    nvs_close(handle);
    return ret;
}

esp_err_t config_load(void) {
    nvs_handle_t handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (ret != ESP_OK) return ret;
    
    size_t required_size = sizeof(motor_config_t);
    ret = nvs_get_blob(handle, "config", &g_config, &required_size);
    
    if (ret == ESP_OK && g_config.magic == CONFIG_MAGIC) {
        ESP_LOGI(TAG, "Configuration loaded: CAN ID=%d, Name=%s", 
                 g_config.can_id, g_config.name);
    } else {
        ret = ESP_ERR_NOT_FOUND;
    }
    
    nvs_close(handle);
    return ret;
}
