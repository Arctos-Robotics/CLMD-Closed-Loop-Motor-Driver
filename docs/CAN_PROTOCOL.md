# CAN Protocol Specification

This document describes the CAN bus communication protocol used by the closed-loop stepper motor driver. The protocol is based on the MKS SERVO protocol with extensions for closed-loop control.

## Overview

- **Bus Speed**: 500 kbps (configurable)
- **Topology**: Daisy-chain (multi-drop)
- **Node IDs**: 0x01 to 0x06 (6 axes maximum)
- **Message Format**: 8 bytes + CRC
- **Termination**: 120Ω on-board resistors

## Message Format

### Command Message (Host → Driver)

```
Byte 0:    CAN ID (0x01-0x06)
Byte 1:    Command Mode
Bytes 2-3: Speed (16-bit, big-endian) in RPM
Byte 4:    Acceleration (8-bit) in deg/s²
Bytes 5-7: Position (24-bit, big-endian) in degrees × 100
Byte 8:    CRC = SUM(bytes 0-7) & 0xFF
```

### Response Message (Driver → Host)

```
Byte 0:    Response ID (CAN_ID + 0x10)
Byte 1:    Status flags (bitfield)
Bytes 2-4: Current position (24-bit) in degrees × 100
Bytes 5-6: Current speed (16-bit) in RPM
Byte 7:    Error code
Byte 8:    CRC = SUM(bytes 0-7) & 0xFF
```

## Command Modes (Byte 1)

| Mode | Value | Description |
|------|-------|-------------|
| Relative Move | 0xF4 | Move by specified distance |
| Absolute Move | 0xF5 | Move to absolute position |
| Enable Motor | 0xF0 | Enable motor driver |
| Disable Motor | 0xF1 | Disable motor driver |
| Set Zero | 0xF2 | Set current position as zero |
| Query Status | 0xF3 | Request status response |
| Emergency Stop | 0xF6 | Immediate stop |
| Set Current | 0xF7 | Set motor current |
| Start Homing | 0xF8 | Begin homing sequence |
| Set PID | 0xF9 | Update PID parameters |
| Save Config | 0xFA | Save to NVS flash |

## Status Flags (Byte 1 of Response)

| Bit | Name | Description |
|-----|------|-------------|
| 0 | MOVING | 1 = motor is moving, 0 = idle |
| 1 | ERROR | 1 = error condition, 0 = ok |
| 2 | HOMED | 1 = homing complete, 0 = not homed |
| 3 | ENABLED | 1 = motor enabled, 0 = disabled |
| 4 | AT_LIMIT | 1 = limit switch triggered, 0 = ok |
| 5 | RESERVED | Reserved for future use |
| 6 | RESERVED | Reserved for future use |
| 7 | RESERVED | Reserved for future use |

## Error Codes (Byte 7 of Response)

| Code | Name | Description |
|------|------|-------------|
| 0x00 | NO_ERROR | No error |
| 0x01 | POSITION_ERROR | Position error too large |
| 0x02 | ENCODER_ERROR | Encoder communication failure |
| 0x03 | TMC_ERROR | TMC2209 driver error |
| 0x04 | LIMIT_SWITCH | Limit switch triggered |
| 0x05 | OVERCURRENT | Motor overcurrent detected |
| 0x06 | WATCHDOG | CAN watchdog timeout |
| 0x07 | INVALID_COMMAND | Unknown command received |
| 0xFF | UNKNOWN_ERROR | Unspecified error |

## Position Representation

Positions are encoded as **degrees × 100** in 24-bit signed integer format:

- **Range**: -83,886.08° to +83,886.07°
- **Resolution**: 0.01°
- **Format**: Two's complement for negative values

### Examples

