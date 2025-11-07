#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "gpio_init.h"
#include "encoder.h"
#include "motor_control.h"
#include "pid_controller.h"
#include "motion_planner.h"
#include "can_protocol.h"
#include "usb_config.h"
#include "config.h"
#include "safety.h"
#include "tmc2209_advanced.h"

static const char *TAG = "MAIN";
static pid_controller_t pid;

void control_loop_task(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1);  // 1 kHz
    
    ESP_LOGI(TAG, "Control loop started");
    
    while (1) {
        if (motor_is_enabled()) {
            // Read current position
            float current_pos = encoder_read_degrees();
            
            // Get target from motion planner
            float target_pos = motion_get_target_position();
            
            // Calculate error
            float error = target_pos - current_pos;
            
            // Run PID
            float pid_output = pid_compute(&pid, error);
            
            // Convert to step rate
            float step_rate = pid_output * g_config.steps_per_degree;
            
            // Set direction
            motor_set_direction(step_rate >= 0);
            
            // Set step rate
            motor_set_step_rate(step_rate);
            
            // Safety checks
            safety_check_position_error(error);
            
            // Update moving status
            g_status.is_moving = motion_is_moving();
        }
        
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void motion_planner_task(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(10);  // 100 Hz
    
    ESP_LOGI(TAG, "Motion planner started");
    
    while (1) {
        motion_update();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void status_update_task(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100);  // 10 Hz
    
    ESP_LOGI(TAG, "Status update started");
    
    while (1) {
        safety_check_limit_switches();
        safety_check_encoder();
        safety_check_watchdog();
        
        // Update LED
        if (g_status.has_error) {
            gpio_set_level(LED_RED_PIN, 1);
            gpio_set_level(LED_GREEN_PIN, 0);
            gpio_set_level(LED_BLUE_PIN, 0);
        } else if (g_status.is_moving) {
            gpio_set_level(LED_RED_PIN, 0);
            gpio_set_level(LED_GREEN_PIN, 0);
            gpio_set_level(LED_BLUE_PIN, 1);
        } else if (g_status.is_enabled) {
            gpio_set_level(LED_RED_PIN, 0);
            gpio_set_level(LED_GREEN_PIN, 1);
            gpio_set_level(LED_BLUE_PIN, 0);
        } else {
            gpio_set_level(LED_RED_PIN, 0);
            gpio_set_level(LED_GREEN_PIN, 0);
            gpio_set_level(LED_BLUE_PIN, 0);
        }
        
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Closed Loop Stepper Driver Starting...");
    
    // Initialize configuration
    ESP_ERROR_CHECK(config_init());
    
    // Initialize GPIO
    gpio_init_all();
    
    // Initialize safety system
    safety_init();
    
    // Initialize encoder
    ESP_ERROR_CHECK(encoder_init());
    
    // Initialize motor control
    ESP_ERROR_CHECK(motor_init());
    
    // Initialize TMC2209 UART communication
    ESP_LOGI(TAG, "Initializing TMC2209 advanced features...");
    ESP_ERROR_CHECK(tmc2209_init_uart());
    
    // Configure TMC2209 advanced features
    tmc2209_set_current(1400, 700);      // 1.4A run, 0.7A hold
    tmc2209_set_microstepping(4);        // 1/16 microstepping (mres=4)
    tmc2209_enable_stealthchop(500);     // StealthChop below 500 steps/s
    
    // Enable CoolStep for 50-75% power savings
    tmc2209_enable_coolstep(5, 10, 1);   // semin=5, semax=10, sedn=1
    
    // Enable StallGuard for load monitoring and collision detection
    tmc2209_enable_stallguard(10);       // threshold=10
    
    ESP_LOGI(TAG, "TMC2209 CoolStep enabled - expect 50-75%% power savings!");
    
    // Initialize motion planner
    motion_init();
    
    // Initialize PID controller
    pid_init(&pid, g_config.pid_kp, g_config.pid_ki, g_config.pid_kd, 
             g_config.pid_output_limit);
    
    // Initialize CAN bus
    ESP_ERROR_CHECK(can_init());
    
    // Initialize USB configuration interface
    ESP_ERROR_CHECK(usb_config_init());
    
    ESP_LOGI(TAG, "All systems initialized");
    ESP_LOGI(TAG, "CAN ID: %d, Name: %s", g_config.can_id, g_config.name);
    
    // Create FreeRTOS tasks
    xTaskCreatePinnedToCore(control_loop_task, "control", 4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(can_task, "can_rx", 4096, NULL, 4, NULL, 0);
    xTaskCreatePinnedToCore(motion_planner_task, "motion", 4096, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(usb_config_task, "usb", 4096, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(status_update_task, "status", 2048, NULL, 1, NULL, 0);
    
    ESP_LOGI(TAG, "System ready - Green LED = Idle, Blue = Moving, Red = Error");
}
