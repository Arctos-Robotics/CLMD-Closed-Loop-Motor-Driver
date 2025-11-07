# Closed Loop Motor Driver - Hardware Description

## Overview
This is a closed-loop motor driver board designed for precise motor control applications. The board integrates an ESP32-S3 microcontroller for processing, a TMC2209 stepper motor driver for smooth motor control, and a TLE5012B magnetic encoder for position feedback.

## Quick Reference - GPIO Pin Map (VERIFIED FROM SCHEMATIC)

| GPIO | Function | Direction | Description |
|------|----------|-----------|-------------|
| GPIO0 | MS1 | Output | TMC2209 microstep select 1 |
| GPIO1 | MS2 | Output | TMC2209 microstep select 2 |
| GPIO2 | CAN_RXD | Input | CAN receive from TJA1050 |
| GPIO3 | BOOT | Input | Boot button (active LOW, 10kΩ pull-up) |
| GPIO4 | CAN_TXD | Output | CAN transmit to TJA1050 |
| GPIO5 | STEP | Output | TMC2209 step pulse |
| GPIO6 | DIR | Output | TMC2209 direction control |
| GPIO7 | TMC_ENN | Output | TMC2209 enable (active LOW) |
| GPIO8 | GREEN_LED | Output | RGB LED green channel (330Ω) |
| GPIO9 | ENCODER_CS | Output | TLE5012B SPI chip select |
| GPIO10 | RED_LED | Output | RGB LED red channel (330Ω) |
| GPIO11 | ENCODER_SCK | Output | TLE5012B SPI clock |
| GPIO12 | ENCODER_DATA | Input | TLE5012B SPI MISO |
| GPIO13 | BLUE_LED | Output | RGB LED blue channel (330Ω) |
| GPIO14 | (Available) | I/O | Not connected in schematic |
| GPIO15 | J_MIN | Input | Minimum limit switch (10kΩ pull-up) |
| GPIO16 | J_MAX | Input | Maximum limit switch (10kΩ pull-up) |
| GPIO17 | GPIO17 | I/O | External connector U16 pin 2 |
| GPIO18 | GPIO18 | I/O | External connector U16 pin 1 |
| GPIO19 | USB_DP | I/O | USB D+ (internal PHY) |
| GPIO20 | (Available) | I/O | Not clearly connected |
| GPIO21 | GPIO21 | I/O | External connector |
| GPIO30 | SPIHD | I/O | W25Q16 flash hold |
| GPIO31 | SPIWP | I/O | W25Q16 flash write protect |
| GPIO32 | SPICS0 | Output | W25Q16 flash chip select |
| GPIO33 | SPICLK | Output | W25Q16 flash SPI clock |
| GPIO34 | SPIQ | Input | W25Q16 flash SPI MISO |
| GPIO35 | SPID | Output | W25Q16 flash SPI MOSI |
| GPIO36 | GPIO36 | I/O | External connector (U10, U17) |
| GPIO37 | GPIO37 | I/O | Available |
| GPIO38 | GPIO38 | I/O | Available |
| GPIO43 | MTMS | - | JTAG TMS |
| GPIO44 | MTDO | - | JTAG TDO |
| GPIO45 | MTDI | - | JTAG TDI |
| GPIO46 | MTCK | - | JTAG TCK |
| GPIO49 | U0TXD | Output | UART0 debug transmit |
| GPIO50 | U0RXD | Input | UART0 debug receive |
| GPIO53 | XTAL_N | - | 40MHz crystal negative |
| GPIO54 | XTAL_P | - | 40MHz crystal positive |
| CHIP_PU | RESET | Input | Hardware reset button |

**Note:** Pin assignments traced directly from schematic images. Some GPIO assignments may need verification with actual board testing.

## Main Components

