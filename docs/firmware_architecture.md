# Closed Loop Stepper Motor Driver - Firmware Architecture

## Project Overview

This is a closed-loop stepper motor driver firmware for ESP32-S3 based hardware with TMC2209 motor driver and TLE5012B magnetic encoder. The system is designed for a 6-axis robotic arm where each joint has one motor/driver connected via CAN bus in a daisy-chain configuration.

## Hardware Components

- **MCU**: ESP32-S3R8 (dual-core, 8MB PSRAM)
- **Motor Driver**: TMC2209 (silent stepper, up to 2A RMS)
- **Encoder**: TLE5012BE1000 (15-bit magnetic, 360° absolute)
- **Communication**: CAN bus (TJA1050, 500 kbps), USB Type-C
- **Interfaces**: Limit switches (J_MIN/J_MAX), RGB LED, Status LED

## System Architecture

### Communication Topology
```
PC/Controller → USB-to-CAN Adapter → CAN Bus (500 kbps)
                                        ↓
                    Driver 1 (ID=0x01, X-axis) ←→ CAN Bus
                    Driver 2 (ID=0x02, Y-axis) ←→ CAN Bus
                    Driver 3 (ID=0x03, Z-axis) ←→ CAN Bus
                    Driver 4 (ID=0x04, A-axis) ←→ CAN Bus
                    Driver 5 (ID=0x05, B-axis) ←→ CAN Bus
                    Driver 6 (ID=0x06, C-axis) ←→ CAN Bus
```

### Configuration Interface
```
PC → USB Type-C → ESP32-S3 USB-CDC → Configuration Commands
                                    ↓
                        Python PyQt5 GUI Application
```

## MKS CAN Protocol Specification

### Command Message Format (8 bytes + CRC)
```
Byte 0:    CAN ID (0x01-0x06)
Byte 1:    Mode
           - 0xF4 = Relative move
           - 0xF5 = Absolute move
           - 0xF0 = Enable motor (future)
           - 0xF1 = Disable motor (future)
           - 0xF2 = Set zero position (future)
           - 0xF3 = Query status (future)
           - 0xF6 = Emergency stop (future)
           - 0xF7 = Set current (future)
           - 0xF8 = Start homing (future)
Bytes 2-3: Speed (16-bit, big-endian) in RPM
           Example: 0x0190 = 400 RPM
Byte 4:    Acceleration (8-bit) in degrees/sec²
           Example: 0x2C = 44 deg/s²
Bytes 5-7: Position (24-bit, big-endian) in degrees × 100
           Example: 0x0003E8 = 1000 = 10.00°
           Supports negative values (two's complement)
Byte 8:    CRC = SUM(bytes 0-7) & 0xFF
```

### Example Message
```
01F501902C0003E89E
│ │ │    │ │      └─ CRC: 0x9E
│ │ │    │ └─ Position: 0x0003E8 = 1000 = 10.00°
│ │ │    └─ Acceleration: 0x2C = 44 deg/s²
│ │ └─ Speed: 0x0190 = 400 RPM
│ └─ Mode: 0xF5 = Absolute move
└─ CAN ID: 0x01 = Axis 1
```

### Response Message Format (8 bytes + CRC)
```
Byte 0:    Response ID (CAN_ID + 0x10)
           Example: 0x11 for Axis 1, 0x12 for Axis 2
Byte 1:    Status flags (bitfield)
           - Bit 0: Moving (1=moving, 0=idle)
           - Bit 1: Error (1=error, 0=ok)
           - Bit 2: Homed (1=homed, 0=not homed)
           - Bit 3: Enabled (1=enabled, 0=disabled)
           - Bit 4: At limit (1=limit triggered, 0=ok)
           - Bit 5-7: Reserved
Bytes 2-4: Current position (24-bit) in degrees × 100
Bytes 5-6: Current speed (16-bit) in RPM
Byte 7:    Error code
           - 0x00: No error
           - 0x01: Position error too large
           - 0x02: Encoder communication error
           - 0x03: TMC2209 error
           - 0x04: Limit switch triggered
           - 0x05: Overcurrent
           - 0xFF: Unknown error
Byte 8:    CRC = SUM(bytes 0-7) & 0xFF
```

## Position Representation

### Coordinate Systems
1. **CAN Protocol**: Degrees × 100 (0-36000 for 0-360°, signed 24-bit)
2. **Encoder**: 15-bit counts (0-32767 per revolution)
3. **Motor Steps**: 200 steps/rev × microstepping (e.g., 3200 for 1/16)

