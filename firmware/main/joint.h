#pragma once
#include "core.h"

enum {JOINT_OFF, JOINT_MONITOR, JOINT_CONTROL};
typedef struct {
    uint32_t version;
    uint8_t mode, address;
    int8_t sign;
    uint8_t reserved;
    double ratio, zero_deg, tolerance_deg, max_error_deg, max_rpm;
} joint_config_t;
typedef struct {
    bool valid, previous_valid, aligned, motor_valid;
    uint16_t raw;
    uint32_t failures;
    int32_t error_code;
    double sample_at, previous_angle, unwrapped, motor_reference, joint_reference;
} joint_state_t;

void joint_defaults(joint_config_t *c);
bool joint_config_valid(const joint_config_t *c);
bool joint_setting(joint_config_t *c,const char *key,double value);
void joint_reset(joint_state_t *s);
double joint_max_age(const joint_config_t *c,const settings_t *motor);
bool joint_fresh(const joint_state_t *s,const joint_config_t *c,const settings_t *motor,double now);
void joint_sample(joint_state_t *s,const joint_config_t *c,const settings_t *motor,
                  uint16_t raw,bool valid,int32_t error,double now,bool motor_valid);
bool joint_align(joint_state_t *s,const joint_config_t *c,const settings_t *motor,
                 double motor_position,double now,bool motor_valid);
double joint_angle(const joint_state_t *s,const joint_config_t *c);
double joint_equivalent(const joint_state_t *s,const joint_config_t *c);
double joint_error(const joint_state_t *s,const joint_config_t *c,double motor_position);
void joint_bind(core_t *motor,const settings_t *settings,const joint_config_t *c,
                const joint_state_t *s,double now);
