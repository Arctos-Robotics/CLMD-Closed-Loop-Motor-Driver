#include "gpio_init.h"
#include "esp_log.h"

static const char *TAG = "GPIO";

void gpio_init_all(void) {
    ESP_LOGI(TAG, "Initializing GPIO pins");
    
    // TMC2209 outputs
    gpio_config_t tmc_out = {
        .pin_bit_mask = (1ULL << TMC_STEP_PIN) | (1ULL << TMC_DIR_PIN) | 
                        (1ULL << TMC_ENN_PIN) | (1ULL << TMC_MS1_PIN) | 
                        (1ULL << TMC_MS2_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&tmc_out);
    
    // Motor disabled by default (ENN is active LOW)
    gpio_set_level(TMC_ENN_PIN, 1);
    gpio_set_level(TMC_STEP_PIN, 0);
    gpio_set_level(TMC_DIR_PIN, 0);
    
    // Set 1/16 microstepping (MS1=0, MS2=0)
    gpio_set_level(TMC_MS1_PIN, 0);
    gpio_set_level(TMC_MS2_PIN, 0);
    
    // Limit switch inputs with pull-up
    gpio_config_t limit_in = {
        .pin_bit_mask = (1ULL << J_MIN_PIN) | (1ULL << J_MAX_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&limit_in);
    
    // RGB LED outputs
    gpio_config_t led_out = {
        .pin_bit_mask = (1ULL << LED_RED_PIN) | (1ULL << LED_GREEN_PIN) | 
                        (1ULL << LED_BLUE_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&led_out);
    
    // LED off by default
    gpio_set_level(LED_RED_PIN, 0);
    gpio_set_level(LED_GREEN_PIN, 0);
    gpio_set_level(LED_BLUE_PIN, 0);
    
    ESP_LOGI(TAG, "GPIO initialization complete");
}
