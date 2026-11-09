#include "core.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

void settings_defaults(settings_t *c) {
    *c = (settings_t){.version=2, .node=1, .run_ma=1000, .hold_ma=200,
        .fullsteps=200, .max_rpm=1200, .accel_rpm_s=400, .watchdog_ms=1500,
        .home_rpm=30, .home_timeout_s=25, .motor_sign=1, .encoder_sign=1,
        .min_enabled=1, .max_enabled=0, .home_dir=1, .require_home=0,
        .min_threshold=874, .max_threshold=874, .hysteresis=100,
        .home_travel_turns=12, .min_counts=-8388608, .max_counts=8388607};
}
bool settings_valid(const settings_t *c) {
    return c->version==2 && c->node>=1 && c->node<=127 && c->run_ma>=50 && c->run_ma<=CLMD_MAX_CURRENT_MA &&
        c->hold_ma<=c->run_ma && (c->fullsteps==200 || c->fullsteps==400) &&
        c->max_rpm>=5 && c->max_rpm<=1200 && c->accel_rpm_s>=50 && c->accel_rpm_s<=600 &&
        c->watchdog_ms>=500 && c->watchdog_ms<=10000 && c->home_rpm>=1 && c->home_rpm<=30 &&
        c->home_timeout_s>=1 && c->home_timeout_s<=25 && abs(c->motor_sign)==1 && abs(c->encoder_sign)==1 &&
        c->min_enabled<=1 && c->max_enabled<=1 && c->home_dir<=1 && c->require_home<=1 &&
        c->hysteresis>=10 && c->hysteresis<=500 && c->min_threshold>c->hysteresis &&
        c->max_threshold>c->hysteresis && c->min_threshold+c->hysteresis<4095 &&
        c->max_threshold+c->hysteresis<4095 && isfinite(c->home_travel_turns) &&
        c->home_travel_turns>0 && c->home_travel_turns<=100 && isfinite(c->min_counts) &&
        isfinite(c->max_counts) && c->min_counts>=-8388608 && c->max_counts<=8388607 && c->min_counts<c->max_counts;
}
void core_init(core_t *s) {
    memset(s,0,sizeof(*s));
    s->min_active=s->max_active=1; // Unknown at boot is treated as active.
}
void core_stop(core_t *s, uint8_t fault) {
    s->velocity=0; s->mode=IDLE; s->enabled=false;
    s->target=s->joint_control && s->joint_ready?s->joint_feedback:s->position; s->complete=false;
    if (fault) s->fault=fault;
}
bool core_enable(core_t *s, const settings_t *c) {
    if (!settings_valid(c) || !s->encoder_ok || !s->inputs_ok || (s->joint_control && !s->joint_ready)) return false;
    s->fault=0; s->enabled=true; s->target=core_feedback(s); s->mode=IDLE;
    s->speed_limit=c->max_rpm; s->acceleration=c->accel_rpm_s;
    s->velocity=0; s->stationary_seconds=0; s->complete=false;
    return true;
}
bool core_startup_ready(const core_t *s) {
    return !s->enabled && s->mode==IDLE && s->encoder_ok && s->inputs_ok &&
        (!s->joint_control || s->joint_ready) &&
        (s->fault==FAULT_NONE || s->fault==FAULT_DRIVER);
}
bool core_encoder(core_t *s, const settings_t *c, uint16_t raw, bool valid, double dt) {
    if (!valid || raw>32767 || dt<=0 || dt>0.05) {
        s->encoder_ok=false; s->encoder_started=false; s->homed=false;
        if (s->enabled) core_stop(s,FAULT_ENCODER);
        return false;
    }
    if (!s->encoder_started) {s->encoder_started=true; s->encoder_raw=raw;}
    int delta=(int)raw-s->encoder_raw;
    if (delta>16384) delta-=32768;
    if (delta< -16384) delta+=32768;
    // Reject physically impossible jumps at the configured speed ceiling.
    if (abs(delta)>c->max_rpm/60.0*32768*dt+300) {
        s->encoder_ok=false; s->encoder_started=false; s->homed=false;
        if(s->enabled) core_stop(s,FAULT_ENCODER);
        return false;
    }
    s->position += delta*0.5*c->encoder_sign;
    double rpm=delta*c->encoder_sign/32768.0/dt*60;
    s->measured_rpm=0.8*s->measured_rpm+0.2*rpm;
    s->encoder_raw=raw; s->encoder_ok=true;
    return true;
}
static void input_filter(int raw, int threshold, int hysteresis, uint8_t *active, uint8_t *filter) {
    bool next=*active;
    if (raw<=threshold-hysteresis) next=true;
    if (raw>=threshold+hysteresis) next=false;
    if (next!=!!*active) {
        if (++*filter>=2) {*active=next; *filter=0;}
    } else *filter=0;
}
void core_inputs(core_t *s, const settings_t *c, int minimum, int maximum, bool valid) {
    s->inputs_ok=valid && minimum>=0 && minimum<=4095 && maximum>=0 && maximum<=4095;
    if (!s->inputs_ok) {if(s->enabled) core_stop(s,FAULT_CONTROL); return;}
    s->adc_min=minimum; s->adc_max=maximum;
    bool ambiguous=(c->min_enabled && abs(minimum-c->min_threshold)<c->hysteresis) ||
                   (c->max_enabled && abs(maximum-c->max_threshold)<c->hysteresis);
    if(ambiguous) {
        if(s->ambiguous_filter<3) s->ambiguous_filter++;
        if(s->ambiguous_filter>=3) {s->inputs_ok=false; if(s->enabled) core_stop(s,FAULT_CONTROL);}
    } else s->ambiguous_filter=0;
    input_filter(minimum,c->min_threshold,c->hysteresis,&s->min_active,&s->min_filter);
    input_filter(maximum,c->max_threshold,c->hysteresis,&s->max_active,&s->max_filter);
}
double core_feedback(const core_t *s) {return s->joint_control?s->joint_feedback:s->position;}
static bool ready(core_t *s, const settings_t *c) {
    return !s->fault && s->encoder_ok && s->inputs_ok && (!c->require_home || s->homed) &&
        (!s->joint_control || s->joint_ready);
}
static double acc_value(const settings_t *c, unsigned acc) {
    // Studio's byte increases aggressiveness. Explicitly bound the physical ramp.
    return acc ? fmin(c->accel_rpm_s,50+acc*10.0) : c->accel_rpm_s;
}
static bool blocked(const core_t *s, const settings_t *c, double velocity) {
    int home_sign=c->home_dir ? -1:1;
    return (c->min_enabled && s->min_active && velocity*home_sign>0) ||
           (c->max_enabled && s->max_active && velocity*home_sign<0);
}
bool core_move(core_t *s, const settings_t *c, double target, unsigned rpm, unsigned acc, double now) {
    if (!ready(s,c) || !isfinite(target) || target<c->min_counts || target>c->max_counts || !rpm || rpm>3000) return false;
    if (blocked(s,c,target-core_feedback(s))) return false;
    s->target=target; s->speed_limit=fmin(rpm,c->max_rpm); s->acceleration=acc_value(c,acc);
    s->enabled=true; s->mode=POSITION; s->complete=false; s->stationary_seconds=0;
    s->deadline=now+120; return true;
}
bool core_speed(core_t *s, const settings_t *c, double rpm, unsigned acc, double now) {
    if (!ready(s,c) || !isfinite(rpm) || fabs(rpm)>3000 || blocked(s,c,rpm)) return false;
    if(rpm==0) {s->velocity=0; s->target=core_feedback(s); s->mode=IDLE; s->complete=true; return true;}
    s->speed_limit=fmin(fabs(rpm),c->max_rpm)*(rpm<0?-1:1);
    s->acceleration=acc_value(c,acc); s->enabled=true; s->mode=VELOCITY;
    s->deadline=now+120; s->complete=false; s->stationary_seconds=0; return true;
}
bool core_home(core_t *s, const settings_t *c, double now) {
    if (s->fault || !s->encoder_ok || !s->inputs_ok || !c->min_enabled ||
        (s->joint_control && !s->joint_sensor_ok)) return false;
    s->mode=s->min_active?HOME_RELEASE:HOME_SEEK; s->enabled=true; s->complete=false;
    s->homed=false; s->home_start=s->position; s->deadline=now+c->home_timeout_s;
    s->velocity=0; s->stationary_seconds=0; s->acceleration=100; return true;
}
void core_tick(core_t *s, const settings_t *c, double now, double dt, double host_age) {
    if (!s->enabled) {s->velocity=0; return;}
    if (!s->encoder_ok || !s->inputs_ok || !isfinite(dt) || dt<=0 || dt>0.05) {core_stop(s,FAULT_CONTROL); return;}
    if (s->joint_control) {
        bool homing=s->mode>=HOME_RELEASE;
        if((homing && !s->joint_sensor_ok) || (!homing && !s->joint_ready)) {core_stop(s,FAULT_JOINT_ENCODER); return;}
        if(!homing && fabs(s->position-s->joint_feedback)>s->joint_max_error) {core_stop(s,FAULT_JOINT_ERROR); return;}
        if(s->mode==IDLE && s->complete && fabs(s->target-s->joint_feedback)>2*s->joint_tolerance) {
            s->mode=POSITION; s->complete=false; s->deadline=now+120;
        }
    }
    if ((s->mode!=IDLE || s->joint_control) && host_age*1000>c->watchdog_ms) {core_stop(s,FAULT_HOST); return;}
    if (s->mode!=IDLE && now>s->deadline) {core_stop(s,s->mode>=HOME_RELEASE?FAULT_HOME:FAULT_CONTROL); return;}
    if (s->mode<HOME_RELEASE && (s->position<c->min_counts || s->position>c->max_counts)) {core_stop(s,FAULT_CONTROL); return;}
    double desired=0;
    int sign=c->home_dir?-1:1;
    bool homing=s->mode>=HOME_RELEASE;
    if (homing) {
        if (fabs(s->position-s->home_start)>c->home_travel_turns*COUNTS_TURN) {core_stop(s,FAULT_HOME); return;}
        if (c->max_enabled && s->max_active) {core_stop(s,FAULT_MAX); return;}
        if (s->mode==HOME_RELEASE) {
            desired=-sign*c->home_rpm;
            if (!s->min_active) {s->mode=HOME_SEEK; s->velocity=0;}
        } else if (s->mode==HOME_SEEK) {
            desired=sign*c->home_rpm;
            if (s->min_active) {s->mode=HOME_BACKOFF; s->release_at=NAN; s->velocity=0; desired=0;}
        } else if (s->mode==HOME_BACKOFF) {
            desired=-sign*fmin(c->home_rpm,10);
            if (!s->min_active && isnan(s->release_at)) s->release_at=s->position;
            if (!isnan(s->release_at) && fabs(s->position-s->release_at)>COUNTS_TURN/72) {
                s->mode=HOME_LATCH; s->velocity=0; desired=0;
            }
        } else {
            desired=sign*fmin(c->home_rpm,5);
            if (s->min_active) {
                s->position=0; s->target=0; s->velocity=0; s->mode=IDLE;
                s->homed=true; s->complete=true; return;
            }
        }
    } else if (s->mode==POSITION) {
        double error=s->target-core_feedback(s);
        double distance=fabs(error)/COUNTS_TURN;
        double ceiling=s->joint_control?fmin(s->speed_limit,s->joint_max_rpm):s->speed_limit;
        desired=fmin(ceiling,fmin(8*distance*60,sqrt(2*s->acceleration*distance*60)))*(error<0?-1:1);
        double tolerance=s->joint_control?s->joint_tolerance:10;
        if (fabs(error)<=tolerance && fabs(s->velocity)<2) {s->velocity=0; s->mode=IDLE; s->complete=true; return;}
    } else if (s->mode==VELOCITY) desired=s->speed_limit;
    if (!homing && blocked(s,c,desired)) {core_stop(s,desired*sign>0?FAULT_MIN:FAULT_MAX); return;}
    double change=s->acceleration*dt;
    s->velocity += fmax(-change,fmin(change,desired-s->velocity));
    if (fabs(s->velocity)>10 && (fabs(s->measured_rpm)<fabs(s->velocity)*0.1 || s->velocity*s->measured_rpm< -20)) {
        s->stationary_seconds += dt;
        if (s->stationary_seconds>0.5) core_stop(s,FAULT_STALL);
    } else s->stationary_seconds=0;
}
uint8_t core_status(const core_t *s) {
    if (s->fault) return 0;
    if (s->mode>=HOME_RELEASE) return 5;
    if (s->mode==IDLE) return 1;
    return 2; // Position/speed ramp is active; no false 'completed' while decelerating.
}
uint8_t wire_crc(unsigned id, const uint8_t *data, size_t length) {
    unsigned sum=id; for(size_t i=0;i<length;i++) sum+=data[i]; return sum;
}
bool wire_valid(unsigned id, const uint8_t *data, size_t length) {
    return id<=127 && length>=2 && length<=8 && wire_crc(id,data,length-1)==data[length-1];
}
int32_t wire_i24(const uint8_t *data) {
    int32_t value=((uint32_t)data[0]<<16)|((uint32_t)data[1]<<8)|data[2];
    return value&0x800000 ? value-0x1000000:value;
}
void wire_position(unsigned id, double counts, uint8_t command, uint8_t out[8]) {
    int64_t value=(int64_t)floor(counts); out[0]=command;
    if(command==0x30) {
        int32_t carry=(int32_t)floor(value/COUNTS_TURN);
        unsigned within=(unsigned)(value-(int64_t)carry*16384);
        for(int i=0;i<4;i++) out[1+i]=(uint32_t)carry>>(24-8*i);
        out[5]=within>>8; out[6]=within;
    } else for(int i=0;i<6;i++) out[1+i]=(uint64_t)value>>(40-8*i);
    out[7]=wire_crc(id,out,7);
}
uint8_t sensor_crc(const uint8_t *bytes, size_t size) {
    uint8_t crc=0xff;
    for(size_t i=0;i<size;i++) {crc^=bytes[i]; for(int j=0;j<8;j++) crc=(crc&0x80)?(crc<<1)^0x1d:crc<<1;}
    return crc^0xff;
}