### Conversions
```c
// CAN position to degrees
float degrees = (float)can_position / 100.0f;

// Degrees to encoder counts
int16_t encoder_counts = (int16_t)((degrees * 32768.0f) / 360.0f);

// Degrees to motor steps
int32_t steps = (int32_t)(degrees * steps_per_degree);

// Steps per degree calculation
float steps_per_degree = (200.0f * microsteps) / 360.0f;
// Example: 200 * 16 / 360 = 8.888 steps/degree
```

## USB Configuration Protocol

### Text-based Commands (ASCII over USB-CDC)

#### Configuration Commands
```
SET_ID <1-6>                    - Set CAN ID
SET_CURRENT <mA>                - Set motor current (0-2000)
SET_MICROSTEPS <1-256>          - Set microstepping
SET_STEPS_PER_DEG <float>       - Set steps per degree
SET_INVERT <0|1>                - Invert motor direction
SET_MAX_RPM <rpm>               - Set speed limit
SET_MAX_ACCEL <deg/s²>          - Set acceleration limit
SET_PID <kp> <ki> <kd>          - Set PID parameters
SET_LIMITS <min> <max>          - Set position limits (degrees)
SET_HOME_DIR <0|1>              - Set homing direction (0=J_MIN, 1=J_MAX)
SET_HOME_SPEED <rpm>            - Set homing speed
SET_NAME <string>               - Set joint name
```

#### Motion Commands
```
ENABLE                          - Enable motor
DISABLE                         - Disable motor
MOVE_ABS <deg> <rpm>            - Move to absolute position
MOVE_REL <deg> <rpm>            - Move relative
JOG <deg>                       - Quick jog move
HOME                            - Start homing sequence
STOP                            - Emergency stop
SET_ZERO                        - Set current position as zero
```

#### Query Commands
```
GET_POS                         - Get current position (degrees)
GET_STATUS                      - Get status flags
GET_CONFIG                      - Get all configuration
GET_ENCODER                     - Get raw encoder value
GET_ENDSTOPS                    - Get limit switch states
GET_TEMP                        - Get TMC2209 temperature
GET_ERROR                       - Get last error code
```

#### System Commands
```
SAVE                            - Save config to NVS flash
LOAD                            - Load config from NVS flash
RESET                           - Factory reset
LED <r> <g> <b>                 - Set RGB LED (0-255 each)
REBOOT                          - Restart ESP32
VERSION                         - Get firmware version
```

#### Response Format
```
OK <data>                       - Command successful
ERROR <message>                 - Command failed
POS <degrees>                   - Position response
STATUS <hex_flags>              - Status response
CONFIG <json>                   - Configuration data
```


## Configuration Data Structure

### Motor Configuration (Stored in NVS Flash)
```c
typedef struct {
    // Magic number for validation
    uint32_t magic;                    // 0xDEADBEEF
    uint32_t version;                  // Config version (1)
    
    // Identity
    uint8_t can_id;                    // 1-6 (default: 1)
    char name[16];                     // "Joint 1", "Joint 2", etc.
    
    // Motor Settings
    uint16_t motor_current_ma;         // 0-2000 mA (default: 1400)
    uint8_t microsteps;                // 1,2,4,8,16,32,64,128,256 (default: 16)
    float steps_per_degree;            // Calculated: (200 * microsteps) / 360
    bool invert_direction;             // false
    
    // Motion Limits
    uint16_t max_rpm;                  // 0-1000 (default: 600)
    uint16_t max_accel;                // deg/s² (default: 500)
    float min_position_deg;            // Software limit (default: -180.0)
    float max_position_deg;            // Software limit (default: +180.0)
    
    // Homing Configuration
    uint8_t home_direction;            // 0=J_MIN, 1=J_MAX (default: 0)
    uint16_t home_speed_rpm;           // Homing speed (default: 100)
    float home_offset_deg;             // Offset after homing (default: 0.0)
    bool auto_home_on_boot;            // Auto-home on startup (default: false)
    
    // PID Parameters
    float pid_kp;                      // Proportional (default: 2.0)
    float pid_ki;                      // Integral (default: 0.1)
    float pid_kd;                      // Derivative (default: 0.05)
    float pid_output_limit;            // Max PID output (default: 1000.0)
    
    // Encoder Configuration
    int32_t encoder_offset;            // Zero position offset (default: 0)
    bool encoder_invert;               // Invert encoder direction (default: false)
    
    // Safety
    float position_error_limit;        // Max allowed error (default: 5.0 degrees)
    uint16_t watchdog_timeout_ms;      // CAN watchdog (default: 1000)
    
} motor_config_t;
```