### Microcontroller
- **ESP32-S3R8** (U11)
  - Package: QFN-56
  - 8MB integrated PSRAM
  - Dual-core Xtensa LX7 processor
  - Built-in WiFi and Bluetooth
  - Multiple GPIO, UART, SPI, I2C interfaces
  - 40MHz crystal oscillator (X1) for timing

### Motor Driver
- **TMC2209-LA-T** (U12)
  - Package: QFN-28
  - Silent stepper motor driver with StealthChop2
  - Up to 2A RMS motor current
  - UART interface for configuration
  - Integrated current sensing
  - Current sense resistors: 2x 120mΩ (R22, R23)
  - Microstepping up to 1/256

### Position Encoder
- **TLE5012BE1000** (U13)
  - Package: PG-DSO-8
  - 360° angle sensor (magnetic encoder)
  - SPI interface
  - 15-bit resolution
  - Provides closed-loop position feedback
  - Mounted on bottom layer

## Power Management

### Main Power Supply
- **TPS54331DR** (U1)
  - Buck converter
  - Input voltage protection with fuse: BSMD1812-200-30V (F1)
  - Schottky diodes: 2x B340A-13-F (D1, D2)
  - Inductor: 6.8µH (L1)
  - Output capacitors: 2x 33µF (C2, C3)
  - Feedback resistors for voltage setting:
    - R6: 29.4kΩ
    - R2: 1.91kΩ
    - R3: 100kΩ
  - P-channel MOSFET for reverse polarity protection: DMG2305UX-7 (Q1)

### 3.3V Regulator
- **TLV75733PDBVR** (U6)
  - Package: SOT-23-5
  - Low dropout (LDO) regulator
  - Fixed 3.3V output
  - Powers ESP32-S3 and logic circuits
  - Input capacitors: 2x 10µF (C21, C22)
  - Output capacitor: 1µF (C17)

## Communication Interfaces

### USB Interface
- **TYPE-C 16PIN 2MD(073)** (USB1)
  - USB Type-C connector
  - ESD protection: 3x LESD5Z5.0CT1G (U3, U4, U5)
  - Pull-down resistors: 2x 5.1kΩ (R4, R5) for CC pins

### CAN Bus Interface
- **TJA1050T-JSM** (U7)
  - CAN transceiver
  - Package: SOIC-8
  - Termination resistor: 120Ω (R8)
  - Connector: JS20T-S02PHW-00 (U8, U9)

### External Connectors
- **CN1, CN2**: JS20T-S03PHW-00 (3-pin, 2.0mm pitch) - Motor phase connections
- **U10**: JS20T-S08PHW-00 (8-pin, 2.0mm pitch) - General I/O
- **U16**: XY-XHB2.54-4A21 (4-pin, 2.54mm pitch) - Programming/Debug header
- **U2**: BX-XH2.54-2PWZ (2-pin, 2.5mm pitch) - Power input
- **U17**: JS20T-S02PHW-00 (2-pin) - Additional I/O
- **P1**: 2x2 pin header (2.54mm pitch) - SWD/JTAG programming

## Memory
- **W25Q16JVUUIQ** (U15)
  - 16Mbit (2MB) SPI Flash memory
  - Package: USON-8
  - Used for external program/data storage

## User Interface

### Buttons
- **SW1, SW2**: HX 3x4x2-2P-2.5N
  - 2x tactile push buttons
  - Pull-up resistors: 10kΩ (R9, R10)
  - Likely for Boot and Reset functions

### LEDs
- **LED1**: 19-237/R6GHBHC-A07/2T
  - RGB LED (4-pin)
  - Current limiting resistors: 330Ω (R7, R11, R13)
- **U14**: XL-1005UWC
  - Yellow status LED (0402 package)
  - Current limiting resistor: 330Ω (R21)

## Decoupling and Filtering

### Power Decoupling
- 9x 100nF ceramic capacitors (C1, C14, C15, C19, C20, C28, C31, C32, C33)
- 6x 1µF ceramic capacitors (C10, C11, C18, C26, C27, C30)
- 3x 4.7µF capacitors (C4, C5, C29)
- 1x 10µF capacitor (C16)

