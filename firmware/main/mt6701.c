#include "mt6701.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"

// PCB v2 U19: SDA=pin 3/GPIO17, SCL=pin 4/GPIO18. MD must be HIGH
// for I2C/SSI (tie MD to the module's 3.3V supply); LOW selects ABZ outputs.
// GPIO21 is unused: the user's five-pin module does not expose CSN.
#define JOINT_SDA 17
#define JOINT_SCL 18
static i2c_master_bus_handle_t bus;
static i2c_master_dev_handle_t device;
static unsigned clock_hz, fixed_address, read_method;

esp_err_t mt6701_begin(void) {
    return mt6701_begin_test(100000,0x06,1);
}
esp_err_t mt6701_begin_test(unsigned hz,unsigned address,unsigned method) {
    if((hz!=25000 && hz!=50000 && hz!=100000) ||
       (address!=0x06 && address!=0x46) || method>1) return ESP_ERR_INVALID_ARG;
    mt6701_end();
    i2c_master_bus_config_t config={.i2c_port=-1,.sda_io_num=JOINT_SDA,
        .scl_io_num=JOINT_SCL,.clk_source=I2C_CLK_SRC_DEFAULT,.glitch_ignore_cnt=7,
        .flags.enable_internal_pullup=true};
    esp_err_t ret=i2c_new_master_bus(&config,&bus);
    if(ret!=ESP_OK) return ret;
    i2c_device_config_t config_device={.dev_addr_length=I2C_ADDR_BIT_LEN_7,
        .device_address=method?I2C_DEVICE_ADDRESS_NOT_USED:address,.scl_speed_hz=hz};
    ret=i2c_master_bus_add_device(bus,&config_device,&device);
    if(ret!=ESP_OK) {mt6701_end(); return ret;}
    clock_hz=hz; fixed_address=address; read_method=method;
    return ESP_OK;
}
static esp_err_t read_register(uint8_t reg,uint8_t *value) {
    if(!read_method) return i2c_master_transmit_receive(device,&reg,1,value,1,20);
    // Explicit one-byte read avoids the convenience API's zero-length ACK-read
    // operation. Same wire transaction, independently testable driver path.
    uint8_t write_bytes[]={(uint8_t)(fixed_address<<1),reg};
    uint8_t read_address=(fixed_address<<1)|1;
    i2c_operation_job_t jobs[]={
        {.command=I2C_MASTER_CMD_START},
        {.command=I2C_MASTER_CMD_WRITE,.write={.ack_check=true,.data=write_bytes,.total_bytes=2}},
        {.command=I2C_MASTER_CMD_START},
        {.command=I2C_MASTER_CMD_WRITE,.write={.ack_check=true,.data=&read_address,.total_bytes=1}},
        {.command=I2C_MASTER_CMD_READ,.read={.ack_value=I2C_NACK_VAL,.data=value,.total_bytes=1}},
        {.command=I2C_MASTER_CMD_STOP},
    };
    return i2c_master_execute_defined_operations(device,jobs,6,20);
}
static esp_err_t read_angle(uint8_t *high,uint8_t *low,uint8_t *failed_register) {
    // Datasheet 7.7.2: separate single-byte reads, 0x03 before 0x04.
    // Transmit only the register pointer; never write sensor configuration.
    uint8_t reg=0x03;
    *failed_register=reg;
    esp_err_t ret=read_register(reg,high);
    if(ret!=ESP_OK) return ret;
    reg=0x04;
    *failed_register=reg;
    ret=read_register(reg,low);
    if(ret==ESP_OK) *failed_register=0;
    return ret;
}
esp_err_t mt6701_read(mt6701_i2c_sample_t *sample) {
    if(!bus || !sample) return ESP_ERR_INVALID_STATE;
    uint8_t high=0,low=0,failed=0;
    bool sda=gpio_get_level(JOINT_SDA),scl=gpio_get_level(JOINT_SCL);
    esp_err_t ret;
    if(!sda || !scl) {ret=ESP_ERR_INVALID_STATE; failed=0xff;}
    else ret=read_angle(&high,&low,&failed);
    *sample=(mt6701_i2c_sample_t){.address=fixed_address,.reg03=high,.reg04=low,
        .raw=mt6701_i2c_angle(high,low),.transfer_ok=ret==ESP_OK,.error_code=ret,
        .valid=ret==ESP_OK,.hz=clock_hz,.method=read_method,.failed_register=failed,
        .sda_before=sda,.scl_before=scl,.sda_after=gpio_get_level(JOINT_SDA),
        .scl_after=gpio_get_level(JOINT_SCL)};
    // Failed transfers remain visible as diagnostic samples. I2C angle reads
    // provide neither SSI CRC nor magnetic-status flags.
    return ESP_OK;
}
void mt6701_end(void) {
    if(!bus) return;
    if(device) {i2c_master_bus_rm_device(device); device=NULL;}
    i2c_del_master_bus(bus); bus=NULL;
    gpio_set_direction(JOINT_SDA,GPIO_MODE_INPUT);
    gpio_set_pull_mode(JOINT_SDA,GPIO_PULLUP_ONLY);
    gpio_set_direction(JOINT_SCL,GPIO_MODE_INPUT);
    gpio_set_pull_mode(JOINT_SCL,GPIO_PULLUP_ONLY);
}