### Default Configuration Values
```c
#define DEFAULT_CAN_ID              1
#define DEFAULT_MOTOR_CURRENT       1400    // mA
#define DEFAULT_MICROSTEPS          16
#define DEFAULT_MAX_RPM             600
#define DEFAULT_MAX_ACCEL           500     // deg/s²
#define DEFAULT_MIN_POSITION        -180.0  // degrees
#define DEFAULT_MAX_POSITION        180.0   // degrees
#define DEFAULT_HOME_DIRECTION      0       // J_MIN
#define DEFAULT_HOME_SPEED          100     // RPM
#define DEFAULT_PID_KP              2.0
#define DEFAULT_PID_KI              0.1
#define DEFAULT_PID_KD              0.05
#define DEFAULT_POSITION_ERROR_LIMIT 5.0    // degrees
```

## Firmware Project Structure

```
firmware/
├── main/
│   ├── main.c                     # Entry point, FreeRTOS tasks
│   ├── can_protocol.c/h           # MKS CAN message handling
│   ├── usb_config.c/h             # USB-CDC configuration interface
│   ├── motor_control.c/h          # TMC2209 step/dir generation
│   ├── encoder.c/h                # TLE5012B SPI communication
│   ├── pid_controller.c/h         # Closed-loop PID algorithm
│   ├── motion_planner.c/h         # Trapezoidal motion profiles
│   ├── homing.c/h                 # Homing sequence logic
│   ├── config.c/h                 # NVS flash storage
│   ├── safety.c/h                 # Limits, watchdog, error handling
│   └── gpio_init.c/h              # Pin configuration
│
├── components/
│   ├── tmc2209/
│   │   ├── tmc2209.c/h            # TMC2209 UART driver
│   │   └── CMakeLists.txt
│   │
│   └── tle5012/
│       ├── tle5012.c/h            # TLE5012B SPI driver
│       └── CMakeLists.txt
│
├── CMakeLists.txt
├── sdkconfig.defaults
└── partitions.csv
```

## FreeRTOS Task Architecture

### Task Overview
```
Priority 5 (Highest):  control_loop_task      - 10 kHz closed-loop control
Priority 4:            can_rx_task            - CAN message reception
Priority 3:            motion_planner_task    - Motion profile generation
Priority 2:            usb_config_task        - USB configuration interface
Priority 1:            status_update_task     - LED updates, status reporting
Priority 0 (Lowest):   idle_task              - System idle
```

### Task Descriptions

#### 1. control_loop_task (10 kHz, Priority 5)
```c
void control_loop_task(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1);  // 1ms = 1kHz
    
    while (1) {
        // Read encoder position
        float current_position = encoder_read_angle();
        
        // Get target position from motion planner
        float target_position = motion_get_target_position();
        
        // Calculate position error
        float error = target_position - current_position;
        
        // Run PID controller
        float pid_output = pid_compute(error);
        
        // Convert to step rate and generate steps
        motor_set_step_rate(pid_output);
        
        // Check for errors
        safety_check_position_error(error);
        
        // Wait for next cycle
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
```

#### 2. can_rx_task (Priority 4)
```c
void can_rx_task(void *pvParameters) {
    twai_message_t rx_message;
    
    while (1) {
        // Wait for CAN message
        if (twai_receive(&rx_message, portMAX_DELAY) == ESP_OK) {
            // Check if message is for this driver
            if (rx_message.data[0] == config.can_id) {
                // Parse and execute command
                can_protocol_handle_message(&rx_message);
            }
        }
    }
}
```

#### 3. motion_planner_task (100 Hz, Priority 3)
```c
void motion_planner_task(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(10);  // 10ms = 100Hz
    
    while (1) {
        // Update trapezoidal motion profile
        motion_planner_update();
        
        // Check if motion complete
        if (motion_is_complete()) {
            // Send CAN status response
            can_send_status_response();
        }
        
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
```

#### 4. usb_config_task (Priority 2)
```c
void usb_config_task(void *pvParameters) {
    char rx_buffer[256];
    
    while (1) {
        // Read USB-CDC data
        int len = usb_cdc_read(rx_buffer, sizeof(rx_buffer));
        
        if (len > 0) {
            // Parse and execute configuration command
            usb_config_handle_command(rx_buffer, len);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
```

#### 5. status_update_task (10 Hz, Priority 1)
```c
void status_update_task(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100);  // 100ms = 10Hz
    
    while (1) {
        // Update RGB LED based on status
        update_status_led();
        
        // Check watchdog
        safety_check_watchdog();
        
        // Send periodic status (if configured)
        can_send_periodic_status();
        
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}
```

## Control Loop Algorithm

