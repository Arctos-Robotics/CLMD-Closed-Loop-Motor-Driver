#include "core.h"
#include "settings.h"
#include "hardware.h"
#include "gpio_init.h"
#include "tmc2209_advanced.h"
#include "mt6701.h"
#include "joint.h"
#include "joint_storage.h"
#include "driver/twai.h"
#include "driver/usb_serial_jtag.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

enum {REPLY_CAN, REPLY_USB, REPLY_TEXT};
enum {CMD_NATIVE, CMD_SETTING, CMD_JOINT_START, CMD_JOINT_READ, CMD_JOINT_STOP, CMD_JOINT_ALIGN};
typedef struct {
    settings_t config;
    core_t core;
    uint16_t encoder_word, encoder_safety;
    uint32_t driver_status;
    bool encoder_frame_ok;
    double encoder_dt;
    double encoder_sample_at;
    bool joint_test;
    double joint_deadline;
    joint_config_t joint_config;
    joint_state_t joint;
    bool joint_bus_ready;
    bool driver_configured;
    bool startup_enabled, startup_pending, startup_settings_ok;
    bool settings_ok, joint_settings_ok;
} snapshot_t;
typedef struct {
    uint32_t epoch;
    uint16_t id;
    uint8_t kind, reply, length, data[8];
    char key[32]; double value;
} command_t;
static snapshot_t published;
static portMUX_TYPE state_lock=portMUX_INITIALIZER_UNLOCKED;
static portMUX_TYPE gate_lock=portMUX_INITIALIZER_UNLOCKED;
static QueueHandle_t commands;
static SemaphoreHandle_t usb_mutex;
static atomic_uint generation, urgent, last_host_ms, control_ms;
static atomic_bool active;
static atomic_bool executing;
static uint32_t milliseconds(void) {return (uint32_t)(esp_timer_get_time()/1000);}
static snapshot_t snapshot(void) {
    portENTER_CRITICAL(&state_lock); snapshot_t s=published; portEXIT_CRITICAL(&state_lock); return s;
}
static void publish(const snapshot_t *s) {
    portENTER_CRITICAL(&state_lock); published=*s; portEXIT_CRITICAL(&state_lock);
}
static void emergency(unsigned fault) {
    portENTER_CRITICAL(&gate_lock);
    gpio_set_level(TMC_ENN_PIN,1);
    atomic_store(&active,false);
    atomic_fetch_add(&generation,1); atomic_store(&urgent,fault);
    portEXIT_CRITICAL(&gate_lock);
}
static void gate(bool enabled,unsigned epoch) {
    portENTER_CRITICAL(&gate_lock);
    bool on=enabled && !atomic_load(&urgent) && epoch==atomic_load(&generation);
    gpio_set_level(TMC_ENN_PIN,!on); atomic_store(&active,on);
    portEXIT_CRITICAL(&gate_lock);
}
static void usb_text(const char *text) {
    if(!usb_mutex || xSemaphoreTake(usb_mutex,pdMS_TO_TICKS(150))!=pdTRUE) return;
    usb_serial_jtag_write_bytes(text,strlen(text),pdMS_TO_TICKS(100));
    xSemaphoreGive(usb_mutex);
}
static void send_frame(unsigned id,const uint8_t *bytes,size_t size,unsigned reply) {
    if(reply==REPLY_USB) {
        char out[96]; int n=snprintf(out,sizeof(out),"FRAME %u ",id);
        for(size_t i=0;i<size;i++) n+=snprintf(out+n,sizeof(out)-n,"%02X",bytes[i]);
        snprintf(out+n,sizeof(out)-n,"\r\n"); usb_text(out); return;
    }
    twai_message_t frame={.identifier=id,.data_length_code=size}; memcpy(frame.data,bytes,size);
    if(twai_transmit(&frame,0)!=ESP_OK && atomic_load(&active)) emergency(FAULT_HOST);
}
static void response(unsigned id,uint8_t cmd,uint8_t result,unsigned reply) {
    if(reply==REPLY_TEXT) {usb_text(result?"OK\r\n":"ERROR Command rejected\r\n"); return;}
    if(!id) return; // No replies to broadcast commands.
    uint8_t out[]={cmd,result,0}; out[2]=wire_crc(id,out,2); send_frame(id,out,3,reply);
}
static void submit(command_t *cmd) {
    cmd->epoch=atomic_load(&generation);
    if(xQueueSend(commands,cmd,0)!=pdTRUE) {
        emergency(FAULT_CONTROL); response(cmd->id,cmd->data[0],0,cmd->reply);
    }
}
static void native_request(unsigned id,const uint8_t *data,size_t size,unsigned reply) {
    snapshot_t s=snapshot();
    if((id!=s.config.node && id!=0) || !wire_valid(id,data,size)) return;
    uint8_t cmd=data[0];
    if(size==2 && id && cmd>=0xE9 && cmd<=0xEC) {
        uint8_t out[8]={cmd};
        if(cmd==0xE9) {
            bool fresh=esp_timer_get_time()/1e6-s.encoder_sample_at<=0.05;
            out[1]=(s.encoder_frame_ok && fresh) | ((s.core.encoder_ok && fresh)<<1) | (s.core.enabled<<2) |
                (s.core.inputs_ok<<3) | (s.driver_configured<<4);
            uint16_t raw=s.encoder_word&0x7fff;
            out[2]=raw>>8; out[3]=raw; out[4]=s.encoder_safety>>8; out[5]=s.encoder_safety;
            out[6]=s.core.fault;
        } else if(cmd==0xEA) {
            out[1]=s.core.adc_min>>8; out[2]=s.core.adc_min;
            out[3]=s.core.adc_max>>8; out[4]=s.core.adc_max;
            out[5]=s.core.min_active | (s.core.max_active<<1) | (s.core.inputs_ok<<2) |
                (s.config.min_enabled<<3) | (s.config.max_enabled<<4);
            out[6]=s.core.mode;
        } else if(cmd==0xEB) {
            for(int i=0;i<4;++i) out[1+i]=s.driver_status>>(24-8*i);
            out[5]=s.core.enabled | (s.driver_configured<<1); out[6]=s.core.fault;
        } else {
            out[1]=s.config.run_ma>>8; out[2]=s.config.run_ma;
            out[3]=s.config.hold_ma>>8; out[4]=s.config.hold_ma;
            out[5]=CLMD_MAX_CURRENT_MA>>8; out[6]=CLMD_MAX_CURRENT_MA&255;
        }
        out[7]=wire_crc(id,out,7); send_frame(id,out,8,reply);
        atomic_store(&last_host_ms,milliseconds()); return;
    }
    if(size==2 && cmd>=0xE0 && cmd<=0xE8 && cmd!=0xE4 && id) {
        double now=esp_timer_get_time()/1e6;
        bool fresh=joint_fresh(&s.joint,&s.joint_config,&s.config,now);
        uint8_t flags=(s.joint_config.mode!=0) | (fresh<<1) | ((fresh && s.joint.aligned)<<2) |
            ((s.joint_config.mode==JOINT_CONTROL)<<3);
        uint8_t out[8]={cmd,flags};
        if(cmd==0xE5) {
            wire_position(id,s.core.position,0x31,out); out[0]=0xE5;
        } else if(cmd==0xE0) {
            unsigned age=s.joint.sample_at>0?fmin(65535,fmax(0,(now-s.joint.sample_at)*1000)):65535;
            out[2]=s.joint.raw>>8; out[3]=s.joint.raw; out[4]=age>>8; out[5]=age;
            out[6]=s.joint.error_code?1:0;
        } else {
            double value;
            if(cmd==0xE1) value=joint_angle(&s.joint,&s.joint_config)*1000;
            else if(cmd==0xE2) value=joint_error(&s.joint,&s.joint_config,s.core.position)*1000;
            else if(cmd==0xE3) value=joint_equivalent(&s.joint,&s.joint_config);
            else if(cmd==0xE6) value=(s.core.position-s.joint.motor_reference)*360/COUNTS_TURN/s.joint_config.ratio*1000;
            else if(cmd==0xE7) value=(s.joint.unwrapped-s.joint.joint_reference)*1000;
            else value=(s.core.target-s.joint.motor_reference)*360/COUNTS_TURN/s.joint_config.ratio*1000;
            if(value>INT32_MAX || value<INT32_MIN) {flags|=0x80; out[1]=flags; value=0;}
            int32_t encoded=lround(value);
            for(int i=0;i<4;++i) out[2+i]=(uint32_t)encoded>>(24-8*i);
        }
        out[7]=wire_crc(id,out,7); send_frame(id,out,8,reply);
        atomic_store(&last_host_ms,milliseconds()); return;
    }
    if(size==2 && (cmd==0x30 || cmd==0x31 || cmd==0x32 || cmd==0x34 || cmd==0x3A || cmd==0xF1)) {
        if(!id) return;
        atomic_store(&last_host_ms,milliseconds());
        if((cmd==0x30 || cmd==0x31) && (!s.core.encoder_ok ||
            (s.core.joint_control && !joint_fresh(&s.joint,&s.joint_config,&s.config,esp_timer_get_time()/1e6)) ||
            (s.core.joint_control && !s.joint.aligned))) {response(id,cmd,0,reply); return;}
        uint8_t out[8]={cmd}; size_t n=0;
        if(cmd==0x30 || cmd==0x31) {wire_position(id,core_feedback(&s.core),cmd,out); n=8;}
        else if(cmd==0x32) {
            // Studio 3.3 connection.py decodes speed with struct.unpack('<h').
            int16_t rpm=lround(s.core.measured_rpm); out[1]=rpm; out[2]=(uint16_t)rpm>>8; n=4;
        } else {
            out[1]=cmd==0x34 ? (s.core.min_active | (s.core.max_active<<1)) :
                   cmd==0x3A ? s.core.enabled:core_status(&s.core); n=3;
            if(cmd==0xF1 && out[1]==1 && (atomic_load(&executing) || uxQueueMessagesWaiting(commands))) out[1]=2;
        }
        out[n-1]=wire_crc(id,out,n-1); send_frame(id,out,n,reply); return;
    }
    size_t expected=0;
    switch(cmd) {
        case 0xF7: case 0x91: case 0x92: case 0x3D: case 0xE4: expected=2; break;
        case 0xF3: case 0x82: case 0x84: case 0x8A: case 0x9B: expected=3; break;
        case 0x83: expected=4; break;
        case 0x90: expected=7; break;
        case 0xF6: expected=5; break;
        case 0xF4: case 0xF5: expected=8; break;
        default: response(id,cmd,0,reply); return;
    }
    if(size!=expected) {response(id,cmd,0,reply); return;}
    if(cmd==0xE4 && !id) return;
    atomic_store(&last_host_ms,milliseconds());
    if(cmd==0xF7) {emergency(FAULT_ESTOP); response(id,cmd,1,reply); return;}
    if(cmd==0xF3 && !data[1]) emergency(255); // Immediate disable, without latching a fault.
    command_t request={.kind=CMD_NATIVE,.reply=reply,.id=id,.length=size};
    memcpy(request.data,data,size); submit(&request);
}
static void config_json(const snapshot_t *s) {
    const settings_t *c=&s->config; char out[1800];
    snprintf(out,sizeof(out),"{\"version\":\"%s\",\"node\":%u,\"run_ma\":%u,\"hold_ma\":%u,\"fullsteps\":%u,\"max_rpm\":%u,\"accel_rpm_s\":%u,\"watchdog_ms\":%u,\"home_rpm\":%u,\"home_timeout_s\":%u,\"home_dir\":%u,\"home_travel_turns\":%.3f,\"motor_sign\":%d,\"encoder_sign\":%d,\"min_enabled\":%u,\"max_enabled\":%u,\"min_threshold\":%u,\"max_threshold\":%u,\"hysteresis\":%u,\"require_home\":%u,\"min_counts\":%.0f,\"max_counts\":%.0f}\r\n",
        CLMD_VERSION,c->node,c->run_ma,c->hold_ma,c->fullsteps,c->max_rpm,c->accel_rpm_s,c->watchdog_ms,
        c->home_rpm,c->home_timeout_s,c->home_dir,c->home_travel_turns,c->motor_sign,c->encoder_sign,
        c->min_enabled,c->max_enabled,c->min_threshold,c->max_threshold,c->hysteresis,c->require_home,c->min_counts,c->max_counts);
    const joint_config_t *j=&s->joint_config;
    size_t n=strlen(out)-3;
    snprintf(out+n,sizeof(out)-n,",\"joint_mode\":%u,\"joint_address\":%u,\"joint_sign\":%d,\"joint_ratio\":%.9g,\"joint_zero_deg\":%.9g,\"joint_tolerance_deg\":%.9g,\"joint_max_error_deg\":%.9g,\"joint_max_rpm\":%.9g,\"current_max_ma\":%u,\"run_actual_ma\":%.3f,\"hold_actual_ma\":%.3f}\r\n",
        j->mode,j->address,j->sign,j->ratio,j->zero_deg,j->tolerance_deg,j->max_error_deg,j->max_rpm,
        CLMD_MAX_CURRENT_MA,current_rms_ma(c->run_ma),current_rms_ma(c->hold_ma));
    n=strlen(out)-3;
    snprintf(out+n,sizeof(out)-n,",\"startup_enabled\":%d}\r\n",s->startup_enabled);
    usb_text(out);
}
static void status_json(const snapshot_t *s) {
    char out[1800]; const core_t *c=&s->core;
    snprintf(out,sizeof(out),"{\"version\":\"%s\",\"position_counts\":%.1f,\"target_counts\":%.1f,\"rpm\":%.2f,\"command_rpm\":%.2f,\"enabled\":%d,\"mode\":%u,\"fault\":%u,\"homed\":%d,\"encoder_ok\":%d,\"encoder_raw\":%u,\"encoder_word\":%u,\"encoder_safety\":%u,\"inputs_ok\":%d,\"min_adc\":%ld,\"max_adc\":%ld,\"min_active\":%u,\"max_active\":%u,\"driver_status\":%lu}\r\n",
        CLMD_VERSION,c->position,c->target,c->measured_rpm,c->velocity,c->enabled,c->mode,c->fault,c->homed,
        c->encoder_ok,c->encoder_raw,s->encoder_word,s->encoder_safety,c->inputs_ok,(long)c->adc_min,
        (long)c->adc_max,c->min_active,c->max_active,(unsigned long)s->driver_status);
    double now=esp_timer_get_time()/1e6;
    bool fresh=joint_fresh(&s->joint,&s->joint_config,&s->config,now);
    size_t n=strlen(out)-3;
    snprintf(out+n,sizeof(out)-n,",\"joint_mode\":%u,\"joint_valid\":%d,\"joint_raw\":%u,\"joint_angle_deg\":%.6f,\"joint_aligned\":%d,\"joint_error_code\":%ld,\"joint_failures\":%lu,\"joint_age_ms\":%.3f,\"joint_motor_frame_ok\":%d,\"joint_equivalent_counts\":%.3f,\"joint_measured_travel_deg\":%.6f,\"joint_expected_travel_deg\":%.6f,\"joint_error_deg\":%.6f,\"joint_target_travel_deg\":%.6f,\"settings_ok\":%d,\"joint_settings_ok\":%d}\r\n",
        s->joint_config.mode,fresh,s->joint.raw,joint_angle(&s->joint,&s->joint_config),
        fresh && s->joint.aligned,(long)s->joint.error_code,(unsigned long)s->joint.failures,
        s->joint.sample_at>0?fmax(0,(now-s->joint.sample_at)*1000):-1,s->encoder_frame_ok,
        joint_equivalent(&s->joint,&s->joint_config),s->joint.unwrapped-s->joint.joint_reference,
        (c->position-s->joint.motor_reference)*360/COUNTS_TURN/s->joint_config.ratio,
        joint_error(&s->joint,&s->joint_config,c->position),
        (c->target-s->joint.motor_reference)*360/COUNTS_TURN/s->joint_config.ratio,s->settings_ok,s->joint_settings_ok);
    size_t end=strlen(out)-3;
    snprintf(out+end,sizeof(out)-end,",\"encoder_frame_ok\":%d,\"encoder_frame_raw\":%u,\"encoder_dt_us\":%.1f,\"driver_configured\":%d,\"current_pending\":%d,\"startup_enabled\":%d,\"startup_pending\":%d,\"startup_settings_ok\":%d}\r\n",
        s->encoder_frame_ok && now-s->encoder_sample_at<=0.05,s->encoder_word&0x7fff,s->encoder_dt*1e6,s->driver_configured,!s->driver_configured,s->startup_enabled,s->startup_pending,s->startup_settings_ok);
    usb_text(out);
}
static void ascii_request(char *line) {
    snapshot_t s=snapshot();
    if(!strcmp(line,"VERSION")) {usb_text("OK CLMD_STUDIO " CLMD_VERSION " MKS_16384 ADC_LIMIT CRC_ENCODER MT6701_I2C_V1 MT6701_TEST_V2 JOINT_FEEDBACK_V1 JOINT_CONTROL_V1 MOTOR_DIAG_V1 CURRENT_STORE_V1 STARTUP_HOLD_V1\r\n"); return;}
    if(!strcmp(line,"STATUS")) {atomic_store(&last_host_ms,milliseconds()); status_json(&s); return;}
    if(!strcmp(line,"GET_CONFIG")) {config_json(&s); return;}
    if(!strcmp(line,"JOINT ALIGN")) {
        command_t cmd={.kind=CMD_JOINT_ALIGN,.reply=REPLY_TEXT}; submit(&cmd); return;
    }
    if(!strcmp(line,"MT6701 START") || !strcmp(line,"MT6701 READ") || !strcmp(line,"MT6701 STOP")) {
        command_t cmd={.kind=!strcmp(line,"MT6701 START")?CMD_JOINT_START:
            (!strcmp(line,"MT6701 READ")?CMD_JOINT_READ:CMD_JOINT_STOP),.reply=REPLY_TEXT};
        submit(&cmd); return;
    }
    if(!strncmp(line,"MT6701 START ",13)) {
        unsigned hz,address,method; char extra;
        if(sscanf(line,"MT6701 START %u %u %u %c",&hz,&address,&method,&extra)!=3 ||
           (hz!=25000 && hz!=50000 && hz!=100000) || (address!=6 && address!=70) || method>1) {
            usb_text("ERROR Use MT6701 START hz address method; hz=25000/50000/100000 address=6/70 method=0/1\r\n"); return;
        }
        command_t cmd={.kind=CMD_JOINT_START,.reply=REPLY_TEXT,.value=hz,.id=address};
        cmd.data[0]=method; submit(&cmd); return;
    }
    if(!strcmp(line,"STOP") || !strcmp(line,"DISABLE")) {
        emergency(!strcmp(line,"STOP")?FAULT_ESTOP:255); usb_text("OK Stop requested\r\n"); return;
    }
    if(!strcmp(line,"ENABLE")) {
        uint8_t frame[]={0xF3,1,0}; frame[2]=wire_crc(s.config.node,frame,2);
        native_request(s.config.node,frame,3,REPLY_TEXT); return;
    }
    char key[32],tail; double value;
    if(sscanf(line,"SET %31s %lf %c",key,&value,&tail)==2) {
        command_t cmd={.kind=CMD_SETTING,.reply=REPLY_TEXT,.value=value};
        strncpy(cmd.key,key,sizeof(cmd.key)-1); submit(&cmd); return;
    }
    unsigned id; char hex[18];
    if(sscanf(line,"CAN %u %17s %c",&id,hex,&tail)==2) {
        size_t n=strlen(hex); uint8_t bytes[8];
        if(n>=4 && n<=16 && !(n&1)) {
            for(size_t i=0;i<n/2;i++) {
                char pair[]={hex[2*i],hex[2*i+1],0},*end;
                unsigned val=strtoul(pair,&end,16); if(*end) {usb_text("ERROR Hex frame\r\n"); return;} bytes[i]=val;
            }
            if(!wire_valid(id,bytes,n/2)) {usb_text("ERROR Frame checksum\r\n"); return;}
            native_request(id,bytes,n/2,REPLY_USB); return;
        }
    }
    usb_text("ERROR Use VERSION, STATUS, GET_CONFIG, SET key value, CAN id hex, ENABLE, DISABLE, STOP\r\n");
}
static bool configure(snapshot_t *s,const settings_t *next) {
    if(s->core.enabled || !settings_valid(next)) return false;
    bool hardware=next->run_ma!=s->config.run_ma || next->hold_ma!=s->config.hold_ma;
    // Save desired settings with USB-only power. Hardware programming/readback
    // is mandatory at the next enable, before ENN can be asserted.
    if(settings_save(next)!=ESP_OK) return false;
    if(hardware) s->driver_configured=false;
    if(next->encoder_sign!=s->config.encoder_sign || next->motor_sign!=s->config.motor_sign)
        s->joint.aligned=false;
    s->config=*next; s->settings_ok=true;
    // Read-only USB/CAN queries run on other tasks. Publish the persisted
    // request before its ACK so an immediate readback cannot see the old value.
    publish(s);
    return true;
}
static bool prepare_motion(snapshot_t *s,bool force) {
    if(!force && s->core.enabled && s->driver_configured) return true;
    if(s->core.enabled || hw_driver_configure(&s->config)!=ESP_OK) {
        s->driver_configured=false; core_stop(&s->core,FAULT_DRIVER); return false;
    }
    s->driver_configured=true; return true;
}
static bool align_joint(snapshot_t *s,double now) {
    return !s->core.enabled && !s->joint_test &&
        joint_align(&s->joint,&s->joint_config,&s->config,s->core.position,now,s->core.encoder_ok);
}
static void production_joint(snapshot_t *s,double now) {
    if(s->joint_test || !s->joint_config.mode) return;
    if(s->joint.sample_at>0 && now-s->joint.sample_at<0.01) return;
    if(!s->joint_bus_ready) {
        esp_err_t ret=mt6701_begin_test(100000,s->joint_config.address,1);
        if(ret!=ESP_OK) {joint_sample(&s->joint,&s->joint_config,&s->config,0,false,ret,now,s->core.encoder_ok); return;}
        s->joint_bus_ready=true;
    }
    mt6701_i2c_sample_t sample={0}; esp_err_t ret=mt6701_read(&sample);
    joint_sample(&s->joint,&s->joint_config,&s->config,sample.raw,
        ret==ESP_OK && sample.valid,ret!=ESP_OK?ret:sample.error_code,
        esp_timer_get_time()/1e6,s->core.encoder_ok);
}
static void process(snapshot_t *s,const command_t *cmd,double now) {
    if(cmd->epoch!=atomic_load(&generation)) {response(cmd->id,cmd->data[0],0,cmd->reply); return;}
    core_t *c=&s->core; settings_t next=s->config; bool ok=false;
    // Any explicit control/configuration intent cancels the one-shot boot hold.
    // Stop/Disable can never be followed by an automatic restart in this boot.
    if(cmd->kind!=CMD_JOINT_ALIGN && !(cmd->kind==CMD_NATIVE && cmd->data[0]==0xE4))
        s->startup_pending=false;
    if(cmd->kind==CMD_JOINT_STOP) {
        mt6701_end(); s->joint_test=false; s->joint_bus_ready=false;
        usb_text("OK MT6701 stopped\r\n"); return;
    }
    if(cmd->kind==CMD_JOINT_START) {
        if(c->enabled || c->mode!=IDLE) {usb_text("ERROR Disable motor before MT6701 test\r\n"); return;}
        unsigned hz=cmd->value?cmd->value:100000,address=cmd->value?cmd->id:6;
        unsigned method=cmd->value?cmd->data[0]:1;
        if(mt6701_begin_test(hz,address,method)!=ESP_OK) {
            s->joint_test=false; usb_text("ERROR MT6701 I2C initialization\r\n"); return;
        }
        s->joint_test=true; s->joint_deadline=now+2;
        s->joint_bus_ready=false; joint_reset(&s->joint);
        char ack[100];
        snprintf(ack,sizeof(ack),"OK MT6701 started I2C SDA=17 SCL=18 HZ=%u ADDRESS=%u METHOD=%u\r\n",hz,address,method);
        usb_text(ack); return;
    }
    if(cmd->kind==CMD_JOINT_READ) {
        if(!s->joint_test || c->enabled) {usb_text("ERROR MT6701 test inactive or motor enabled\r\n"); return;}
        s->joint_deadline=now+2;
        mt6701_i2c_sample_t joint;
        if(mt6701_read(&joint)!=ESP_OK) {usb_text("ERROR MT6701 I2C session unavailable\r\n"); return;}
        char out[750];
        snprintf(out,sizeof(out),"{\"type\":\"mt6701\",\"interface\":\"i2c\",\"address\":%u,\"reg03\":%u,\"reg04\":%u,\"raw\":%u,\"transfer_ok\":%d,\"error_code\":%d,\"valid\":%d,\"motor_counts\":%.1f,\"motor_raw\":%u,\"motor_ok\":%d,\"enabled\":%d,\"sample_us\":%lld,\"hz\":%u,\"method\":%u,\"failed_register\":%u,\"sda_before\":%d,\"scl_before\":%d,\"sda_after\":%d,\"scl_after\":%d,\"motor_frame_ok\":%d,\"motor_dt_us\":%lld}\r\n",
            joint.address,joint.reg03,joint.reg04,joint.raw,joint.transfer_ok,joint.error_code,
            joint.valid,c->position,c->encoder_raw,c->encoder_ok,c->enabled,
            (long long)esp_timer_get_time(),joint.hz,joint.method,joint.failed_register,
            joint.sda_before,joint.scl_before,joint.sda_after,joint.scl_after,
            s->encoder_frame_ok,(long long)(s->encoder_dt*1e6));
        usb_text(out); return;
    }
    // Diagnostic session is serialized with CAN/USB commands. No command can
    // enable or move the motor during the test; disable and emergency stop work.
    if(!mt6701_test_allows(s->joint_test,cmd->kind==CMD_NATIVE,cmd->data[0],cmd->data[1])) {
        response(cmd->id,cmd->data[0],0,cmd->reply); return;
    }
    if(cmd->kind==CMD_JOINT_ALIGN) {
        usb_text(align_joint(s,now)?"OK Joint reference aligned\r\n":"ERROR Disable motor and obtain fresh readings from both encoders\r\n"); return;
    }
    if(cmd->kind==CMD_SETTING) {
        if(!strcmp(cmd->key,"startup_enabled")) {
            if(!c->enabled && (cmd->value==0 || cmd->value==1) && startup_save(cmd->value!=0)==ESP_OK) {
                s->startup_enabled=cmd->value!=0; s->startup_settings_ok=true; ok=true;
            }
        } else if(!strncmp(cmd->key,"joint_",6)) {
            joint_config_t joint_next=s->joint_config;
            if(!c->enabled && joint_setting(&joint_next,cmd->key,cmd->value) && joint_save(&joint_next)==ESP_OK) {
                s->joint_config=joint_next; s->joint_settings_ok=true; joint_reset(&s->joint); mt6701_end(); s->joint_bus_ready=false; ok=true;
                joint_bind(c,&s->config,&s->joint_config,&s->joint,now);
            }
        } else ok=settings_set(&next,cmd->key,cmd->value) && configure(s,&next);
        if(ok) publish(s);
        response(0,0,ok,REPLY_TEXT); return;
    }
    const uint8_t *d=cmd->data; uint8_t op=d[0];
    if((!s->settings_ok || !s->joint_settings_ok || !s->startup_settings_ok) &&
        (op==0x3D || (op==0xF3 && d[1]) || op==0x91 || op==0xF4 || op==0xF5 || op==0xF6)) {
        response(cmd->id,op,0,cmd->reply); return;
    }
    switch(op) {
        case 0xF3:
            if(d[1]==0) {core_stop(c,0); ok=true;}
            else if(d[1]==1 && c->enabled) ok=s->driver_configured && !c->fault;
            else if(d[1]==1 && (!c->joint_control || c->joint_ready) && prepare_motion(s,true)) ok=core_enable(c,&s->config);
            break;
        case 0x3D: // Explicit clear-protection while disabled; enabling is still a separate action.
            if(!c->enabled && c->encoder_ok && c->inputs_ok && (!c->joint_control || c->joint_ready) && hw_driver_health(&s->driver_status)==ESP_OK) {c->fault=0; ok=true;}
            break;
        case 0x92:
            if(!c->enabled && c->mode==IDLE && c->encoder_ok) {
                c->position=c->target=0; s->joint.aligned=false;
                if(s->joint_config.mode) align_joint(s,now);
                joint_bind(c,&s->config,&s->joint_config,&s->joint,now); ok=true;
            }
            break;
        case 0xF4: case 0xF5: {
            double target=wire_i24(d+4)+(op==0xF4?core_feedback(c):0);
            if(!c->fault && prepare_motion(s,false)) {
                ok=core_move(c,&s->config,target,((unsigned)d[1]<<8)|d[2],d[3],now);
            }
            break;
        }
        case 0xF6:
            if(!(d[1]&0x70) && !c->fault && prepare_motion(s,false)) ok=core_speed(c,&s->config,(((d[1]&15)<<8)|d[2])*(d[1]&0x80?-1:1),d[3],now);
            break;
        case 0x91: if(!c->fault && prepare_motion(s,false)) ok=core_home(c,&s->config,now); break;
        case 0xE4: ok=align_joint(s,now); break;
        case 0x90:
            // Retain the protection choice in board config. Studio cannot disable physical interlocks.
            if(d[1]==0 && d[2]<=1 && d[5]==1) {
                unsigned rpm=((unsigned)d[3]<<8)|d[4];
                if(rpm) {next.home_dir=d[2]; next.home_rpm=rpm>30?30:rpm; ok=configure(s,&next);}
            }
            break;
        case 0x83: {
            unsigned ma=((unsigned)d[1]<<8)|d[2];
            if(ma>=50 && ma<=CLMD_MAX_CURRENT_MA) {next.run_ma=ma; if(next.hold_ma>ma) next.hold_ma=ma; ok=configure(s,&next);} break;
        }
        case 0x9B:
            if(d[1]<=8) {next.hold_ma=next.run_ma*(d[1]+1)/10; ok=configure(s,&next);} break;
        case 0x8A:
            if(d[1]>=1 && d[1]<=127) {next.node=d[1]; ok=configure(s,&next);} break;
        case 0x82: ok=d[1]==4 && !c->enabled; break; // SR_CLOSE supported; no fake FOC support.
        case 0x84:
            // Internal generator is fixed at 256; acknowledge only that supported subdivision.
            ok=d[1]==0 && !c->enabled; break;
    }
    publish(s); // Publish enabled/config state before the acknowledgement.
    response(cmd->id,op,ok,cmd->reply);
}
static void control_task(void *arg) {
    snapshot_t s=snapshot(); double previous=esp_timer_get_time()/1e6,driver_at=previous;
    double input_at=0; int32_t previous_v=INT32_MIN; bool previous_enabled=false;
    double startup_at=previous+0.1;
    unsigned epoch=atomic_load(&generation);
    TickType_t wake=xTaskGetTickCount();
    for(;;) {
        double now=esp_timer_get_time()/1e6,dt=now-previous; previous=now;
        atomic_store(&control_ms,milliseconds());
        unsigned stop=atomic_exchange(&urgent,0);
        if(stop) {s.startup_pending=false; core_stop(&s.core,stop==255?0:stop); hw_velocity(0,&s.config); previous_v=0;}
        epoch=atomic_load(&generation);
        if(s.joint_test && now>=s.joint_deadline) {mt6701_end(); s.joint_test=false; s.joint_bus_ready=false;}
        uint16_t raw=0; bool enc_ok=hw_encoder(&raw,&s.encoder_word,&s.encoder_safety);
        s.encoder_sample_at=esp_timer_get_time()/1e6;
        s.encoder_frame_ok=enc_ok; s.encoder_dt=dt;
        core_encoder(&s.core,&s.config,raw,enc_ok,dt);
        production_joint(&s,now);
        joint_bind(&s.core,&s.config,&s.joint_config,&s.joint,esp_timer_get_time()/1e6);
        if(now>=input_at) {
            int minimum=0,maximum=0; bool ok=hw_inputs(&minimum,&maximum);
            core_inputs(&s.core,&s.config,minimum,maximum,ok); input_at=now+0.01;
        }
        atomic_store(&executing,true);
        command_t cmd;
        if(xQueueReceive(commands,&cmd,0)==pdTRUE) process(&s,&cmd,esp_timer_get_time()/1e6);
        if(s.startup_pending && now>=startup_at && s.settings_ok && s.joint_settings_ok &&
           s.startup_settings_ok && !s.joint_test && core_startup_ready(&s.core)) {
            // One attempt: USB-only monitoring must not be interrupted by
            // repeated 100-ms timeouts against an unpowered driver. A later
            // operator test/Enable retries after motor power is connected.
            s.startup_pending=false;
            if(prepare_motion(&s,false)) core_enable(&s.core,&s.config);
        }
        double age=(uint32_t)(milliseconds()-atomic_load(&last_host_ms))/1000.0;
        bool was_homing=s.core.mode>=HOME_RELEASE;
        double tick_now=esp_timer_get_time()/1e6;
        joint_bind(&s.core,&s.config,&s.joint_config,&s.joint,tick_now);
        core_tick(&s.core,&s.config,tick_now,dt,age);
        if(was_homing && s.core.mode==IDLE && s.core.homed && s.joint_config.mode) {
            joint_align(&s.joint,&s.joint_config,&s.config,s.core.position,tick_now,s.core.encoder_ok);
            joint_bind(&s.core,&s.config,&s.joint_config,&s.joint,tick_now);
        }
        // Encoder-only sessions never depend on motor power. Avoid blocking UART
        // polls of the unpowered TMC2209; motor enable remains interlocked.
        if(!s.joint_test && now>=driver_at && (s.core.enabled || s.core.fault!=FAULT_DRIVER)) {
            if(hw_driver_health(&s.driver_status)!=ESP_OK) {s.driver_configured=false; core_stop(&s.core,FAULT_DRIVER);}
            driver_at=now+0.2;
        }
        int32_t v=lround(s.core.velocity*s.config.motor_sign/60*s.config.fullsteps*256*16777216/12000000);
        if(v!=previous_v || s.core.enabled!=previous_enabled) {
            if(hw_velocity(s.core.enabled?s.core.velocity:0,&s.config)!=ESP_OK) {s.driver_configured=false; core_stop(&s.core,FAULT_DRIVER);}
            previous_v=v;
        }
        gate(s.core.enabled,epoch); previous_enabled=s.core.enabled;
        hw_led(s.core.enabled,s.core.mode!=IDLE,s.core.fault!=0); publish(&s);
        atomic_store(&executing,false);
        vTaskDelayUntil(&wake,pdMS_TO_TICKS(5));
    }
}
static void guard_task(void *arg) {
    for(;;) {
        if(atomic_load(&active) && (uint32_t)(milliseconds()-atomic_load(&control_ms))>50) emergency(FAULT_CONTROL);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
static void can_task(void *arg) {
    for(;;) {
        twai_status_info_t status;
        if(twai_get_status_info(&status)==ESP_OK) {
            if(status.state==TWAI_STATE_BUS_OFF) {emergency(FAULT_HOST); twai_initiate_recovery();}
            else if(status.state==TWAI_STATE_STOPPED) twai_start();
        }
        twai_message_t frame;
        if(twai_receive(&frame,pdMS_TO_TICKS(10))==ESP_OK && !frame.extd && !frame.rtr)
            native_request(frame.identifier,frame.data,frame.data_length_code,REPLY_CAN);
    }
}
static void usb_task(void *arg) {
    char line[256]; size_t size=0; bool discard=false;
    for(;;) {
        uint8_t bytes[128]; int n=usb_serial_jtag_read_bytes(bytes,sizeof(bytes),pdMS_TO_TICKS(20));
        for(int i=0;i<n;i++) {
            if(bytes[i]=='\n' || bytes[i]=='\r') {
                if(discard) usb_text("ERROR Line too long\r\n");
                else if(size) {line[size]=0; ascii_request(line);}
                size=0; discard=false;
            } else if(bytes[i]<32 || bytes[i]>126 || size>=sizeof(line)-1) discard=true;
            else if(!discard) line[size++]=bytes[i];
        }
    }
}
void app_main(void) {
    hw_pins_init();
    commands=xQueueCreate(16,sizeof(command_t)); usb_mutex=xSemaphoreCreateMutex();
    if(!commands || !usb_mutex) abort();
    snapshot_t s={0}; core_init(&s.core);
    s.settings_ok=settings_load(&s.config)==ESP_OK;
    s.joint_settings_ok=joint_load(&s.joint_config)==ESP_OK;
    s.startup_settings_ok=startup_load(&s.startup_enabled)==ESP_OK;
    s.startup_pending=s.startup_enabled && s.startup_settings_ok;
    if(!s.settings_ok || !s.joint_settings_ok || !s.startup_settings_ok) s.core.fault=FAULT_CONTROL;
    joint_bind(&s.core,&s.config,&s.joint_config,&s.joint,esp_timer_get_time()/1e6);
    s.driver_configured=tmc2209_init_uart()==ESP_OK && hw_driver_configure(&s.config)==ESP_OK;
    if(!s.driver_configured) s.core.fault=FAULT_DRIVER;
    if(hw_driver_health(&s.driver_status)!=ESP_OK) {s.driver_configured=false; s.core.fault=FAULT_DRIVER;}
    publish(&s); atomic_store(&last_host_ms,milliseconds()); atomic_store(&control_ms,milliseconds());
    usb_serial_jtag_driver_config_t usb={.rx_buffer_size=1024,.tx_buffer_size=2048};
    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb));
    twai_general_config_t general=TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN,CAN_RX_PIN,TWAI_MODE_NORMAL);
    general.tx_queue_len=32; general.rx_queue_len=32;
    twai_timing_config_t timing=TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t filter=TWAI_FILTER_CONFIG_ACCEPT_ALL();
    ESP_ERROR_CHECK(twai_driver_install(&general,&timing,&filter)); ESP_ERROR_CHECK(twai_start());
    xTaskCreatePinnedToCore(control_task,"control",8192,NULL,5,NULL,1);
    xTaskCreatePinnedToCore(guard_task,"guard",2048,NULL,7,NULL,0);
    xTaskCreatePinnedToCore(can_task,"can",4096,NULL,4,NULL,0);
    xTaskCreatePinnedToCore(usb_task,"usb",6144,NULL,3,NULL,0);
}