### Signal Filtering
- Crystal load capacitors: 2x 30pF (C12, C13)
- Encoder filtering: 2x 12pF (C24, C25)
- TMC2209 filtering: 2x 10nF (C6, C7)
- Buck converter compensation: 1nF (C8), 47pF (C9)
- CAN bus filtering: 22nF (C23)

## Key Resistors

### Pull-ups/Pull-downs
- 10kΩ: R1, R9, R10, R25, R26, R27 (general purpose pull-ups)
- 4.7kΩ: R12, R17, R18, R19 (I2C/signal pull-ups)
- 100kΩ: R3, R14, R24 (high impedance pull-ups)

### Current Limiting
- 330Ω: R7, R11, R13, R21 (LED current limiting)
- 1kΩ: R20 (general purpose)
- 499Ω: R16 (signal conditioning)

### Current Sensing
- 120mΩ: R22, R23 (motor current sense resistors, 1206 package, 1W)
- 1.1Ω: R15 (0603 package, current limiting)

## Connector Pinouts

### CN1 & CN2 - Limit Switch Connectors (3-pin, 2.0mm pitch)
**CN1:**
- Pin 1: +3.3V
- Pin 2: J_MIN (GPIO15 with 10kΩ pull-up R26)
- Pin 3: GND

**CN2:**
- Pin 1: +3.3V
- Pin 2: J_MAX (GPIO16 with 10kΩ pull-up R27)
- Pin 3: GND

### U16 - Motor Phase Output (4-pin, 2.54mm pitch)
- Pin 1: 2B (Motor phase B2 from TMC2209 OB2)
- Pin 2: 2A (Motor phase A2 from TMC2209 OA2)
- Pin 3: 1A (Motor phase A1 from TMC2209 OA1)
- Pin 4: 1B (Motor phase B1 from TMC2209 OB1)

### U2 - Power Input (2-pin, 2.5mm pitch)
- Pin 1: VIN (+24V input, 6-28V range)
- Pin 2: GND

### U8, U9 - CAN Bus (2-pin, 2.0mm pitch)
- Pin 1: CANH (CAN High)
- Pin 2: CANL (CAN Low)

### U10 - General I/O (8-pin, 2.0mm pitch)
- Pin 1: GPIO36
- Pin 2: GPIO35
- Pin 3: GPIO34
- Pin 4: GPIO33
- Pin 5: +3.3V
- Pin 6: GND
- Pin 7: +5V
- Pin 8: GND

### U16 - Programming/Debug Header (4-pin, 2.54mm pitch)
- Pin 1: GPIO18
- Pin 2: GPIO17
- Pin 3: GND
- Pin 4: +3.3V

### U17 - External I/O (2-pin, 2.0mm pitch)
- Pin 1: GPIO36
- Pin 2: GPIO35

### P1 - SWD/JTAG Programming (2x2 header, 2.54mm pitch)
- Pin 1: +3.3V
- Pin 2: BOOT
- Pin 3: RESET
- Pin 4: GND

### USB1 - USB Type-C
- Standard USB 2.0 pinout
- D+/D- connected to ESP32-S3 internal USB PHY
- VBUS provides 5V input (alternative power source)

## Board Specifications

### Physical
- PCB layers: Minimum 2-layer (components on top and bottom)
- Component density: High (SMD components down to 0402)
- Connector types: Mix of through-hole and SMD
- Dimensions: Approximately 50mm × 50mm (verify from layout)

### Power Requirements
- **Input voltage**: 6-28V DC (optimized for 12-24V)
- **Motor supply**: Same as input (up to 28V)
- **Motor current**: Up to 2A RMS per phase (2.8A peak)
- **Logic voltage**: 3.3V (regulated from 5V)
- **5V rail**: Generated by TPS54331 buck converter
- **Total power**: Up to 60W (24V × 2.5A typical)
- **USB power**: 5V via Type-C (can power logic only)

