#ifndef USB_CONFIG_H
#define USB_CONFIG_H

#include "esp_err.h"

esp_err_t usb_config_init(void);
void usb_config_task(void *pvParameters);

#endif