| Degrees | Encoded Value (hex) | Calculation |
|---------|---------------------|-------------|
| 0.00° | 0x000000 | 0 × 100 = 0 |
| 10.00° | 0x0003E8 | 10 × 100 = 1000 |
| 45.50° | 0x0011C6 | 45.5 × 100 = 4550 |
| 90.00° | 0x002328 | 90 × 100 = 9000 |
| 180.00° | 0x004650 | 180 × 100 = 18000 |
| -10.00° | 0xFFFC18 | -10 × 100 = -1000 (two's complement) |
| -45.00° | 0xFFEE6E | -45 × 100 = -4500 (two's complement) |

## Speed Representation

Speed is encoded as **RPM** in 16-bit unsigned integer format:

- **Range**: 0 to 65,535 RPM
- **Typical**: 0 to 1000 RPM
- **Default Max**: 600 RPM

### Examples

| RPM | Encoded Value (hex) |
|-----|---------------------|
| 0 | 0x0000 |
| 100 | 0x0064 |
| 400 | 0x0190 |
| 600 | 0x0258 |
| 1000 | 0x03E8 |

## Acceleration Representation

Acceleration is encoded as **deg/s²** in 8-bit unsigned integer format:

- **Range**: 0 to 255 deg/s²
- **Typical**: 100 to 500 deg/s²
- **Default**: 500 deg/s²

## Example Messages

### Example 1: Absolute Move to 45° at 400 RPM

**Command:**
```
01 F5 01 90 2C 00 11 C6 9E
│  │  │     │  │        └─ CRC: 0x9E
│  │  │     │  └─ Position: 0x0011C6 = 4550 = 45.50°
│  │  │     └─ Acceleration: 0x2C = 44 deg/s²
│  │  └─ Speed: 0x0190 = 400 RPM
│  └─ Mode: 0xF5 = Absolute move
└─ CAN ID: 0x01 = Axis 1
```

**Response (after move complete):**
```
11 04 00 11 C6 00 00 00 2C
│  │  │        │     │  └─ CRC: 0x2C
│  │  │        │     └─ Error: 0x00 = No error
│  │  │        └─ Speed: 0x0000 = 0 RPM (stopped)
│  │  └─ Position: 0x0011C6 = 4550 = 45.50°
│  └─ Status: 0x04 = 0b00000100 (HOMED)
└─ Response ID: 0x11 = Axis 1 response
```

### Example 2: Relative Move by -10° at 200 RPM

**Command:**
```
02 F4 00 C8 64 FF FC 18 B0
│  │  │     │  │        └─ CRC: 0xB0
│  │  │     │  └─ Position: 0xFFFC18 = -1000 = -10.00°
│  │  │     └─ Acceleration: 0x64 = 100 deg/s²
│  │  └─ Speed: 0x00C8 = 200 RPM
│  └─ Mode: 0xF4 = Relative move
└─ CAN ID: 0x02 = Axis 2
```

### Example 3: Enable Motor

**Command:**
```
03 F0 00 00 00 00 00 00 F3
│  │  │                 └─ CRC: 0xF3
│  │  └─ Unused bytes (all zeros)
│  └─ Mode: 0xF0 = Enable motor
└─ CAN ID: 0x03 = Axis 3
```

**Response:**
```
13 08 00 00 00 00 00 00 1B
│  │  │                 └─ CRC: 0x1B
│  │  │                 └─ Error: 0x00 = No error
│  │  │              └─ Speed: 0x0000 = 0 RPM
│  │  └─ Position: 0x000000 = 0.00°
│  └─ Status: 0x08 = 0b00001000 (ENABLED)
└─ Response ID: 0x13 = Axis 3 response
```

### Example 4: Emergency Stop

**Command:**
```
01 F6 00 00 00 00 00 00 F7
│  │  │                 └─ CRC: 0xF7
│  │  └─ Unused bytes
│  └─ Mode: 0xF6 = Emergency stop
└─ CAN ID: 0x01 = Axis 1
```

### Example 5: Query Status

**Command:**
```
04 F3 00 00 00 00 00 00 F7
│  │  │                 └─ CRC: 0xF7
│  │  └─ Unused bytes
│  └─ Mode: 0xF3 = Query status
└─ CAN ID: 0x04 = Axis 4
```

**Response:**
```
14 09 00 2D 00 01 90 00 D5
│  │  │        │     │  └─ CRC: 0xD5
│  │  │        │     └─ Error: 0x00 = No error
│  │  │        └─ Speed: 0x0190 = 400 RPM
│  │  └─ Position: 0x002D00 = 11520 = 115.20°
│  └─ Status: 0x09 = 0b00001001 (MOVING + ENABLED)
└─ Response ID: 0x14 = Axis 4 response
```

## Multi-Axis Coordination

### Synchronized Motion

To coordinate multiple axes:

1. Send commands to all axes with same timestamp
2. Use same speed and acceleration for synchronized motion
3. Poll status to verify all axes complete simultaneously

**Example: Move 3 axes simultaneously**

```python
# Axis 1: Move to 45°
send_can_message(0x01, MODE_ABSOLUTE, 400, 500, 45.0)

# Axis 2: Move to 90°
send_can_message(0x02, MODE_ABSOLUTE, 400, 500, 90.0)

# Axis 3: Move to 30°
send_can_message(0x03, MODE_ABSOLUTE, 400, 500, 30.0)

# Wait for all axes to complete
while True:
    status1 = query_status(0x01)
    status2 = query_status(0x02)
    status3 = query_status(0x03)
    
    if not (status1.moving or status2.moving or status3.moving):
        break
    
    time.sleep(0.01)
```

## CRC Calculation

The CRC is a simple 8-bit checksum:

```c
uint8_t calculate_crc(uint8_t *data, uint8_t length) {
    uint8_t sum = 0;
    for (uint8_t i = 0; i < length; i++) {
        sum += data[i];
    }
    return sum & 0xFF;
}
```

**Example:**
```
Data: 01 F5 01 90 2C 00 11 C6
Sum:  0x01 + 0xF5 + 0x01 + 0x90 + 0x2C + 0x00 + 0x11 + 0xC6 = 0x29E
CRC:  0x29E & 0xFF = 0x9E
```

## Timing Considerations

### Command Rate
- **Maximum**: 100 Hz per axis (10ms interval)
- **Recommended**: 10-50 Hz for smooth operation
- **Minimum**: 1 Hz to avoid watchdog timeout

### Response Timing
- **Typical**: <5ms after command received
- **Maximum**: 10ms
- **Periodic**: Can be configured for automatic status updates

### Watchdog Timeout
- **Default**: 1000ms (1 second)
- **Behavior**: Motor stops if no CAN message received within timeout
- **Disable**: Set timeout to 0 (not recommended)

## Error Handling

### Invalid CRC
- Message is ignored
- No response sent
- Error logged internally

### Invalid Command
- Response sent with ERROR flag set
- Error code: 0x07 (INVALID_COMMAND)
- Motor state unchanged

### Position Error
- Motor stops immediately
- Response sent with ERROR flag set
- Error code: 0x01 (POSITION_ERROR)
- Requires manual recovery (disable/enable)

## Implementation Example (Python)

```python
import can
import struct

class ClosedLoopDriver:
    def __init__(self, can_id, bus):
        self.can_id = can_id
        self.bus = bus
    
    def calculate_crc(self, data):
        return sum(data) & 0xFF
    
    def move_absolute(self, position_deg, speed_rpm, accel_deg_s2):
        # Encode position (degrees × 100)
        pos_encoded = int(position_deg * 100)
        if pos_encoded < 0:
            pos_encoded = (1 << 24) + pos_encoded  # Two's complement
        
        # Build message
        data = [
            self.can_id,                    # Byte 0: CAN ID
            0xF5,                           # Byte 1: Absolute move
            (speed_rpm >> 8) & 0xFF,        # Byte 2: Speed high
            speed_rpm & 0xFF,               # Byte 3: Speed low
            accel_deg_s2 & 0xFF,            # Byte 4: Acceleration
            (pos_encoded >> 16) & 0xFF,     # Byte 5: Position high
            (pos_encoded >> 8) & 0xFF,      # Byte 6: Position mid
            pos_encoded & 0xFF,             # Byte 7: Position low
        ]
        
        # Add CRC
        data.append(self.calculate_crc(data))
        
        # Send CAN message
        msg = can.Message(arbitration_id=0x100 + self.can_id,
                         data=data,
                         is_extended_id=False)
        self.bus.send(msg)
    
    def query_status(self):
        # Build query message
        data = [self.can_id, 0xF3, 0, 0, 0, 0, 0, 0]
        data.append(self.calculate_crc(data))
        
        # Send query
        msg = can.Message(arbitration_id=0x100 + self.can_id,
                         data=data,
                         is_extended_id=False)
        self.bus.send(msg)
        
        # Wait for response
        response = self.bus.recv(timeout=0.1)
        if response and response.data[0] == (self.can_id + 0x10):
            return self.parse_status(response.data)
        return None
    
    def parse_status(self, data):
        # Extract status fields
        status_flags = data[1]
        position = ((data[2] << 16) | (data[3] << 8) | data[4])
        if position & 0x800000:  # Negative (two's complement)
            position = position - (1 << 24)
        position_deg = position / 100.0
        
        speed_rpm = (data[5] << 8) | data[6]
        error_code = data[7]
        
        return {
            'moving': bool(status_flags & 0x01),
            'error': bool(status_flags & 0x02),
            'homed': bool(status_flags & 0x04),
            'enabled': bool(status_flags & 0x08),
            'at_limit': bool(status_flags & 0x10),
            'position': position_deg,
            'speed': speed_rpm,
            'error_code': error_code
        }

# Usage example
bus = can.interface.Bus(channel='can0', bustype='socketcan')
driver = ClosedLoopDriver(can_id=0x01, bus=bus)

# Move to 45° at 400 RPM
driver.move_absolute(45.0, 400, 500)

# Query status
status = driver.query_status()
print(f"Position: {status['position']}°")
print(f"Moving: {status['moving']}")
```

## Troubleshooting

### No Response from Driver
- Check CAN bus termination (120Ω)
- Verify bus speed matches (500 kbps)
- Check CAN_H and CAN_L wiring
- Ensure driver is powered and initialized

### CRC Errors
- Verify CRC calculation matches specification
- Check for electrical noise on CAN bus
- Use twisted pair cable for CAN_H/CAN_L
- Add ferrite beads if needed

### Position Drift
- Check encoder magnet alignment
- Verify encoder communication
- Tune PID parameters
- Increase position error limit if needed

### Watchdog Timeouts
- Increase command rate (>1 Hz)
- Adjust watchdog timeout in settings
- Check for CAN bus congestion
- Verify host application is running

---

For more information, see:
- [Firmware Architecture](firmware_architecture.md)
- [Hardware Description](hardware_description.md)
- [Configuration Tool Guide](../software/README.md)
