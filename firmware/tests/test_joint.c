#include "joint.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

static settings_t config;
static joint_config_t joint_config;
static joint_state_t joint;
static core_t motor;
static uint16_t raw_angle(double degrees) {
    double wrapped=fmod(degrees,360); if(wrapped<0) wrapped+=360;
    return (uint16_t)lround(wrapped*16384/360)&16383;
}
static void ready(unsigned mode) {
    settings_defaults(&config); core_init(&motor); joint_defaults(&joint_config); joint_reset(&joint);
    joint_config.mode=mode; joint_config.ratio=24; joint_config.tolerance_deg=0.05;
    assert(core_encoder(&motor,&config,100,true,.005));
    core_inputs(&motor,&config,1737,821,true); core_inputs(&motor,&config,1737,821,true);
    joint_sample(&joint,&joint_config,&config,raw_angle(359.9),true,0,1,true);
    assert(joint_align(&joint,&joint_config,&config,0,1,true));
    joint_bind(&motor,&config,&joint_config,&joint,1);
    assert(core_enable(&motor,&config));
}
static void simulate(double target_motor_degrees,double *output,double *time) {
    assert(core_move(&motor,&config,target_motor_degrees/360*COUNTS_TURN,150,20,*time));
    for(int i=0;i<15000 && motor.mode!=IDLE;i++) {
        *time+=.005;
        motor.position+=motor.velocity*COUNTS_TURN/60*.005;
        motor.measured_rpm=motor.velocity;
        // Play/backlash plant: output moves only after 0.2 degrees of lost motion.
        double ideal=motor.position*360/COUNTS_TURN/24;
        if(ideal-*output>.2) *output=ideal-.2;
        if(ideal-*output<-.2) *output=ideal+.2;
        joint_sample(&joint,&joint_config,&config,raw_angle(359.9+*output),true,0,*time,true);
        joint_bind(&motor,&config,&joint_config,&joint,*time);
        core_tick(&motor,&config,*time,.005,0);
    }
    assert(motor.complete && motor.enabled && !motor.fault);
}
int main(void) {
    joint_defaults(&joint_config); assert(joint_config_valid(&joint_config));
    assert(joint_config.mode==JOINT_OFF);
    for(unsigned mode=0;mode<3;mode++) assert(joint_setting(&joint_config,"joint_mode",mode));
    assert(!joint_setting(&joint_config,"joint_mode",257));
    assert(!joint_setting(&joint_config,"joint_ratio",NAN));
    assert(!joint_setting(&joint_config,"joint_ratio",0));
    assert(!joint_setting(&joint_config,"joint_sign",0));
    assert(!joint_setting(&joint_config,"joint_address",7));
    assert(!joint_setting(&joint_config,"joint_tolerance_deg",.6));
    ready(JOINT_CONTROL);
    double output=0,time=1;
    simulate(24,&output,&time);
    assert(fabs(output-1)<=.07);
    assert(motor.position*360/COUNTS_TURN>27); // Extra motor travel takes up backlash.
    assert(fabs(core_feedback(&motor)-COUNTS_TURN/15)<=joint_config.tolerance_deg*24*COUNTS_TURN/360+1);
    simulate(-24,&output,&time);
    assert(fabs(output+1)<=.07 && motor.position*360/COUNTS_TURN< -27);
    // Monitoring alone does not correct the motor-coordinate endpoint.
    ready(JOINT_MONITOR); output=0;time=1; simulate(24,&output,&time);
    assert(output<.85 && fabs(motor.position*360/COUNTS_TURN-24)<.25);
    // Freshness, bad transfers and implausible jumps invalidate alignment.
    ready(JOINT_CONTROL);
    joint_sample(&joint,&joint_config,&config,0,false,259,1.01,true);
    joint_bind(&motor,&config,&joint_config,&joint,1.01);
    core_tick(&motor,&config,1.01,.005,0);
    assert(!motor.enabled && motor.fault==FAULT_JOINT_ENCODER && !joint.aligned);
    ready(JOINT_CONTROL);
    joint_bind(&motor,&config,&joint_config,&joint,1.1);
    core_tick(&motor,&config,1.1,.005,0); assert(motor.fault==FAULT_JOINT_ENCODER);
    ready(JOINT_CONTROL);
    motor.position=joint_config.max_error_deg*24*COUNTS_TURN/360+1;
    core_tick(&motor,&config,1.01,.005,0);
    assert(!motor.enabled && motor.fault==FAULT_JOINT_ERROR);
    // Motor travel limits also apply when the joint itself is at its target.
    ready(JOINT_CONTROL); config.max_counts=1;
    motor.position=2; motor.mode=POSITION; motor.target=core_feedback(&motor); motor.deadline=10;
    core_tick(&motor,&config,1.01,.005,0);
    assert(!motor.enabled && motor.fault==FAULT_CONTROL);
    // Completed output holds re-enter correction after external displacement.
    ready(JOINT_CONTROL); motor.complete=true;
    motor.joint_feedback=3*motor.joint_tolerance;
    core_tick(&motor,&config,1.01,.005,0);
    assert(motor.mode==POSITION && motor.velocity<0 && motor.enabled);
    ready(JOINT_CONTROL);
    joint_sample(&joint,&joint_config,&config,raw_angle(180),true,0,1.01,true);
    assert(!joint.valid && !joint.aligned && joint.error_code==-3);
    ready(JOINT_CONTROL);
    joint_sample(&joint,&joint_config,&config,raw_angle(359.9),true,0,1.1,true);
    assert(!joint.aligned && !joint.valid);
    // Re-enable and zero-speed hold the measured joint coordinate, not a motor correction offset.
    ready(JOINT_CONTROL); motor.position=100;
    assert(core_speed(&motor,&config,0,20,1)); assert(motor.target==core_feedback(&motor));
    core_stop(&motor,0); assert(core_enable(&motor,&config)); assert(motor.target==core_feedback(&motor));
    // An outer-loop hold requires heartbeat too.
    motor.complete=true; core_tick(&motor,&config,3,.005,2);
    assert(!motor.enabled && motor.fault==FAULT_HOST);
    // Homing may establish alignment from an unknown reference, but needs the sensor.
    ready(JOINT_CONTROL); joint.aligned=false; joint_bind(&motor,&config,&joint_config,&joint,1);
    core_stop(&motor,0); assert(!core_enable(&motor,&config)); assert(core_home(&motor,&config,1));
    motor.joint_sensor_ok=false; core_tick(&motor,&config,1.01,.005,0);
    assert(motor.fault==FAULT_JOINT_ENCODER);
    // Polarity and single-turn wrap.
    ready(JOINT_MONITOR); joint_config.sign=-1; joint_reset(&joint);
    joint_sample(&joint,&joint_config,&config,16380,true,0,1,true);
    assert(joint_align(&joint,&joint_config,&config,0,1,true));
    joint_sample(&joint,&joint_config,&config,10,true,0,1.01,true);
    assert(joint.unwrapped-joint.joint_reference<0);
    puts("JOINT TESTS PASS: 24:1 backlash/reversal simulation, motor-only/monitor behavior, alignment, wrap, polarity, stale/error/jump rejection, correction limit, heartbeat and homing guards");
}
