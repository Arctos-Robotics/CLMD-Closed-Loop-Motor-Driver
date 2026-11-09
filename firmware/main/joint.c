#include "joint.h"
#include <math.h>
#include <string.h>

static double normalize(double degrees) {
    double value=fmod(degrees,360.0); return value<0?value+360:value;
}
void joint_defaults(joint_config_t *c) {
    *c=(joint_config_t){.version=1,.mode=JOINT_OFF,.address=6,.sign=1,
        .ratio=1,.zero_deg=0,.tolerance_deg=0.1,.max_error_deg=0.5,.max_rpm=60};
}
bool joint_config_valid(const joint_config_t *c) {
    return c->version==1 && c->mode<=JOINT_CONTROL && (c->address==6 || c->address==70) &&
        (c->sign==1 || c->sign==-1) && isfinite(c->ratio) && c->ratio>=1 && c->ratio<=100000 &&
        isfinite(c->zero_deg) && fabs(c->zero_deg)<=360 && isfinite(c->tolerance_deg) &&
        c->tolerance_deg>=0.022 && c->tolerance_deg<=5 && isfinite(c->max_error_deg) &&
        c->max_error_deg>c->tolerance_deg && c->max_error_deg<=30 &&
        isfinite(c->max_rpm) && c->max_rpm>=5 && c->max_rpm<=600;
}
bool joint_setting(joint_config_t *c,const char *key,double value) {
    if(!isfinite(value)) return false;
    joint_config_t next=*c;
    if(!strcmp(key,"joint_mode")) {
        if(value!=floor(value) || value<0 || value>2) return false;
        next.mode=value;
    } else if(!strcmp(key,"joint_address")) {
        if(value!=6 && value!=70) return false;
        next.address=value;
    } else if(!strcmp(key,"joint_sign")) {
        if(value!=1 && value!=-1) return false;
        next.sign=value;
    } else if(!strcmp(key,"joint_ratio")) next.ratio=value;
    else if(!strcmp(key,"joint_zero_deg")) next.zero_deg=value;
    else if(!strcmp(key,"joint_tolerance_deg")) next.tolerance_deg=value;
    else if(!strcmp(key,"joint_max_error_deg")) next.max_error_deg=value;
    else if(!strcmp(key,"joint_max_rpm")) next.max_rpm=value;
    else return false;
    if(!joint_config_valid(&next)) return false;
    *c=next; return true;
}
void joint_reset(joint_state_t *s) {memset(s,0,sizeof(*s));}
double joint_max_age(const joint_config_t *c,const settings_t *motor) {
    // Stay below half an output revolution at the configured speed ceiling.
    return fmin(0.05,0.4*60*c->ratio/motor->max_rpm);
}
bool joint_fresh(const joint_state_t *s,const joint_config_t *c,const settings_t *motor,double now) {
    return c->mode!=JOINT_OFF && s->valid && now>=s->sample_at && now-s->sample_at<=joint_max_age(c,motor);
}
double joint_angle(const joint_state_t *s,const joint_config_t *c) {
    return normalize((s->raw*360.0/16384-c->zero_deg)*c->sign);
}
void joint_sample(joint_state_t *s,const joint_config_t *c,const settings_t *motor,
                  uint16_t raw,bool valid,int32_t error,double now,bool motor_valid) {
    bool gap=s->previous_valid && (now<=s->sample_at || now-s->sample_at>joint_max_age(c,motor));
    s->motor_valid=motor_valid;
    if(!valid || raw>16383 || !isfinite(now) || gap) {
        s->valid=false; s->previous_valid=false; s->aligned=false;
        s->error_code=error?error:-2; s->sample_at=now; ++s->failures;
        return;
    }
    double angle=normalize((raw*360.0/16384-c->zero_deg)*c->sign);
    if(s->previous_valid) {
        double delta=fmod(angle-s->previous_angle+540,360)-180;
        double maximum=motor->max_rpm/c->ratio*6*(now-s->sample_at)+0.5;
        if(fabs(delta)>maximum) {
            s->valid=false; s->previous_valid=false; s->aligned=false;
            s->error_code=-3; s->sample_at=now; ++s->failures; return;
        }
        s->unwrapped+=delta;
    } else s->unwrapped=angle;
    s->raw=raw; s->previous_angle=angle; s->sample_at=now;
    s->valid=s->previous_valid=true; s->error_code=0;
    if(!motor_valid) s->aligned=false;
}
bool joint_align(joint_state_t *s,const joint_config_t *c,const settings_t *motor,
                 double motor_position,double now,bool motor_valid) {
    if(!motor_valid || !joint_fresh(s,c,motor,now) || !isfinite(motor_position)) return false;
    s->motor_reference=motor_position; s->joint_reference=s->unwrapped; s->aligned=true; return true;
}
double joint_equivalent(const joint_state_t *s,const joint_config_t *c) {
    return s->motor_reference+(s->unwrapped-s->joint_reference)*c->ratio*COUNTS_TURN/360;
}
double joint_error(const joint_state_t *s,const joint_config_t *c,double motor_position) {
    return s->unwrapped-s->joint_reference-(motor_position-s->motor_reference)*360/COUNTS_TURN/c->ratio;
}
void joint_bind(core_t *motor,const settings_t *settings,const joint_config_t *c,
                const joint_state_t *s,double now) {
    motor->joint_control=c->mode==JOINT_CONTROL;
    motor->joint_sensor_ok=joint_fresh(s,c,settings,now);
    motor->joint_ready=motor->joint_sensor_ok && s->aligned;
    motor->joint_feedback=joint_equivalent(s,c);
    motor->joint_tolerance=c->tolerance_deg*c->ratio*COUNTS_TURN/360;
    motor->joint_max_error=c->max_error_deg*c->ratio*COUNTS_TURN/360;
    motor->joint_max_rpm=c->max_rpm;
}
