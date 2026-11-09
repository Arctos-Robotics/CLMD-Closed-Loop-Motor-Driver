#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "current.h"

#define CLMD_VERSION "2.0.3"
#define COUNTS_TURN 16384.0
enum {FAULT_NONE, FAULT_ESTOP, FAULT_ENCODER, FAULT_DRIVER, FAULT_MIN,
      FAULT_MAX, FAULT_HOST, FAULT_STALL, FAULT_HOME, FAULT_CONTROL,
      FAULT_JOINT_ENCODER, FAULT_JOINT_ERROR};
enum {IDLE, POSITION, VELOCITY, HOME_RELEASE, HOME_SEEK, HOME_BACKOFF, HOME_LATCH};
typedef struct {
    uint32_t version;
    uint16_t node, run_ma, hold_ma, fullsteps, max_rpm, accel_rpm_s;
    uint16_t watchdog_ms, home_rpm, home_timeout_s;
    int8_t motor_sign, encoder_sign;
    uint8_t min_enabled, max_enabled, home_dir, require_home;
    uint16_t min_threshold, max_threshold, hysteresis;
    double home_travel_turns, min_counts, max_counts;
} settings_t;
typedef struct {
    double position, target, velocity, measured_rpm, home_start, release_at;
    double last_motion_position, stationary_seconds, deadline;
    double speed_limit, acceleration;
    int32_t adc_min, adc_max;
    uint16_t encoder_raw;
    uint8_t mode, fault, min_active, max_active, min_filter, max_filter, ambiguous_filter;
    bool enabled, homed, complete, encoder_ok, inputs_ok;
    bool encoder_started;
    bool joint_control, joint_sensor_ok, joint_ready;
    double joint_feedback, joint_tolerance, joint_max_error, joint_max_rpm;
} core_t;

double core_feedback(const core_t *s);

void settings_defaults(settings_t *c);
bool settings_valid(const settings_t *c);
void core_init(core_t *s);
void core_stop(core_t *s, uint8_t fault);
bool core_enable(core_t *s, const settings_t *c);
bool core_startup_ready(const core_t *s);
bool core_encoder(core_t *s, const settings_t *c, uint16_t raw, bool valid, double dt);
void core_inputs(core_t *s, const settings_t *c, int minimum, int maximum, bool valid);
bool core_move(core_t *s, const settings_t *c, double target, unsigned rpm, unsigned acc, double now);
bool core_speed(core_t *s, const settings_t *c, double rpm, unsigned acc, double now);
bool core_home(core_t *s, const settings_t *c, double now);
void core_tick(core_t *s, const settings_t *c, double now, double dt, double host_age);
uint8_t core_status(const core_t *s);
uint8_t wire_crc(unsigned id, const uint8_t *data, size_t length);
bool wire_valid(unsigned id, const uint8_t *data, size_t length);
int32_t wire_i24(const uint8_t *data);
void wire_position(unsigned id, double counts, uint8_t command, uint8_t out[8]);
uint8_t sensor_crc(const uint8_t *bytes, size_t size);
