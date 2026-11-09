#include "settings.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>
#include <math.h>
esp_err_t startup_load(bool *enabled) {
    *enabled=true; // New and existing boards default to powered, stationary hold.
    nvs_handle_t nvs; esp_err_t ret=nvs_open("clmd_studio",NVS_READONLY,&nvs);
    if(ret==ESP_ERR_NVS_NOT_FOUND) return ESP_OK;
    if(ret!=ESP_OK) return ret;
    uint8_t value=1; ret=nvs_get_u8(nvs,"startup_enable",&value); nvs_close(nvs);
    if(ret==ESP_ERR_NVS_NOT_FOUND) return ESP_OK;
    if(ret!=ESP_OK || value>1) return ESP_ERR_INVALID_STATE;
    *enabled=value!=0; return ESP_OK;
}
esp_err_t startup_save(bool enabled) {
    nvs_handle_t nvs; esp_err_t ret=nvs_open("clmd_studio",NVS_READWRITE,&nvs);
    if(ret!=ESP_OK) return ret;
    ret=nvs_set_u8(nvs,"startup_enable",enabled);
    if(ret==ESP_OK) ret=nvs_commit(nvs);
    nvs_close(nvs); return ret;
}
esp_err_t settings_load(settings_t *c) {
    settings_defaults(c);
    esp_err_t ret=nvs_flash_init();
    // Preserve existing NVS. Do not silently erase board configuration.
    if(ret!=ESP_OK) return ret;
    nvs_handle_t nvs; ret=nvs_open("clmd_studio",NVS_READONLY,&nvs);
    if(ret==ESP_ERR_NVS_NOT_FOUND) return ESP_OK;
    if(ret!=ESP_OK) return ret;
    settings_t loaded; size_t size=sizeof(loaded);
    ret=nvs_get_blob(nvs,"settings",&loaded,&size); nvs_close(nvs);
    if(ret==ESP_ERR_NVS_NOT_FOUND) return ESP_OK;
    if(ret!=ESP_OK || size!=sizeof(loaded) || !settings_valid(&loaded)) return ESP_ERR_INVALID_STATE;
    *c=loaded; return ESP_OK;
}
esp_err_t settings_save(const settings_t *c) {
    if(!settings_valid(c)) return ESP_ERR_INVALID_ARG;
    nvs_handle_t nvs; esp_err_t ret=nvs_open("clmd_studio",NVS_READWRITE,&nvs);
    if(ret!=ESP_OK) return ret;
    ret=nvs_set_blob(nvs,"settings",c,sizeof(*c)); if(ret==ESP_OK) ret=nvs_commit(nvs);
    nvs_close(nvs); return ret;
}
bool settings_set(settings_t *c,const char *key,double value) {
    if(!isfinite(value)) return false;
    settings_t next=*c;
    // Bounds are checked as doubles before narrowing, preventing integer wrap.
    if(!strcmp(key,"min_counts") || !strcmp(key,"max_counts") || !strcmp(key,"home_travel_turns")) {
        if(!strcmp(key,"min_counts")) next.min_counts=value;
        else if(!strcmp(key,"max_counts")) next.max_counts=value;
        else next.home_travel_turns=value;
    } else {
        if(value!=floor(value) || value< -1 || value>10000) return false;
        if((!strcmp(key,"motor_sign") || !strcmp(key,"encoder_sign")) && value!=1 && value!=-1) return false;
        if((!strcmp(key,"min_enabled") || !strcmp(key,"max_enabled") || !strcmp(key,"home_dir") || !strcmp(key,"require_home")) && (value<0 || value>1)) return false;
        if(!strcmp(key,"node")) next.node=value;
        else if(!strcmp(key,"run_ma")) next.run_ma=value;
        else if(!strcmp(key,"hold_ma")) next.hold_ma=value;
        else if(!strcmp(key,"fullsteps")) next.fullsteps=value;
        else if(!strcmp(key,"max_rpm")) next.max_rpm=value;
        else if(!strcmp(key,"accel_rpm_s")) next.accel_rpm_s=value;
        else if(!strcmp(key,"watchdog_ms")) next.watchdog_ms=value;
        else if(!strcmp(key,"home_rpm")) next.home_rpm=value;
        else if(!strcmp(key,"home_timeout_s")) next.home_timeout_s=value;
        else if(!strcmp(key,"motor_sign")) next.motor_sign=value;
        else if(!strcmp(key,"encoder_sign")) next.encoder_sign=value;
        else if(!strcmp(key,"min_enabled")) next.min_enabled=value;
        else if(!strcmp(key,"max_enabled")) next.max_enabled=value;
        else if(!strcmp(key,"home_dir")) next.home_dir=value;
        else if(!strcmp(key,"require_home")) next.require_home=value;
        else if(!strcmp(key,"min_threshold")) next.min_threshold=value;
        else if(!strcmp(key,"max_threshold")) next.max_threshold=value;
        else if(!strcmp(key,"hysteresis")) next.hysteresis=value;
        else return false;
    }
    if(!settings_valid(&next)) return false;
    *c=next;
    return true;
}
