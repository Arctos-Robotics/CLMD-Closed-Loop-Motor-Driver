#ifndef GPIO_INIT_H
#define GPIO_INIT_H

#include "driver/gpio.h"

// TMC2209 Motor Driver - VERIFIED FROM SCHEMATIC BY USER
#define TMC_STEP_PIN        GPIO_NUM_6   // Schematic verified
#define TMC_DIR_PIN         GPIO_NUM_7   // Schematic verified
#define TMC_ENN_PIN         GPIO_NUM_8   // Schematic verified
#define TMC_MS1_PIN         GPIO_NUM_1   // Schematic verified
#define TMC_MS2_PIN         GPIO_NUM_2   // Schematic verified
#define TMC_UART_TX_PIN     GPIO_NUM_39  // PCB v2: MTCK / USART_TX through R20 to PDN_UART
#define TMC_UART_RX_PIN     GPIO_NUM_40  // PCB v2: MTDO / USART_RX from PDN_UART

// TLE5012BE1000 Encoder - SSC (SPI) Interface
// E1000 in IIF mode but SSC interface available in parallel
// Hardware verified: Pin 2 (SCK) connected to GPIO12, Pin 1 (IFC) pulled high
#define ENCODER_SCK_PIN     GPIO_NUM_12  // SPI Clock (Pin 2 - SCK)
#define ENCODER_CS_PIN      GPIO_NUM_10  // SPI Chip Select (Pin 3 - CSQ)
#define ENCODER_MISO_PIN    GPIO_NUM_13  // SPI Data (Pin 4 - DATA)

// CAN Bus - VERIFIED FROM SCHEMATIC BY USER
#define CAN_TX_PIN          GPIO_NUM_5   // Schematic verified (CAN_TXD)
#define CAN_RX_PIN          GPIO_NUM_4   // Schematic verified (CAN_RXD)

// Limit Switches - VERIFIED FROM SCHEMATIC BY USER
#define J_MIN_PIN           GPIO_NUM_15  // Schematic verified
#define J_MAX_PIN           GPIO_NUM_16  // Schematic verified

// RGB LED - VERIFIED FROM SCHEMATIC BY USER
#define LED_RED_PIN         GPIO_NUM_11  // Schematic verified
#define LED_GREEN_PIN       GPIO_NUM_9   // Schematic verified
#define LED_BLUE_PIN        GPIO_NUM_14  // Schematic verified

// Boot Button - VERIFIED FROM SCHEMATIC BY USER
#define BOOT_PIN            GPIO_NUM_0   // Schematic verified

// Reset - VERIFIED FROM SCHEMATIC BY USER
// RESET is on CHIP_PU (hardware reset pin)

void gpio_init_all(void);

#endif