### Closed-Loop Control Flow
```
1. Read Encoder Position (TLE5012B via SPI)
   ↓
2. Get Target Position (from motion planner)
   ↓
3. Calculate Position Error
   error = target - current
   ↓
4. PID Controller
   P = Kp × error
   I += Ki × error × dt
   D = Kd × (error - prev_error) / dt
   output = P + I + D
   ↓
5. Convert to Step Rate
   step_rate = output × steps_per_degree
   ↓
6. Generate Step Pulses (TMC2209)
   Set timer frequency = step_rate
   Toggle STEP pin
   ↓
7. Safety Checks
   - Position error within limits?
   - Limit switches triggered?
   - Encoder communication OK?
   ↓
8. Repeat at 1-10 kHz
```

### PID Controller Implementation
```c
typedef struct {
    float kp, ki, kd;
    float integral;
    float prev_error;
    float output_limit;
    uint32_t last_time_us;
} pid_controller_t;

float pid_compute(pid_controller_t *pid, float error) {
    uint32_t now = esp_timer_get_time();
    float dt = (now - pid->last_time_us) / 1000000.0f;  // seconds
    pid->last_time_us = now;
    
    // Proportional
    float p_term = pid->kp * error;
    
    // Integral with anti-windup
    pid->integral += pid->ki * error * dt;
    if (pid->integral > pid->output_limit) pid->integral = pid->output_limit;
    if (pid->integral < -pid->output_limit) pid->integral = -pid->output_limit;
    
    // Derivative
    float d_term = pid->kd * (error - pid->prev_error) / dt;
    pid->prev_error = error;
    
    // Output
    float output = p_term + pid->integral + d_term;
    
    // Limit output
    if (output > pid->output_limit) output = pid->output_limit;
    if (output < -pid->output_limit) output = -pid->output_limit;
    
    return output;
}
```

### Motion Planner (Trapezoidal Profile)
```c
typedef struct {
    float start_position;
    float target_position;
    float current_position;
    float current_velocity;
    float max_velocity;      // From RPM
    float acceleration;      // deg/s²
    uint32_t start_time_us;
    bool is_moving;
} motion_profile_t;

void motion_planner_update(motion_profile_t *profile) {
    if (!profile->is_moving) return;
    
    uint32_t now = esp_timer_get_time();
    float elapsed = (now - profile->start_time_us) / 1000000.0f;
    
    float distance = profile->target_position - profile->start_position;
    float direction = (distance > 0) ? 1.0f : -1.0f;
    distance = fabsf(distance);
    
    // Calculate acceleration and deceleration times
    float accel_time = profile->max_velocity / profile->acceleration;
    float accel_distance = 0.5f * profile->acceleration * accel_time * accel_time;
    
    if (distance < 2 * accel_distance) {
        // Triangular profile (no constant velocity phase)
        accel_distance = distance / 2.0f;
        accel_time = sqrtf(2.0f * accel_distance / profile->acceleration);
    }
    
    float total_time = 2 * accel_time + (distance - 2 * accel_distance) / profile->max_velocity;
    
    // Calculate current position based on profile
    if (elapsed < accel_time) {
        // Acceleration phase
        profile->current_position = profile->start_position + 
            direction * 0.5f * profile->acceleration * elapsed * elapsed;
        profile->current_velocity = direction * profile->acceleration * elapsed;
    }
    else if (elapsed < total_time - accel_time) {
        // Constant velocity phase
        float const_vel_time = elapsed - accel_time;
        profile->current_position = profile->start_position + 
            direction * (accel_distance + profile->max_velocity * const_vel_time);
        profile->current_velocity = direction * profile->max_velocity;
    }
    else if (elapsed < total_time) {
        // Deceleration phase
        float decel_time = elapsed - (total_time - accel_time);
        float decel_distance = profile->max_velocity * decel_time - 
            0.5f * profile->acceleration * decel_time * decel_time;
        profile->current_position = profile->start_position + 
            direction * (distance - accel_distance + decel_distance);
        profile->current_velocity = direction * (profile->max_velocity - 
            profile->acceleration * decel_time);
    }
    else {
        // Motion complete
        profile->current_position = profile->target_position;
        profile->current_velocity = 0;
        profile->is_moving = false;
    }
}
```


## Hardware Interface Details

### GPIO Pin Assignments (from hardware_description.md)

#### TMC2209 Motor Driver
```c
#define TMC_STEP_PIN        GPIO_NUM_5      // Step pulse
#define TMC_DIR_PIN         GPIO_NUM_6      // Direction
#define TMC_ENN_PIN         GPIO_NUM_7      // Enable (active LOW)
#define TMC_MS1_PIN         GPIO_NUM_0      // Microstep select 1
#define TMC_MS2_PIN         GPIO_NUM_1      // Microstep select 2
#define TMC_UART_TX_PIN     GPIO_NUM_49     // UART TX (via 1kΩ)
#define TMC_UART_RX_PIN     GPIO_NUM_49     // UART RX (same pin, half-duplex)
```

