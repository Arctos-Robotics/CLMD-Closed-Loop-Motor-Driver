#include "core.h"
#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
static settings_t cfg;
static core_t s;
static void ready(void) {
    settings_defaults(&cfg); core_init(&s);
    assert(core_encoder(&s,&cfg,100,true,0.005));
    core_inputs(&s,&cfg,1737,821,true); core_inputs(&s,&cfg,1737,821,true);
    assert(!s.min_active); assert(s.max_active); assert(core_enable(&s,&cfg));
}
int main(void) {
    ready(); core_stop(&s,0); assert(core_startup_ready(&s));
    s.fault=FAULT_DRIVER; assert(core_startup_ready(&s));
    for(unsigned fault=1;fault<=FAULT_JOINT_ERROR;fault++) {
        if(fault==FAULT_DRIVER) continue;
        s.fault=fault; assert(!core_startup_ready(&s));
    }
    s.fault=0; s.encoder_ok=false; assert(!core_startup_ready(&s));
    s.encoder_ok=true; s.inputs_ok=false; assert(!core_startup_ready(&s));
    s.inputs_ok=true; s.joint_control=true; s.joint_ready=false; assert(!core_startup_ready(&s));
    s.joint_ready=true; assert(core_startup_ready(&s));
    assert(core_enable(&s,&cfg)); assert(s.mode==IDLE && s.velocity==0 && s.target==s.position);
    assert(!core_startup_ready(&s));
    ready(); assert(settings_valid(&cfg));
    cfg.node=0; assert(!settings_valid(&cfg)); cfg.node=1;
    cfg.run_ma=CLMD_MAX_CURRENT_MA; assert(settings_valid(&cfg));
    cfg.run_ma=CLMD_MAX_CURRENT_MA+1; assert(!settings_valid(&cfg)); cfg.run_ma=1000;
    cfg.encoder_sign=0; assert(!settings_valid(&cfg)); cfg.encoder_sign=1;
    cfg.home_travel_turns=NAN; assert(!settings_valid(&cfg)); cfg.home_travel_turns=12;
    cfg.min_counts=cfg.max_counts; assert(!settings_valid(&cfg)); ready();
    // Signed 24-bit wire values, including both endpoints.
    uint8_t maximum[]={0x7f,0xff,0xff}, minimum[]={0x80,0,0}, negative[]={0xff,0xc0,0};
    assert(wire_i24(maximum)==8388607); assert(wire_i24(minimum)==-8388608); assert(wire_i24(negative)==-16384);
    uint8_t frame[]={0xf3,1,0}; frame[2]=wire_crc(1,frame,2);
    assert(wire_valid(1,frame,3)); assert(!wire_valid(2,frame,3)); assert(!wire_valid(1,frame,1));
    frame[1]^=1; assert(!wire_valid(1,frame,3));
    // Both exact encoder wrap endpoints are valid sensor positions.
    s.encoder_raw=32760; assert(core_encoder(&s,&cfg,0,true,0.005)); assert(s.position==4);
    assert(core_encoder(&s,&cfg,32767,true,0.005)); assert(s.position==3.5);
    assert(!core_encoder(&s,&cfg,0,false,0.005)); assert(!s.enabled && s.fault==FAULT_ENCODER);
    ready(); assert(!core_move(&s,&cfg,NAN,100,20,0)); assert(!core_move(&s,&cfg,8388608,100,20,0));
    assert(!core_move(&s,&cfg,100,0,20,0));
    // Follow the exact compiled position controller with an ideal motor plant.
    assert(core_move(&s,&cfg,4096,150,20,0));
    for(int i=0;i<4000 && s.mode!=IDLE;i++) {
        s.position+=s.velocity*COUNTS_TURN/60*0.005; s.measured_rpm=s.velocity;
        core_tick(&s,&cfg,i*0.005,0.005,0);
    }
    assert(s.complete && !s.fault && fabs(s.position-4096)<=10);
    ready(); core_inputs(&s,&cfg,10,821,true); assert(!s.min_active);
    core_inputs(&s,&cfg,10,821,true); assert(s.min_active);
    assert(!core_move(&s,&cfg,-1000,100,20,0)); assert(core_move(&s,&cfg,1000,100,20,0));
    // A limit encountered during motion stops and disables locally.
    ready(); assert(core_move(&s,&cfg,-4096,100,20,0));
    core_inputs(&s,&cfg,10,821,true); core_inputs(&s,&cfg,10,821,true);
    core_tick(&s,&cfg,0.1,0.005,0); assert(!s.enabled && s.fault==FAULT_MIN && s.velocity==0);
    // Explicit recovery holds position and preserves the active directional limit.
    double limit_position=s.position;
    assert(core_enable(&s,&cfg)); assert(!s.fault && s.position==limit_position && s.target==limit_position);
    assert(!core_move(&s,&cfg,limit_position-1000,100,20,0.2));
    assert(core_move(&s,&cfg,limit_position+1000,100,20,0.2));
    // Disconnected signal at ~821 cannot release the conservative boot state.
    ready(); core_init(&s); core_inputs(&s,&cfg,821,821,true); core_inputs(&s,&cfg,821,821,true); assert(s.min_active);
    ready(); for(int i=0;i<3;i++) core_inputs(&s,&cfg,821,821,true);
    assert(!s.inputs_ok && !s.enabled && s.fault==FAULT_CONTROL);
    ready(); assert(core_speed(&s,&cfg,1200,255,0)); core_tick(&s,&cfg,2,0.005,2);
    assert(!s.enabled && s.fault==FAULT_HOST);
    ready(); assert(core_speed(&s,&cfg,1200,255,0));
    for(int i=0;i<200 && s.enabled;i++) core_tick(&s,&cfg,i*0.005,0.005,0);
    assert(s.fault==FAULT_STALL && !s.enabled);
    ready(); assert(core_speed(&s,&cfg,0,20,0)); assert(s.mode==IDLE);
    ready(); core_inputs(&s,&cfg,0,0,false); assert(!s.enabled && s.fault==FAULT_CONTROL);
    // Homing seeks, backs away beyond release, and slowly latches, then zeroes.
    ready(); assert(core_home(&s,&cfg,0)); assert(s.mode==HOME_SEEK);
    s.min_active=true; core_tick(&s,&cfg,0.01,0.005,0); assert(s.mode==HOME_BACKOFF);
    s.min_active=false; core_tick(&s,&cfg,0.02,0.005,0);
    s.position+=300; core_tick(&s,&cfg,0.03,0.005,0); assert(s.mode==HOME_LATCH);
    s.min_active=true; core_tick(&s,&cfg,0.04,0.005,0);
    assert(s.homed && s.complete && s.position==0 && s.velocity==0 && core_status(&s)==1);
    ready(); s.min_active=true; assert(core_home(&s,&cfg,0)); assert(s.mode==HOME_RELEASE);
    core_tick(&s,&cfg,26,0.005,0); assert(s.fault==FAULT_HOME && !s.enabled);
    ready(); assert(core_home(&s,&cfg,0)); s.position=cfg.home_travel_turns*COUNTS_TURN+1;
    core_tick(&s,&cfg,1,0.005,0); assert(s.fault==FAULT_HOME);
    ready(); cfg.require_home=1; assert(!core_move(&s,&cfg,100,100,20,0)); assert(core_home(&s,&cfg,0));
    core_stop(&s,FAULT_ESTOP); assert(!core_move(&s,&cfg,100,100,20,0));
    // Fixture frames are consumed by the actual Studio decoder in the Python test.
    double positions[]={-8388608,-16385,-16384,-1,0,1,16383,16384,8388607};
    for(unsigned i=0;i<sizeof(positions)/sizeof(*positions);i++) for(int command=0x30;command<=0x31;command++) {
        uint8_t out[8]; wire_position(1,positions[i],command,out);
        printf("POSITION %.0f ",positions[i]); for(int j=0;j<8;j++) printf("%02X",out[j]); printf("\n");
    }
    puts("CORE TESTS PASS"); return 0;
}