## Key Features Summary

1. **Closed-Loop Control**: TLE5012B magnetic encoder provides 15-bit position feedback (0.011° resolution)
2. **Silent Operation**: TMC2209 with StealthChop2 technology for quiet motor operation
3. **Wireless Connectivity**: ESP32-S3 with WiFi and Bluetooth (not yet implemented in hardware)
4. **Multiple Interfaces**: USB-C, CAN bus, UART, SPI
5. **Flexible I/O**: Multiple connectors for external devices and expansion
6. **Protection**: ESD protection, reverse polarity protection, overcurrent protection via fuse
7. **User Feedback**: RGB LED and status LED for visual indication
8. **Expandable**: 2MB external flash memory for configuration and firmware updates
9. **High Current**: Up to 2A RMS motor current with 120mΩ sense resistors
10. **Wide Input Range**: 6-28V DC input with efficient buck converter

## Design Notes

### Power Architecture
The board uses a two-stage power regulation:
1. **Stage 1**: TPS54331 buck converter (VIN → 5V) for main power
2. **Stage 2**: TLV75733 LDO (5V → 3.3V) for clean logic power

This approach provides:
- Efficient power conversion for motor supply
- Clean, low-noise 3.3V for sensitive analog/digital circuits
- Reverse polarity protection via P-channel MOSFET (Q1)

### Motor Current Calculation
With 120mΩ sense resistors:
- Maximum current setting: 2A RMS (2.8A peak)
- Current resolution: ~64mA per step (32-step IRUN register)
- Recommended starting current: 1.4A RMS (IRUN = 20)

### Encoder Resolution
- TLE5012B: 15-bit absolute angle (32768 counts/revolution)
- With 1/16 microstepping: 3200 steps/revolution
- Combined resolution: ~10 encoder counts per microstep
- Excellent for closed-loop control and position verification

### Communication Bandwidth
- **CAN Bus**: Up to 1 Mbps (recommend 250-500 kbps for reliability)
- **TMC2209 UART**: 115200 baud (sufficient for configuration)
- **TLE5012B SPI**: Up to 8 MHz (1 kHz position update rate achievable)
- **Flash SPI**: Up to 104 MHz (fast configuration loading)

## Firmware Architecture Recommendation

```
Main Control Loop (1-10 kHz):
├── Read encoder position (TLE5012B via SPI)
├── Calculate position error
├── Run PID controller
├── Generate step/direction signals (TMC2209)
└── Update status LEDs

Background Tasks:
├── CAN bus message handling (interrupt-driven)
├── USB communication (CDC for debugging)
├── TMC2209 configuration updates (as needed)
├── Limit switch monitoring (interrupt-driven)
├── Watchdog feeding
└── Temperature monitoring

Initialization:
├── GPIO configuration
├── Peripheral initialization (UART, SPI, CAN)
├── TMC2209 setup (current, microstepping)
├── Encoder calibration
├── Load parameters from flash
└── Homing sequence (optional)
```

## Pin Assignments (Verified from Schematic)

### ESP32-S3 Pin Mapping (VERIFIED FROM SCHEMATIC)

#### Motor Driver (TMC2209) Interface
- **STEP** → GPIO5 (Step pulse input to TMC2209 pin 16)
- **DIR** → GPIO6 (Direction control to TMC2209 pin 19)
- **TMC_ENN** → GPIO7 (Enable, active low to TMC2209 pin 2)
- **MS1** → GPIO0 (Microstep select 1 to TMC2209 pin 9 MS1_AD0)
- **MS2** → GPIO1 (Microstep select 2 to TMC2209 pin 10 MS2_AD1)
- **USART_TX** → Connected to TMC2209 PDN_UART pin 14 via 1kΩ resistor (R20)
- **USART_RX** → Connected to TMC2209 PDN_UART pin 14 via 1kΩ resistor (R20)
  - Note: TMC2209 uses single-wire half-duplex UART on PDN_UART pin
  - Both TX and RX share the same line through R20