#### TLE5012B Encoder (SPI)
```c
#define ENCODER_SCK_PIN     GPIO_NUM_11     // SPI Clock
#define ENCODER_CS_PIN      GPIO_NUM_9      // Chip Select
#define ENCODER_MISO_PIN    GPIO_NUM_12     // Data from encoder
// Note: TLE5012B uses 3-wire SPI (no MOSI)
```

#### CAN Bus
```c
#define CAN_TX_PIN          GPIO_NUM_4      // CAN transmit
#define CAN_RX_PIN          GPIO_NUM_2      // CAN receive
```

#### Limit Switches
```c
#define J_MIN_PIN           GPIO_NUM_15     // Minimum limit (active LOW)
#define J_MAX_PIN           GPIO_NUM_16     // Maximum limit (active LOW)
```

#### RGB LED
```c
#define LED_RED_PIN         GPIO_NUM_10     // Red channel
#define LED_GREEN_PIN       GPIO_NUM_8      // Green channel
#define LED_BLUE_PIN        GPIO_NUM_13     // Blue channel
```

#### USB
```c
#define USB_DP_PIN          GPIO_NUM_19     // USB D+ (internal PHY)
#define USB_DN_PIN          GPIO_NUM_20     // USB D- (internal PHY)
```

### TMC2209 Configuration

#### UART Communication
- **Baud Rate**: 115200
- **Protocol**: Single-wire half-duplex on PDN_UART pin
- **Node Address**: 0x00 (default)

#### Register Configuration
```c
// Motor current setting (IRUN/IHOLD)
// Formula: I_RMS = (CS + 1) / 32 × 1.77A (with 120mΩ sense resistors)
// For 1.4A: CS = 25
#define TMC_IRUN_DEFAULT    25      // Run current
#define TMC_IHOLD_DEFAULT   12      // Hold current (50% of run)

// Microstepping (CHOPCONF register)
#define TMC_MRES_256        0       // 1/256 microstepping
#define TMC_MRES_128        1       // 1/128
#define TMC_MRES_64         2       // 1/64
#define TMC_MRES_32         3       // 1/32
#define TMC_MRES_16         4       // 1/16 (default)
#define TMC_MRES_8          5       // 1/8
#define TMC_MRES_4          6       // 1/4
#define TMC_MRES_2          7       // 1/2
#define TMC_MRES_1          8       // Full step

// StealthChop configuration
#define TMC_TPWMTHRS        500     // Velocity threshold for StealthChop
#define TMC_TCOOLTHRS       0       // Velocity threshold for CoolStep
```

### TLE5012B Encoder Configuration

#### SPI Settings
```c
#define ENCODER_SPI_FREQ    1000000     // 1 MHz (can go up to 8 MHz)
#define ENCODER_SPI_MODE    1           // CPOL=0, CPHA=1
```

#### Register Addresses
```c
#define TLE5012_READ_ANGLE_VALUE    0x8021  // 15-bit angle
#define TLE5012_READ_SPEED_VALUE    0x8031  // Speed value
#define TLE5012_READ_STATUS         0x8001  // Status register
```

#### Reading Angle
```c
uint16_t tle5012_read_angle(void) {
    uint16_t cmd = TLE5012_READ_ANGLE_VALUE;
    uint16_t response[2];
    
    gpio_set_level(ENCODER_CS_PIN, 0);
    spi_transaction_t trans = {
        .length = 16,
        .tx_buffer = &cmd,
        .rx_buffer = response,
    };
    spi_device_transmit(spi_handle, &trans);
    gpio_set_level(ENCODER_CS_PIN, 1);
    
    // Extract 15-bit angle (0-32767)
    uint16_t angle = response[1] & 0x7FFF;
    return angle;
}

float tle5012_angle_to_degrees(uint16_t angle) {
    return (float)angle * 360.0f / 32768.0f;
}
```

### CAN Bus Configuration

#### TWAI (CAN) Settings
```c
#define CAN_BAUD_RATE       500000      // 500 kbps
#define CAN_TX_QUEUE_LEN    10
#define CAN_RX_QUEUE_LEN    10

twai_general_config_t g_config = {
    .mode = TWAI_MODE_NORMAL,
    .tx_io = CAN_TX_PIN,
    .rx_io = CAN_RX_PIN,
    .clkout_io = TWAI_IO_UNUSED,
    .bus_off_io = TWAI_IO_UNUSED,
    .tx_queue_len = CAN_TX_QUEUE_LEN,
    .rx_queue_len = CAN_RX_QUEUE_LEN,
    .alerts_enabled = TWAI_ALERT_NONE,
    .clkout_divider = 0,
};

twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
```

## LED Status Indicators

