#include "motion_planner.h"
#include "encoder.h"
#include "esp_timer.h"
#include "esp_log.h"
#include <math.h>

static const char *TAG = "MOTION";
static motion_profile_t profile = {0};

void motion_init(void) {
    profile.is_moving = false;
    profile.current_position = 0.0f;
    profile.current_velocity = 0.0f;
    ESP_LOGI(TAG, "Motion planner initialized");
}

void motion_move_absolute(float target_deg, float max_rpm, float accel_deg_s2) {
    profile.start_position = encoder_read_degrees();
    profile.target_position = target_deg;
    profile.max_velocity = max_rpm * 6.0f;  // RPM to deg/s
    profile.acceleration = accel_deg_s2;
    profile.start_time_us = esp_timer_get_time();
    profile.is_moving = true;
    
    ESP_LOGI(TAG, "Move absolute: %.2f -> %.2f deg at %.1f RPM", 
             profile.start_position, target_deg, max_rpm);
}

void motion_move_relative(float delta_deg, float max_rpm, float accel_deg_s2) {
    float current = encoder_read_degrees();
    motion_move_absolute(current + delta_deg, max_rpm, accel_deg_s2);
}

void motion_update(void) {
    if (!profile.is_moving) return;
    
    int64_t now = esp_timer_get_time();
    float elapsed = (now - profile.start_time_us) / 1000000.0f;
    
    float distance = profile.target_position - profile.start_position;
    float direction = (distance > 0) ? 1.0f : -1.0f;
    distance = fabsf(distance);
    
    // Calculate trapezoidal profile times
    float accel_time = profile.max_velocity / profile.acceleration;
    float accel_distance = 0.5f * profile.acceleration * accel_time * accel_time;
    
    // Check if we need triangular profile
    if (distance < 2 * accel_distance) {
        accel_distance = distance / 2.0f;
        accel_time = sqrtf(2.0f * accel_distance / profile.acceleration);
    }
    
    float const_vel_distance = distance - 2 * accel_distance;
    float const_vel_time = const_vel_distance / profile.max_velocity;
    float total_time = 2 * accel_time + const_vel_time;
    
    // Calculate current position
    if (elapsed < accel_time) {
        // Acceleration phase
        profile.current_position = profile.start_position + 
            direction * 0.5f * profile.acceleration * elapsed * elapsed;
        profile.current_velocity = direction * profile.acceleration * elapsed;
    }
    else if (elapsed < accel_time + const_vel_time) {
        // Constant velocity phase
        float t = elapsed - accel_time;
        profile.current_position = profile.start_position + 
            direction * (accel_distance + profile.max_velocity * t);
        profile.current_velocity = direction * profile.max_velocity;
    }
    else if (elapsed < total_time) {
        // Deceleration phase
        float t = elapsed - (accel_time + const_vel_time);
        float decel_dist = profile.max_velocity * t - 0.5f * profile.acceleration * t * t;
        profile.current_position = profile.start_position + 
            direction * (distance - accel_distance + decel_dist);
        profile.current_velocity = direction * (profile.max_velocity - profile.acceleration * t);
    }
    else {
        // Motion complete
        profile.current_position = profile.target_position;
        profile.current_velocity = 0.0f;
        profile.is_moving = false;
        ESP_LOGI(TAG, "Motion complete at %.2f deg", profile.current_position);
    }
}

float motion_get_target_position(void) {
    return profile.current_position;
}

bool motion_is_moving(void) {
    return profile.is_moving;
}

void motion_stop(void) {
    profile.is_moving = false;
    profile.current_velocity = 0.0f;
    ESP_LOGI(TAG, "Motion stopped");
}