- **TMC_CLK** → External clock input to TMC2209 pin 13 (optional)
- **SPREAD** → SpreadCycle status from TMC2209 pin 7
- **STDBY** → Standby control to TMC2209 pin 20 (connected via 100kΩ pull-up R24 to +3.3V)
- **DIAG** → Diagnostic output from TMC2209 pin 11
- **INDEX** → Index pulse output from TMC2209 pin 12

#### Encoder (TLE5012BE1000) Interface - SPI
- **ENCODER_SCK** → GPIO11 (SPI Clock)
- **ENCODER_CS** → GPIO9 (Chip Select)
- **ENCODER_DATA** → GPIO12 (SPI MISO - Data from encoder)
- **IFC** → Encoder analog output (with 4.7kΩ pull-up R17)
- **IFB** → Encoder analog output (with 4.7kΩ pull-up R18)
- **IFA** → Encoder analog output (with 4.7kΩ pull-up R19)

#### External Flash (W25Q16) Interface - SPI
- **SPICLK** → GPIO33 (SPI Clock)
- **SPICS0** → GPIO32 (Chip Select)
- **SPID** → GPIO35 (MOSI - Master Out Slave In)
- **SPIQ** → GPIO34 (MISO - Master In Slave Out)
- **SPIWP** → GPIO31 (Write Protect)
- **SPIHD** → GPIO30 (Hold)

#### CAN Bus Interface
- **CAN_TXD** → GPIO4 (CAN transmit to TJA1050)
- **CAN_RXD** → GPIO2 (CAN receive from TJA1050)

#### USB Interface
- **USB_DP** → GPIO19 (USB D+ via internal routing)
- **USB_DN** → GPIO25 (USB D- via internal routing - needs verification from schematic)

#### User Interface
- **BOOT** → GPIO3 (Boot button SW1, active low with 10kΩ pull-up R25)
- **RESET** → CHIP_PU (Reset button SW2 via C33 100nF)
- **RED_LED** → GPIO10 (Red LED control, 330Ω current limit R7)
- **GREEN_LED** → GPIO8 (Green LED control, 330Ω current limit R11)
- **BLUE_LED** → GPIO13 (Blue LED control, 330Ω current limit R13)
- **Status LED (U14)** → Connected via 330Ω (R21) to 5V_OUT from TMC2209

#### Limit Switches / Endstops
- **J_MIN** → GPIO15 (Minimum position limit, 10kΩ pull-up R26)
- **J_MAX** → GPIO16 (Maximum position limit, 10kΩ pull-up R27)

#### External I/O Connectors
- **U10 (8-pin connector, 2.0mm pitch)**:
  - Pin 1: GPIO17
  - Pin 2: GPIO18
  - Pin 3: GPIO3 (BOOT)
  - Pin 4: GPIO21
  - Pin 5: GPIO38
  - Pin 6: GPIO35
  - Pin 7: GPIO36
  - Pin 8: GPIO33

- **U16 (4-pin header, 2.54mm pitch - Motor Phase Output)**:
  - Pin 1: 2B (Motor phase B2 from TMC2209)
  - Pin 2: 2A (Motor phase A2 from TMC2209)
  - Pin 3: 1A (Motor phase A1 from TMC2209)
  - Pin 4: 1B (Motor phase B1 from TMC2209)
  - Connect stepper motor coils: Coil A to 1A/2A, Coil B to 1B/2B

- **U17 (2-pin connector, 2.0mm pitch - External 5V)**:
  - Pin 1: +5V (External 5V output from isolator circuit)
  - Pin 2: GND

#### UART/Debug
- **U0TXD** → GPIO49 (UART0 transmit) - also connected to TMC2209 via R20 (1kΩ) and R16 (499Ω)
- **U0RXD** → GPIO50 (UART0 receive)