### RGB LED Behavior
```c
typedef enum {
    LED_STATUS_IDLE,        // Green: Ready, not moving
    LED_STATUS_MOVING,      // Blue: Executing motion
    LED_STATUS_HOMING,      // Yellow: Homing sequence
    LED_STATUS_ERROR,       // Red: Error condition
    LED_STATUS_CONFIG,      // Cyan: Configuration mode
    LED_STATUS_DISABLED,    // Off: Motor disabled
} led_status_t;

void set_led_status(led_status_t status) {
    switch (status) {
        case LED_STATUS_IDLE:
            gpio_set_level(LED_RED_PIN, 0);
            gpio_set_level(LED_GREEN_PIN, 1);
            gpio_set_level(LED_BLUE_PIN, 0);
            break;
        case LED_STATUS_MOVING:
            gpio_set_level(LED_RED_PIN, 0);
            gpio_set_level(LED_GREEN_PIN, 0);
            gpio_set_level(LED_BLUE_PIN, 1);
            break;
        case LED_STATUS_HOMING:
            gpio_set_level(LED_RED_PIN, 1);
            gpio_set_level(LED_GREEN_PIN, 1);
            gpio_set_level(LED_BLUE_PIN, 0);
            break;
        case LED_STATUS_ERROR:
            gpio_set_level(LED_RED_PIN, 1);
            gpio_set_level(LED_GREEN_PIN, 0);
            gpio_set_level(LED_BLUE_PIN, 0);
            break;
        case LED_STATUS_CONFIG:
            gpio_set_level(LED_RED_PIN, 0);
            gpio_set_level(LED_GREEN_PIN, 1);
            gpio_set_level(LED_BLUE_PIN, 1);
            break;
        case LED_STATUS_DISABLED:
            gpio_set_level(LED_RED_PIN, 0);
            gpio_set_level(LED_GREEN_PIN, 0);
            gpio_set_level(LED_BLUE_PIN, 0);
            break;
    }
}
```

## Homing Sequence

### Homing Algorithm
```c
typedef enum {
    HOMING_IDLE,
    HOMING_MOVE_TO_LIMIT,
    HOMING_BACKOFF,
    HOMING_SLOW_APPROACH,
    HOMING_SET_ZERO,
    HOMING_MOVE_TO_OFFSET,
    HOMING_COMPLETE,
    HOMING_ERROR
} homing_state_t;

void homing_sequence(void) {
    static homing_state_t state = HOMING_IDLE;
    
    switch (state) {
        case HOMING_IDLE:
            // Start homing
            set_led_status(LED_STATUS_HOMING);
            state = HOMING_MOVE_TO_LIMIT;
            break;
            
        case HOMING_MOVE_TO_LIMIT:
            // Move toward limit switch at homing speed
            if (config.home_direction == 0) {
                // Move toward J_MIN
                motor_move_continuous(-config.home_speed_rpm);
                if (gpio_get_level(J_MIN_PIN) == 0) {
                    motor_stop();
                    state = HOMING_BACKOFF;
                }
            } else {
                // Move toward J_MAX
                motor_move_continuous(config.home_speed_rpm);
                if (gpio_get_level(J_MAX_PIN) == 0) {
                    motor_stop();
                    state = HOMING_BACKOFF;
                }
            }
            break;
            
        case HOMING_BACKOFF:
            // Back off 5 degrees
            motor_move_relative(config.home_direction == 0 ? 5.0 : -5.0, 
                              config.home_speed_rpm / 2);
            if (motion_is_complete()) {
                state = HOMING_SLOW_APPROACH;
            }
            break;
            
        case HOMING_SLOW_APPROACH:
            // Slowly approach limit switch again
            motor_move_continuous(config.home_direction == 0 ? 
                                -config.home_speed_rpm / 4 : 
                                config.home_speed_rpm / 4);
            if ((config.home_direction == 0 && gpio_get_level(J_MIN_PIN) == 0) ||
                (config.home_direction == 1 && gpio_get_level(J_MAX_PIN) == 0)) {
                motor_stop();
                state = HOMING_SET_ZERO;
            }
            break;
            
        case HOMING_SET_ZERO:
            // Set current position as zero
            encoder_set_zero_offset();
            state = HOMING_MOVE_TO_OFFSET;
            break;
            
        case HOMING_MOVE_TO_OFFSET:
            // Move to home offset position
            motor_move_absolute(config.home_offset_deg, config.home_speed_rpm);
            if (motion_is_complete()) {
                state = HOMING_COMPLETE;
            }
            break;
            
        case HOMING_COMPLETE:
            // Homing successful
            set_led_status(LED_STATUS_IDLE);
            system_status.is_homed = true;
            state = HOMING_IDLE;
            break;
            
        case HOMING_ERROR:
            // Homing failed
            set_led_status(LED_STATUS_ERROR);
            motor_stop();
            state = HOMING_IDLE;
            break;
    }
}
```

