#pragma once
#include "core.h"
#include "esp_err.h"
esp_err_t settings_load(settings_t *c);
esp_err_t settings_save(const settings_t *c);
bool settings_set(settings_t *c,const char *key,double value);
esp_err_t startup_load(bool *enabled);
esp_err_t startup_save(bool enabled);