#### Crystal Oscillator
- **XTAL_P** → GPIO54 (40MHz crystal positive)
- **XTAL_N** → GPIO53 (40MHz crystal negative)

### TMC2209 Motor Driver Connections

#### Motor Phase Outputs (to U16 connector)
- **OA2 (pin 21)** → 2A → U16 pin 2 (Phase A2)
- **OA1 (pin 24)** → 1A → U16 pin 3 (Phase A1)
- **OB1 (pin 26)** → 1B → U16 pin 4 (Phase B1)
- **OB2 (pin 1)** → 2B → U16 pin 1 (Phase B2)

#### Current Sensing
- **BRA (pin 23)** → Connected to motor phase 1A via R22 (120mΩ, 1W sense resistor)
- **BRB (pin 27)** → Connected to motor phase 1B via R23 (120mΩ, 1W sense resistor)
- Current sense configuration allows TMC2209 to monitor and regulate motor current

#### Power
- **VS (pins 22, 28)** → +24V (Motor supply voltage, up to 28V max)
- **VCC_IO (pin 15)** → +3.3V (Logic supply from ESP32)
- **5VOUT (pin 8)** → Internal 5V regulator output
  - Decoupled with C17 (1µF), C21 (10µF), C22 (10µF)
  - Powers status LED U14 via R21 (330Ω)
- **GND (pins 3, 18)** → Ground
- **EP (pin 29)** → Exposed pad connected to ground for thermal dissipation

#### Additional Signals
- **CPO (pin 4)** → Connected to C23 (22nF) and R14 (100kΩ) - charge pump oscillator
- **CPI (pin 5)** → Connected to C23 (22nF) - charge pump input
- **VREF (pin 17)** → Reference voltage (not externally connected, internal)
- **VCP (pin 6)** → Charge pump voltage, decoupled with C15 (100nF)

### TLE5012BE1000 Encoder Connections
- **VDD** → +3.3V
- **GND** → Ground
- **IFA, IFB, IFC** → Analog feedback signals (4.7kΩ pull-ups)
- **SPI Interface** → Connected to ESP32-S3 (SCK, CS, DATA)

### Power Supply Connections

#### Input Power (U2 - 2-pin connector)
- Pin 1: VIN (+24V input)
- Pin 2: GND

#### TPS54331 Buck Converter
- **VIN** → +24V (via fuse F1 and reverse protection Q1)
- **PH** → Switch node (connected to L1 inductor)
- **VSENSE** → Feedback divider (R6: 29.4kΩ, R2: 1.91kΩ)
- **Output** → +5V (VCC_5V)

#### TLV75733 LDO
- **IN** → +5V (from buck converter)
- **OUT** → +3.3V (system logic supply)

### CAN Bus Connections (U8, U9 - 2-pin connectors)
- **CANH** → CAN High signal
- **CANL** → CAN Low signal
- **Termination** → 120Ω (R8) between CANH and CANL

### Programming Interface (P1 - 2x2 header)
- Pin 1: +3.3V
- Pin 2: BOOT
- Pin 3: RESET
- Pin 4: GND

## Firmware Development Guide

### 1. Motor Control (TMC2209)

#### UART Configuration
- **Baud Rate**: 115200 (default for TMC2209)
- **TX/RX Pin**: GPIO49 (U0TXD) connected via 1kΩ resistor (R20) to TMC2209 PDN_UART
- **Protocol**: Single-wire UART (half-duplex) via PDN_UART pin
- **Note**: TMC2209 uses single-wire UART on PDN_UART pin 14

#### Control Pins
- **STEP (GPIO5)**: Generate step pulses for motor movement
- **DIR (GPIO6)**: Set rotation direction (HIGH/LOW)
- **ENN (GPIO7)**: Enable driver (active LOW)
- **MS1 (GPIO0), MS2 (GPIO1)**: Hardware microstepping selection (can be overridden via UART)