## Safety Features

### Position Error Monitoring
```c
void safety_check_position_error(float error) {
    if (fabsf(error) > config.position_error_limit) {
        // Position error too large
        motor_emergency_stop();
        system_status.error_code = ERROR_POSITION_ERROR;
        set_led_status(LED_STATUS_ERROR);
    }
}
```

### Limit Switch Monitoring
```c
void safety_check_limit_switches(void) {
    bool j_min_triggered = (gpio_get_level(J_MIN_PIN) == 0);
    bool j_max_triggered = (gpio_get_level(J_MAX_PIN) == 0);
    
    if (j_min_triggered || j_max_triggered) {
        motor_emergency_stop();
        system_status.error_code = ERROR_LIMIT_SWITCH;
        system_status.at_limit = true;
        set_led_status(LED_STATUS_ERROR);
    }
}
```

### CAN Watchdog
```c
void safety_check_watchdog(void) {
    uint32_t now = esp_timer_get_time() / 1000;  // milliseconds
    uint32_t elapsed = now - last_can_message_time;
    
    if (elapsed > config.watchdog_timeout_ms && system_status.is_moving) {
        // No CAN message received within timeout
        motor_emergency_stop();
        system_status.error_code = ERROR_WATCHDOG_TIMEOUT;
        set_led_status(LED_STATUS_ERROR);
    }
}
```

### Encoder Communication Check
```c
void safety_check_encoder(void) {
    static uint32_t last_check_time = 0;
    uint32_t now = esp_timer_get_time() / 1000;
    
    if (now - last_check_time > 100) {  // Check every 100ms
        if (!tle5012_is_communication_ok()) {
            motor_emergency_stop();
            system_status.error_code = ERROR_ENCODER_COMM;
            set_led_status(LED_STATUS_ERROR);
        }
        last_check_time = now;
    }
}
```

## Python Configuration Tool Architecture

### PyQt5 GUI Application Structure
```
config_tool/
├── main.py                        # Application entry point
├── ui/
│   ├── main_window.py             # Main window class
│   ├── connection_tab.py          # COM port connection
│   ├── basic_settings_tab.py     # Motor configuration
│   ├── motion_test_tab.py        # Manual control
│   ├── limits_homing_tab.py      # Limits and homing
│   ├── pid_tuning_tab.py         # PID parameters
│   ├── peripherals_tab.py        # LED, encoder, diagnostics
│   └── advanced_tab.py           # Save/load/reset
├── serial_protocol.py            # USB-CDC communication
├── config_manager.py             # Configuration file handling
├── plotter.py                    # Real-time position plotting
├── requirements.txt              # Python dependencies
└── README.md
```

### Python Dependencies (requirements.txt)
```
PyQt5>=5.15.0
pyserial>=3.5
matplotlib>=3.5.0
numpy>=1.21.0
```

### Serial Protocol Handler
```python
import serial
import time

class MotorConfigProtocol:
    def __init__(self, port, baudrate=115200):
        self.ser = serial.Serial(port, baudrate, timeout=1)
        time.sleep(2)  # Wait for ESP32 to boot
        
    def send_command(self, command):
        """Send command and wait for response"""
        self.ser.write(f"{command}\n".encode())
        response = self.ser.readline().decode().strip()
        return response
        
    def set_can_id(self, can_id):
        return self.send_command(f"SET_ID {can_id}")
        
    def set_current(self, current_ma):
        return self.send_command(f"SET_CURRENT {current_ma}")
        
    def move_absolute(self, degrees, rpm):
        return self.send_command(f"MOVE_ABS {degrees} {rpm}")
        
    def get_position(self):
        response = self.send_command("GET_POS")
        if response.startswith("POS"):
            return float(response.split()[1])
        return None
        
    def enable_motor(self):
        return self.send_command("ENABLE")
        
    def disable_motor(self):
        return self.send_command("DISABLE")
        
    def save_config(self):
        return self.send_command("SAVE")
        
    def load_config(self):
        return self.send_command("LOAD")
```

### Main Window Layout
```python
from PyQt5.QtWidgets import QMainWindow, QTabWidget

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Closed Loop Motor Configuration")
        self.setGeometry(100, 100, 1000, 700)
        
        # Create tab widget
        self.tabs = QTabWidget()
        self.setCentralWidget(self.tabs)
        
        # Add tabs
        self.tabs.addTab(ConnectionTab(), "Connection")
        self.tabs.addTab(BasicSettingsTab(), "Basic Settings")
        self.tabs.addTab(MotionTestTab(), "Motion Test")
        self.tabs.addTab(LimitsHomingTab(), "Limits & Homing")
        self.tabs.addTab(PIDTuningTab(), "PID Tuning")
        self.tabs.addTab(PeripheralsTab(), "Peripherals")
        self.tabs.addTab(AdvancedTab(), "Advanced")
```

