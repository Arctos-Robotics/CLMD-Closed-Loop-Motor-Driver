#include "joint_storage.h"
#include "nvs.h"

esp_err_t joint_load(joint_config_t *c) {
    joint_defaults(c);
    nvs_handle_t nvs; esp_err_t ret=nvs_open("clmd_studio",NVS_READONLY,&nvs);
    if(ret==ESP_ERR_NVS_NOT_FOUND) return ESP_OK;
    if(ret!=ESP_OK) return ret;
    joint_config_t stored; size_t size=sizeof(stored);
    ret=nvs_get_blob(nvs,"joint",&stored,&size); nvs_close(nvs);
    if(ret==ESP_ERR_NVS_NOT_FOUND) return ESP_OK;
    if(ret!=ESP_OK || size!=sizeof(stored) || !joint_config_valid(&stored)) return ESP_ERR_INVALID_STATE;
    *c=stored; return ESP_OK;
}
esp_err_t joint_save(const joint_config_t *c) {
    if(!joint_config_valid(c)) return ESP_ERR_INVALID_ARG;
    nvs_handle_t nvs; esp_err_t ret=nvs_open("clmd_studio",NVS_READWRITE,&nvs);
    if(ret!=ESP_OK) return ret;
    ret=nvs_set_blob(nvs,"joint",c,sizeof(*c));
    if(ret==ESP_OK) ret=nvs_commit(nvs);
    nvs_close(nvs); return ret;
}