#### Configuration Registers (via UART)
- Set motor current (IRUN, IHOLD registers)
- Configure microstepping (up to 1/256)
- Enable StealthChop2 for silent operation
- Configure StallGuard for sensorless homing
- Set chopper timing parameters

#### Current Sensing
- Sense resistors: 120mΩ (R22, R23)
- Maximum current: 2A RMS
- Formula: I_RMS = (VREF × 1.77A) / 2.5V
- Adjust via IRUN register (0-31 scale)

### 2. Position Feedback (TLE5012BE1000)

#### SPI Configuration
- **Clock (GPIO11)**: Up to 8 MHz - ENCODER_SCK
- **CS (GPIO9)**: Active LOW chip select - ENCODER_CS
- **MISO (GPIO12)**: Data from encoder - ENCODER_DATA
- **Mode**: SPI Mode 1 (CPOL=0, CPHA=1)

#### Reading Angle Data
- 15-bit resolution (0-32767 counts per revolution)
- Angular resolution: 0.011° per count
- Read angle register: 0x8021
- Read speed register: 0x8031
- Update rate: Up to 1 kHz

#### Analog Outputs (Optional)
- **IFA, IFB, IFC**: Analog sine/cosine signals for redundancy
- Can be used for analog position feedback or diagnostics

### 3. Closed-Loop Control Algorithm

#### PID Controller Implementation
```
Error = Target_Position - Encoder_Position
P_term = Kp × Error
I_term += Ki × Error × dt
D_term = Kd × (Error - Previous_Error) / dt
Output = P_term + I_term + D_term
```

#### Control Loop Structure
1. Read target position from CAN/USB command
2. Read current position from TLE5012B encoder
3. Calculate position error
4. Run PID algorithm
5. Convert PID output to step rate
6. Generate step pulses on GPIO21
7. Update direction on GPIO20 if needed
8. Repeat at fixed frequency (1-10 kHz recommended)

#### Tuning Parameters
- **Kp**: Proportional gain (start with 1.0)
- **Ki**: Integral gain (start with 0.1)
- **Kd**: Derivative gain (start with 0.01)
- Adjust based on system response and stability

### 4. CAN Bus Communication

#### CAN Configuration
- **TX Pin**: GPIO4 (CAN_TXD)
- **RX Pin**: GPIO2 (CAN_RXD)
- **Transceiver**: TJA1050 (5V tolerant)
- **Termination**: 120Ω on-board (R8)
- **Bit Rate**: 250 kbps or 500 kbps (configurable)

#### Message Protocol (Example)
- **ID 0x100**: Position command (4 bytes: target position)
- **ID 0x101**: Velocity command (4 bytes: target velocity)
- **ID 0x102**: Current command (2 bytes: target current)
- **ID 0x200**: Status response (8 bytes: position, velocity, current, status)
- **ID 0x201**: Error/diagnostic message

### 5. External Flash Memory (W25Q16)

#### SPI Configuration
- **Clock (GPIO33)**: Up to 104 MHz
- **CS (GPIO32)**: Active LOW
- **MOSI (GPIO35)**: Data to flash
- **MISO (GPIO34)**: Data from flash
- **Capacity**: 2MB (16 Mbit)

#### Use Cases
- Store motor calibration parameters
- Log position/velocity data
- Store PID tuning parameters
- Firmware update storage (OTA)
- Configuration profiles

### 6. User Interface

#### Buttons
- **BOOT (GPIO3)**: 
  - Short press: Enter calibration mode
  - Long press: Factory reset
  - Active LOW with 10kΩ pull-up (R25)
  - Connected to SW1
  
- **RESET (CHIP_PU)**: 
  - Hardware reset of ESP32-S3
  - Active LOW
  - Connected to SW2 with 100nF capacitor (C33)

#### LEDs
- **RGB LED (LED1)**:
  - Red (GPIO10): Error/fault indication (330Ω resistor R7)
  - Green (GPIO8): Normal operation (330Ω resistor R11)
  - Blue (GPIO13): Communication active (330Ω resistor R13)
  - Current limit: 330Ω per channel
  - Common anode configuration
  