## Build and Flash Instructions

### ESP-IDF Setup
```bash
# Install ESP-IDF (if not already installed)
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32s3
. ./export.sh

# Clone firmware repository
cd ~/projects
git clone <repository_url> closed_loop_driver
cd closed_loop_driver/firmware
```

### Build Firmware
```bash
# Configure for ESP32-S3
idf.py set-target esp32s3

# Configure project (optional)
idf.py menuconfig

# Build
idf.py build
```

### Flash Firmware
```bash
# Flash to device
idf.py -p /dev/ttyUSB0 flash

# Monitor serial output
idf.py -p /dev/ttyUSB0 monitor

# Flash and monitor in one command
idf.py -p /dev/ttyUSB0 flash monitor
```

### Python Tool Setup
```bash
cd config_tool

# Create virtual environment
python3 -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install dependencies
pip install -r requirements.txt

# Run configuration tool
python main.py
```

## Testing and Calibration Procedure

### 1. Initial Power-On Test
- Connect 24V power supply
- Check RGB LED (should be green = idle)
- Connect USB cable
- Open serial monitor (115200 baud)
- Verify boot messages

### 2. Basic Configuration
- Open Python configuration tool
- Connect to COM port
- Set CAN ID (1-6)
- Set motor current (start with 1000mA)
- Set microstepping (1/16)
- Save configuration

### 3. Encoder Test
- Go to Peripherals tab
- Check encoder raw value (0-32767)
- Manually rotate motor shaft
- Verify encoder value changes smoothly

### 4. Motor Test (No Load)
- Enable motor
- Jog ±1 degree
- Verify motor moves smoothly
- Check direction (invert if needed)

### 5. PID Tuning
- Start with Kp=2.0, Ki=0.1, Kd=0.05
- Command small moves (10 degrees)
- Observe position error graph
- Adjust Kp for responsiveness
- Adjust Kd for damping
- Adjust Ki for steady-state error

### 6. Limit Switch Test
- Connect limit switches
- Go to Limits & Homing tab
- Manually trigger switches
- Verify status indicators change

### 7. Homing Test
- Set homing direction
- Set homing speed (100 RPM)
- Click Home button
- Verify homing sequence completes

### 8. CAN Bus Test
- Connect to CAN bus
- Send test messages from USB-to-CAN adapter
- Verify motor responds
- Check status responses

### 9. Full System Test
- Connect all 6 drivers to CAN bus
- Send coordinated motion commands
- Verify synchronized movement
- Check for position errors

## Troubleshooting Guide

### Motor Not Moving
- Check motor current setting (too low?)
- Verify TMC2209 enable pin (should be LOW)
- Check step/dir signals with oscilloscope
- Verify motor wiring (coil A and B)

### Position Error Too Large
- Reduce PID gains (especially Kp)
- Increase motor current
- Check for mechanical binding
- Verify encoder mounting (magnet alignment)

### Encoder Reading Unstable
- Check magnet distance (2-4mm from sensor)
- Verify SPI connections
- Check for electromagnetic interference
- Ensure magnet is diametrically polarized

### CAN Communication Issues
- Verify baud rate (500 kbps)
- Check termination resistor (120Ω)
- Verify CAN_H and CAN_L wiring
- Check for bus-off condition

### Motor Overheating
- Reduce motor current
- Improve cooling (heatsink, fan)
- Reduce hold current (IHOLD)
- Check for mechanical binding

## Future Enhancements

### Firmware
- [ ] WiFi configuration interface
- [ ] OTA firmware updates
- [ ] Advanced motion profiles (S-curve)
- [ ] Multi-axis coordinated motion
- [ ] Torque control mode
- [ ] Velocity control mode
- [ ] CANopen protocol support

### Python Tool
- [ ] Real-time position plotting
- [ ] Data logging to CSV
- [ ] Batch configuration for multiple drivers
- [ ] Firmware update via USB
- [ ] Auto-tuning wizard for PID
- [ ] Motion profile visualization

### Hardware
- [ ] Temperature sensor integration
- [ ] External I/O expansion
- [ ] Ethernet interface option
- [ ] Isolated CAN bus

## References

- ESP32-S3 Technical Reference Manual
- TMC2209 Datasheet and Application Notes
- TLE5012B Datasheet
- MKS SERVO42D User Manual
- ESP-IDF Programming Guide
- PyQt5 Documentation

---

**Document Version**: 1.0  
**Last Updated**: 2025-01-03  
**Author**: Firmware Architecture Specification
