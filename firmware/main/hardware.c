#include "hardware.h"
#include "gpio_init.h"
#include "tmc2209_advanced.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_rom_sys.h"
#include "freertos/FreeRTOS.h"
#include <math.h>

static adc_oneshot_unit_handle_t adc;
static portMUX_TYPE encoder_lock=portMUX_INITIALIZER_UNLOCKED;
void hw_pins_init(void) {
    gpio_config_t out={.pin_bit_mask=(1ULL<<TMC_ENN_PIN)|(1ULL<<TMC_DIR_PIN)|(1ULL<<TMC_STEP_PIN)|
        (1ULL<<TMC_MS1_PIN)|(1ULL<<TMC_MS2_PIN)|(1ULL<<ENCODER_CS_PIN)|(1ULL<<ENCODER_SCK_PIN)|
        (1ULL<<LED_RED_PIN)|(1ULL<<LED_GREEN_PIN)|(1ULL<<LED_BLUE_PIN),.mode=GPIO_MODE_OUTPUT};
    gpio_set_level(TMC_ENN_PIN,1); gpio_config(&out); gpio_set_level(TMC_ENN_PIN,1);
    gpio_set_level(ENCODER_CS_PIN,1); gpio_set_level(ENCODER_SCK_PIN,0);
    gpio_set_level(TMC_STEP_PIN,0); gpio_set_level(TMC_DIR_PIN,0);
    gpio_set_level(TMC_MS1_PIN,0); gpio_set_level(TMC_MS2_PIN,0);
    gpio_config_t in={.pin_bit_mask=(1ULL<<J_MIN_PIN)|(1ULL<<J_MAX_PIN)|(1ULL<<ENCODER_MISO_PIN),
        .mode=GPIO_MODE_INPUT,.pull_up_en=GPIO_PULLUP_ENABLE}; gpio_config(&in);
    adc_oneshot_unit_init_cfg_t unit={.unit_id=ADC_UNIT_2};
    if(adc_oneshot_new_unit(&unit,&adc)!=ESP_OK) adc=NULL;
    hw_led(false,false,false);
}
esp_err_t hw_driver_health(uint32_t *status) {
    esp_err_t ret=tmc2209_read_register(TMC_DRV_STATUS,status);
    return ret==ESP_OK && !(*status&0x3f)?ESP_OK:ESP_FAIL;
}
esp_err_t hw_driver_configure(const settings_t *c) {
    uint32_t before,after,gconf,chop,status;
    if(tmc2209_read_register(0x02,&before)!=ESP_OK || tmc2209_set_velocity(0)!=ESP_OK ||
       tmc2209_set_microstepping(0)!=ESP_OK || tmc2209_write_register(TMC_GCONF,0xC4)!=ESP_OK ||
       tmc2209_set_current(c->run_ma,c->hold_ma)!=ESP_OK || tmc2209_read_register(0x02,&after)!=ESP_OK ||
       ((after-before)&255)!=4 || tmc2209_read_register(TMC_GCONF,&gconf)!=ESP_OK || gconf!=0xC4 ||
       tmc2209_read_register(TMC_CHOPCONF,&chop)!=ESP_OK || chop!=0x10000053 ||
       hw_driver_health(&status)!=ESP_OK) return ESP_FAIL;
    return ESP_OK;
}
esp_err_t hw_velocity(double rpm,const settings_t *c) {
    int32_t v=lround(rpm*c->motor_sign/60*c->fullsteps*256*16777216/12000000);
    return tmc2209_set_velocity(v);
}
static uint16_t read_word(void) {
    uint16_t result=0;
    for(int i=0;i<16;i++) {
        gpio_set_level(ENCODER_SCK_PIN,0); esp_rom_delay_us(1);
        gpio_set_level(ENCODER_SCK_PIN,1);
        result=(result<<1)|gpio_get_level(ENCODER_MISO_PIN); esp_rom_delay_us(1);
    }
    return result;
}
bool hw_encoder(uint16_t *raw,uint16_t *word,uint16_t *safety) {
    const uint16_t command=0x8021;
    portENTER_CRITICAL(&encoder_lock);
    gpio_set_level(ENCODER_CS_PIN,0); esp_rom_delay_us(1);
    gpio_set_direction(ENCODER_MISO_PIN,GPIO_MODE_OUTPUT);
    for(int bit=15;bit>=0;bit--) {
        gpio_set_level(ENCODER_SCK_PIN,0); gpio_set_level(ENCODER_MISO_PIN,(command>>bit)&1);
        esp_rom_delay_us(1); gpio_set_level(ENCODER_SCK_PIN,1); esp_rom_delay_us(1);
    }
    gpio_set_level(ENCODER_SCK_PIN,0); gpio_set_direction(ENCODER_MISO_PIN,GPIO_MODE_INPUT);
    esp_rom_delay_us(2); *word=read_word(); *safety=read_word();
    gpio_set_level(ENCODER_SCK_PIN,0); gpio_set_level(ENCODER_CS_PIN,1);
    portEXIT_CRITICAL(&encoder_lock);
    uint8_t bytes[]={command>>8,command&255,*word>>8,*word&255};
    *raw=*word&0x7fff;
    return (*safety&0x7000)==0x7000 && sensor_crc(bytes,4)==(*safety&255);
}
static bool adc_pin(int pin,adc_channel_t channel,int *value) {
    adc_oneshot_chan_cfg_t cfg={.atten=ADC_ATTEN_DB_12,.bitwidth=ADC_BITWIDTH_12};
    bool ok=adc_oneshot_config_channel(adc,channel,&cfg)==ESP_OK;
    gpio_pullup_en(pin); esp_rom_delay_us(200);
    int sum=0;
    for(int i=0;ok&&i<16;i++) {int v=0; ok=adc_oneshot_read(adc,channel,&v)==ESP_OK; sum+=v;}
    gpio_config_t in={.pin_bit_mask=1ULL<<pin,.mode=GPIO_MODE_INPUT,.pull_up_en=GPIO_PULLUP_ENABLE};
    ok=(gpio_config(&in)==ESP_OK)&&ok; if(ok) *value=(sum+8)/16; return ok;
}
bool hw_inputs(int *minimum,int *maximum) {
    return adc && adc_pin(J_MIN_PIN,ADC_CHANNEL_4,minimum) && adc_pin(J_MAX_PIN,ADC_CHANNEL_5,maximum);
}
void hw_led(bool enabled,bool moving,bool fault) {
    // PCB v2 LED has a common anode: LOW lights a channel.
    gpio_set_level(LED_RED_PIN,!fault); gpio_set_level(LED_GREEN_PIN,!(enabled&&!fault&&!moving));
    gpio_set_level(LED_BLUE_PIN,!(moving&&!fault));
}