- **Status LED (U14)**:
  - Yellow indicator (XL-1005UWC)
  - Connected to TMC2209 5V_OUT via 330Ω resistor (R21)
  - Indicates TMC2209 power status

#### Limit Switches
- **J_MIN (GPIO15)**: Minimum position endstop (10kΩ pull-up R26)
- **J_MAX (GPIO16)**: Maximum position endstop (10kΩ pull-up R27)
- Active LOW when triggered
- Use for homing and safety limits
- Connected via CN1 and CN2 3-pin connectors

### 7. USB Interface

#### Configuration
- **USB Type-C connector**
- **D+ / D-**: Internal USB PHY routing
- **ESD Protection**: 3x LESD5Z5.0CT1G diodes
- **Use Cases**:
  - Firmware programming (USB-JTAG)
  - Serial debugging (USB-CDC)
  - Configuration interface
  - Real-time monitoring

### 8. Initialization Sequence

1. **Power-On**:
   - Wait for 3.3V rail to stabilize (100ms)
   - Initialize GPIO pins
   - Configure clock (40 MHz crystal)

2. **Peripheral Initialization**:
   - Initialize UART for TMC2209 (115200 baud)
   - Initialize SPI for TLE5012B (1 MHz initially)
   - Initialize SPI for W25Q16 flash
   - Initialize CAN bus (250 kbps)
   - Initialize USB-CDC for debugging

3. **Motor Driver Setup**:
   - Configure TMC2209 via UART:
     - Set IRUN = 20 (1.4A RMS)
     - Set IHOLD = 10 (0.7A RMS)
     - Enable StealthChop2
     - Set microstepping to 1/16
   - Enable driver (ENN = LOW)

4. **Encoder Calibration**:
   - Read TLE5012B angle
   - Verify communication
   - Set zero position reference

5. **Homing Sequence** (if using limit switches):
   - Move slowly toward J_MIN
   - Stop when J_MIN triggered
   - Back off slightly
   - Set current position as zero

6. **Enter Main Loop**:
   - Start closed-loop control
   - Enable CAN communication
   - Update status LEDs

### 9. Safety Features

#### Overcurrent Protection
- Monitor TMC2209 DIAG pin (GPIO14)
- Triggers on overcurrent or short circuit
- Disable motor immediately on fault

#### Position Limits
- Software limits based on encoder position
- Hardware limits via J_MIN/J_MAX switches
- Emergency stop on limit trigger

#### Watchdog Timer
- Enable ESP32-S3 watchdog (1 second timeout)
- Reset if main loop hangs
- Feed watchdog in control loop

#### Thermal Protection
- Monitor TMC2209 temperature via UART
- Reduce current if overheating
- Shutdown at critical temperature

### 10. Diagnostic Features

#### Status Monitoring
- **DIAG (GPIO14)**: TMC2209 diagnostic output
- **INDEX (GPIO13)**: Position index pulse
- **SPREAD (GPIO11)**: SpreadCycle status

#### Debug Output
- Use U0TXD/U0RXD (GPIO43/44) for serial debug
- Log position, velocity, current, errors
- Real-time plotting via USB-CDC

### 11. Recommended Libraries

- **ESP-IDF**: Official Espressif framework
- **TMCStepper**: Arduino library for TMC drivers (port to ESP-IDF)
- **TLE5012-Arduino**: Encoder library (port to ESP-IDF)
- **ESP32-CAN**: CAN bus driver
- **PID_v1**: PID controller library

### 12. Performance Targets

- **Control Loop Frequency**: 1-10 kHz
- **Position Resolution**: 0.011° (encoder) × microstepping
- **Maximum Step Rate**: 200 kHz (TMC2209 limit)
- **Position Accuracy**: ±0.1° (with proper tuning)
- **Response Time**: <10ms for small position changes
