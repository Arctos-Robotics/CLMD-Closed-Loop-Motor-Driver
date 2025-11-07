# Closed Loop Stepper Motor Driver Firmware

ESP32-S3 firmware for closed-loop stepper motor control with TMC2209 and TLE5012B encoder.

## Quick Start

### Prerequisites
- ESP-IDF v5.0 or later
- ESP32-S3 development board

### Build and Flash

```bash
# Set up ESP-IDF environment
. $HOME/esp/esp-idf/export.sh

# Configure for ESP32-S3
idf.py set-target esp32s3

# Build
idf.py build

# Flash and monitor
idf.py -p /dev/ttyUSB0 flash monitor
```

## USB Configuration Commands

Connect via USB CDC (115200 baud) and send text commands:

```
ENABLE              - Enable motor
DISABLE             - Disable motor
GET_POS             - Get current position
MOVE_ABS 45.0 400   - Move to 45° at 400 RPM
MOVE_REL 10.0 200   - Move 10° relative at 200 RPM
STOP                - Emergency stop
SET_ZERO            - Set current position as zero
SET_ID 1            - Set CAN ID (1-6)
SAVE                - Save configuration to flash
VERSION             - Get firmware version
GET_STATUS          - Get status flags
```

## CAN Protocol

See `firmware_architecture.md` for complete CAN protocol specification.

## LED Status

- **Green**: Idle, ready
- **Blue**: Moving
- **Red**: Error condition
- **Off**: Motor disabled

## Default Configuration

- CAN ID: 1
- Motor current: 1400 mA
- Microstepping: 1/16
- Max speed: 600 RPM
- PID: Kp=2.0, Ki=0.1, Kd=0.05

## Project Structure

```
firmware/
├── main/
│   ├── main.c              - Application entry point
│   ├── gpio_init.c/h       - Pin configuration
│   ├── encoder.c/h         - TLE5012B driver
│   ├── motor_control.c/h   - TMC2209 step generation
│   ├── pid_controller.c/h  - PID algorithm
│   ├── motion_planner.c/h  - Trapezoidal profiles
│   ├── can_protocol.c/h    - CAN communication
│   ├── usb_config.c/h      - USB configuration
│   ├── config.c/h          - NVS storage
│   └── safety.c/h          - Safety monitoring
├── CMakeLists.txt
├── sdkconfig.defaults
└── partitions.csv
```

## Troubleshooting

### Motor not moving
- Check motor current setting
- Verify motor enable (ENN pin LOW)
- Check encoder communication

### Position error
- Reduce PID gains
- Check encoder magnet alignment (2-4mm distance)
- Verify mechanical coupling

### CAN communication issues
- Check baud rate (500 kbps)
- Verify termination resistor (120Ω)
- Check CAN_H and CAN_L wiring
