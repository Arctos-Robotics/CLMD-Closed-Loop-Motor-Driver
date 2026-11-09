#pragma once
#include "esp_err.h"
typedef void *i2c_master_bus_handle_t;
typedef void *i2c_master_dev_handle_t;
#define I2C_CLK_SRC_DEFAULT 0
#define I2C_ADDR_BIT_LEN_7 0
#define I2C_DEVICE_ADDRESS_NOT_USED 0xffff
#define I2C_NACK_VAL 1
enum {I2C_MASTER_CMD_START,I2C_MASTER_CMD_WRITE,I2C_MASTER_CMD_READ,I2C_MASTER_CMD_STOP};
typedef struct {
    int command;
    union {
        struct {bool ack_check; uint8_t *data; size_t total_bytes;} write;
        struct {int ack_value; uint8_t *data; size_t total_bytes;} read;
    };
} i2c_operation_job_t;
typedef struct {
    int i2c_port, sda_io_num, scl_io_num, clk_source, glitch_ignore_cnt;
    struct {bool enable_internal_pullup;} flags;
} i2c_master_bus_config_t;
typedef struct {int dev_addr_length, device_address, scl_speed_hz;} i2c_device_config_t;
esp_err_t i2c_new_master_bus(const i2c_master_bus_config_t *,i2c_master_bus_handle_t *);
esp_err_t i2c_master_bus_add_device(i2c_master_bus_handle_t,const i2c_device_config_t *,i2c_master_dev_handle_t *);
esp_err_t i2c_master_transmit_receive(i2c_master_dev_handle_t,const uint8_t *,size_t,uint8_t *,size_t,int);
esp_err_t i2c_master_execute_defined_operations(i2c_master_dev_handle_t,i2c_operation_job_t *,size_t,int);
esp_err_t i2c_master_bus_rm_device(i2c_master_dev_handle_t);
esp_err_t i2c_del_master_bus(i2c_master_bus_handle_t);
