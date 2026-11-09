#pragma once
#define GPIO_MODE_INPUT 0
#define GPIO_PULLUP_ONLY 0
int gpio_set_direction(int,int);
int gpio_set_pull_mode(int,int);
int gpio_get_level(int);
