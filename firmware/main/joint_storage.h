#pragma once
#include "joint.h"
#include "esp_err.h"
esp_err_t joint_load(joint_config_t *c);
esp_err_t joint_save(const joint_config_t *c);
