#include "motor_control.h"
#include "gpio_init.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include <math.h>

static const char *TAG = "MOTOR";
static gptimer_handle_t step_timer = NULL;
static bool motor_enabled = false;
static bool current_direction = true;
static float current_step_rate = 0.0f;

static bool IRAM_ATTR step_timer_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx) {
    static bool step_state = false;
    
    if (current_step_rate > 0.0f) {
        step_state = !step_state;
        gpio_set_level(TMC_STEP_PIN, step_state);
    }
    
    return false;
}

esp_err_t motor_init(void) {
    ESP_LOGI(TAG, "Initializing motor control");
    
    // Create high-resolution timer for step generation
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1000000,  // 1 MHz, 1 µs resolution
    };
    
    esp_err_t ret = gptimer_new_timer(&timer_config, &step_timer);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create timer: %s", esp_err_to_name(ret));
        return ret;
    }
    
    gptimer_event_callbacks_t cbs = {
        .on_alarm = step_timer_callback,
    };
    gptimer_register_event_callbacks(step_timer, &cbs, NULL);
    gptimer_enable(step_timer);
    
    ESP_LOGI(TAG, "Motor control initialized");
    return ESP_OK;
}

void motor_enable(void) {
    gpio_set_level(TMC_ENN_PIN, 0);  // Active LOW
    motor_enabled = true;
    ESP_LOGI(TAG, "Motor enabled");
}

void motor_disable(void) {
    gpio_set_level(TMC_ENN_PIN, 1);  // Active LOW
    motor_enabled = false;
    current_step_rate = 0.0f;
    gptimer_stop(step_timer);
    ESP_LOGI(TAG, "Motor disabled");
}

void motor_set_direction(bool forward) {
    current_direction = forward;
    gpio_set_level(TMC_DIR_PIN, forward ? 1 : 0);
}

void motor_set_step_rate(float steps_per_sec) {
    if (!motor_enabled) return;
    
    steps_per_sec = fabsf(steps_per_sec);
    
    if (steps_per_sec < 1.0f) {
        gptimer_stop(step_timer);
        current_step_rate = 0.0f;
        return;
    }
    
    // Limit to reasonable range
    if (steps_per_sec > 10000.0f) steps_per_sec = 10000.0f;
    
    current_step_rate = steps_per_sec;
    
    // Timer toggles twice per step (rising and falling edge)
    uint64_t alarm_count = (uint64_t)(1000000.0f / (steps_per_sec * 2.0f));
    
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = alarm_count,
        .reload_count = 0,
        .flags.auto_reload_on_alarm = true,
    };
    
    gptimer_stop(step_timer);
    gptimer_set_alarm_action(step_timer, &alarm_config);
    gptimer_start(step_timer);
}

void motor_emergency_stop(void) {
    current_step_rate = 0.0f;
    gptimer_stop(step_timer);
    gpio_set_level(TMC_STEP_PIN, 0);
    ESP_LOGW(TAG, "Emergency stop");
}

bool motor_is_enabled(void) {
    return motor_enabled;
}
