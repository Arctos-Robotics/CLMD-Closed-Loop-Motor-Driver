#ifndef GPIO_INIT_H
#define GPIO_INIT_H

#include "driver/gpio.h"

// TMC2209 Motor Driver
#define TMC_STEP_PIN        GPIO_NUM_5
#define TMC_DIR_PIN         GPIO_NUM_6
#define TMC_ENN_PIN         GPIO_NUM_7
#define TMC_MS1_PIN         GPIO_NUM_0
#define TMC_MS2_PIN         GPIO_NUM_1
#define TMC_UART_TX_PIN     GPIO_NUM_17  // Using available GPIO
#define TMC_UART_RX_PIN     GPIO_NUM_17  // Single-wire UART (same pin)

// TLE5012B Encoder (SPI)
#define ENCODER_SCK_PIN     GPIO_NUM_11
#define ENCODER_CS_PIN      GPIO_NUM_9
#define ENCODER_MISO_PIN    GPIO_NUM_12

// CAN Bus
#define CAN_TX_PIN          GPIO_NUM_4
#define CAN_RX_PIN          GPIO_NUM_2

// Limit Switches
#define J_MIN_PIN           GPIO_NUM_15
#define J_MAX_PIN           GPIO_NUM_16

// RGB LED
#define LED_RED_PIN         GPIO_NUM_10
#define LED_GREEN_PIN       GPIO_NUM_8
#define LED_BLUE_PIN        GPIO_NUM_13

void gpio_init_all(void);

#endif
