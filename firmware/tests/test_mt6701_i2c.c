#include "mt6701.h"
#include "driver/i2c_master.h"
#include <assert.h>
#include <stdio.h>

static unsigned connected=6, expected_register=3, reads, buses, removes, configured_address;
static int fault, fail_low, stuck_pin;
esp_err_t i2c_new_master_bus(const i2c_master_bus_config_t *c,i2c_master_bus_handle_t *b) {
    assert(c->sda_io_num==17 && c->scl_io_num==18 && c->i2c_port==-1);
    assert(c->flags.enable_internal_pullup); *b=(void *)1; ++buses; return ESP_OK;
}
esp_err_t i2c_master_bus_add_device(i2c_master_bus_handle_t b,const i2c_device_config_t *c,i2c_master_dev_handle_t *d) {
    assert(b && (c->scl_speed_hz==25000 || c->scl_speed_hz==50000 || c->scl_speed_hz==100000));
    configured_address=c->device_address;
    assert(configured_address==6 || configured_address==70 || configured_address==0xffff);
    *d=(void *)1; return ESP_OK;
}
static esp_err_t receive(unsigned address,uint8_t reg,uint8_t *value) {
    ++reads;
    if(address!=connected || fault) return fault?fault:ESP_ERR_INVALID_STATE;
    assert(reg==expected_register);
    if(reg==3) {*value=0x80; expected_register=4;}
    else {*value=0x07; expected_register=3; if(fail_low) return ESP_FAIL;}
    return ESP_OK;
}
esp_err_t i2c_master_transmit_receive(i2c_master_dev_handle_t d,const uint8_t *w,size_t wn,uint8_t *r,size_t rn,int timeout) {
    assert(d && wn==1 && rn==1 && timeout==20 && configured_address!=0xffff);
    return receive(configured_address,*w,r);
}
esp_err_t i2c_master_execute_defined_operations(i2c_master_dev_handle_t d,i2c_operation_job_t *j,size_t n,int timeout) {
    assert(d && n==6 && timeout==20 && configured_address==0xffff);
    assert(j[0].command==I2C_MASTER_CMD_START && j[2].command==I2C_MASTER_CMD_START);
    assert(j[1].command==I2C_MASTER_CMD_WRITE && j[1].write.ack_check && j[1].write.total_bytes==2);
    assert(j[3].command==I2C_MASTER_CMD_WRITE && j[3].write.ack_check && j[3].write.total_bytes==1);
    unsigned address=j[1].write.data[0]>>1;
    assert(j[1].write.data[0]==address*2 && j[3].write.data[0]==address*2+1);
    assert(j[4].command==I2C_MASTER_CMD_READ && j[4].read.total_bytes==1 && j[4].read.ack_value==I2C_NACK_VAL);
    assert(j[5].command==I2C_MASTER_CMD_STOP);
    return receive(address,j[1].write.data[1],j[4].read.data);
}
esp_err_t i2c_master_bus_rm_device(i2c_master_dev_handle_t d) {assert(d); ++removes; return ESP_OK;}
esp_err_t i2c_del_master_bus(i2c_master_bus_handle_t b) {assert(b); --buses; return ESP_OK;}
int gpio_set_direction(int pin,int mode) {assert(pin==17 || pin==18); return 0;}
int gpio_set_pull_mode(int pin,int mode) {assert(pin==17 || pin==18); return 0;}
int gpio_get_level(int pin) {assert(pin==17 || pin==18); return pin!=stuck_pin;}
int main(void) {
    mt6701_i2c_sample_t s;
    assert(mt6701_read(&s)==ESP_ERR_INVALID_STATE);
    unsigned speeds[]={25000,50000,100000};
    for(unsigned address=6;address<=70;address+=64) for(unsigned method=0;method<2;++method)
        for(unsigned speed=0;speed<3;++speed) {
            connected=address; reads=0; expected_register=3;
            assert(mt6701_begin_test(speeds[speed],address,method)==ESP_OK && buses==1);
            assert(mt6701_read(&s)==ESP_OK && s.valid && s.address==address && s.raw==8193 && reads==2);
            assert(s.hz==speeds[speed] && s.method==method && !s.failed_register);
            assert(s.sda_before && s.scl_before && s.sda_after && s.scl_after);
            for(stuck_pin=17;stuck_pin<=18;stuck_pin++) {
                unsigned before=reads;
                assert(mt6701_read(&s)==ESP_OK && !s.valid && s.failed_register==255);
                assert(s.error_code==ESP_ERR_INVALID_STATE && reads==before);
            }
            stuck_pin=0;
            connected=address^64;
            assert(mt6701_read(&s)==ESP_OK && !s.valid && s.address==address && reads==3 && s.failed_register==3);
            connected=address; fail_low=1;
            assert(mt6701_read(&s)==ESP_OK && !s.valid && s.error_code==ESP_FAIL && s.failed_register==4);
            fail_low=0;
            assert(mt6701_read(&s)==ESP_OK && s.valid);
        }
    assert(mt6701_begin_test(12345,6,0)==ESP_ERR_INVALID_ARG);
    assert(mt6701_begin_test(100000,7,0)==ESP_ERR_INVALID_ARG);
    assert(mt6701_begin_test(100000,6,2)==ESP_ERR_INVALID_ARG);
    mt6701_end(); mt6701_end(); assert(buses==0 && removes==12);
    assert(mt6701_read(&s)==ESP_ERR_INVALID_STATE);
    puts("MT6701 I2C DRIVER PASS: 12 profiles, exact START/ACK/read/NACK/STOP jobs, fixed address, failures, recovery, pin levels and cleanup");
}
