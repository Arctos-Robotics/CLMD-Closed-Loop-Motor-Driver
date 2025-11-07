# Quick Start Guide

Get your closed-loop stepper motor driver up and running in 15 minutes!

## Prerequisites

- Assembled PCB with all components
- 12-24V power supply
- Stepper motor (NEMA 17 or NEMA 23)
- TLE5012B magnetic encoder with magnet
- USB-C cable
- Python 3.7+ installed (for configuration tool)

## Step 1: Hardware Setup (5 minutes)

### Power Connection
1. Connect 12-24V power supply to U2 connector (2-pin, 2.5mm pitch)
   - Pin 1: +VIN (12-24V)
   - Pin 2: GND

### Motor Connection
1. Connect stepper motor to U16 connector (4-pin, 2.54mm pitch)
   - Pin 1: 2B (Motor phase B2)
   - Pin 2: 2A (Motor phase A2)
   - Pin 3: 1A (Motor phase A1)
   - Pin 4: 1B (Motor phase B1)

### Encoder Setup
1. Mount TLE5012B encoder on motor shaft
2. Attach magnet to shaft end (2-4mm distance from encoder)
3. Ensure magnet is centered and perpendicular to encoder

### Optional: Limit Switches
1. Connect J_MIN to CN1 (3-pin connector)
   - Pin 1: +3.3V
   - Pin 2: J_MIN signal
   - Pin 3: GND
2. Connect J_MAX to CN2 (3-pin connector)

## Step 2: Flash Firmware (5 minutes)

### Option A: Using Pre-built Binary (Easiest)
1. Connect USB-C cable to PC
2. Download firmware from `release/firmware/`
3. Use ESP Flash Download Tool or esptool:
   ```bash
   esptool.py -p COM3 -b 460800 write_flash 0x0 bootloader.bin 0x8000 partition-table.bin 0x10000 firmware.bin
   ```

### Option B: Using Configuration Tool (Recommended)
1. Install Python dependencies:
   ```bash
   cd software
   pip install -r requirements.txt
   ```
2. Run configuration tool:
   ```bash
   python main.py
   ```
3. Go to "Firmware Upload" tab
4. Select COM port
5. Click "Upload Firmware"
6. Wait for completion (~30 seconds)

### Option C: Build from Source
See [HOW_TO_BUILD_FIRMWARE.md](HOW_TO_BUILD_FIRMWARE.md) for detailed instructions.

## Step 3: First Connection (3 minutes)

1. Launch configuration tool:
   ```bash
   cd software
   python main.py
   ```

2. In the Dashboard panel:
   - Select your COM port from dropdown
   - Click "Connect"
   - You should see "Connected" status

3. Verify communication:
   - Check command log for "Connected to device"
   - Firmware version should be displayed
   - Position should update (even if motor is disabled)

## Step 4: Basic Testing (2 minutes)

### Enable Motor
1. Go to "Control" tab
2. Click "Enable Motor" (green button)
3. RGB LED should turn green
4. Motor should hold position (slight resistance when turned by hand)

### Test Movement
1. Click jog buttons to test:
   - "+1°" for small movement
   - "+10°" for medium movement
   - "+45°" for large movement
2. Watch position update in real-time on dashboard
3. Observe smooth motion with acceleration/deceleration

### Test Encoder
1. Go to "Peripherals" tab
2. Check "Encoder Diagnostics" section
3. Manually rotate motor shaft
4. Raw encoder value should change (0-32767)
5. Position on dashboard should update

## Troubleshooting

### Motor doesn't move
- ✅ Check motor is enabled (green LED)
- ✅ Verify motor wiring (correct phase connections)
- ✅ Check power supply voltage (12-24V)
- ✅ Ensure motor current is set (default: 1400mA)

### Position not updating
- ✅ Check encoder magnet alignment (2-4mm distance)
- ✅ Verify encoder is powered (3.3V)
- ✅ Test encoder communication in Peripherals tab
- ✅ Check SPI connections (SCK, CS, MISO)

### Connection fails
- ✅ Close other programs using COM port (Arduino IDE, PuTTY)
- ✅ Try different USB cable or port
- ✅ Check USB drivers are installed
- ✅ Press reset button on board and retry

### Motor vibrates or makes noise
- ✅ Reduce motor current (try 1000mA)
- ✅ Check microstepping setting (default: 1/16)
- ✅ Verify encoder magnet is centered
- ✅ Tune PID parameters (see PID Tuning Guide)

## Next Steps

### Configure Your System
1. Go to "Settings" tab
2. Set CAN ID (1-6) for multi-axis systems
3. Adjust motor current for your motor
4. Set position limits if needed
5. Click "Save to Flash" to persist settings

### Tune PID Controller
1. Go to "PID Tuning" tab
2. Start with defaults (Kp=2.0, Ki=0.1, Kd=0.05)
3. Run step response test (45° step)
4. Analyze overshoot and settling time
5. Adjust parameters for optimal performance
6. See [PID Tuning Guide](PID_TUNING.md) for details

### Test Advanced Features
1. **RGB LED Control**: Set custom colors in Peripherals tab
2. **Limit Switches**: Test homing sequence
3. **CAN Bus**: Connect multiple drivers for multi-axis control
4. **TMC Diagnostics**: Monitor temperature and enable CoolStep

## Configuration Examples

### High-Speed Application (CNC, 3D Printer)
```
Motor Current: 1800mA
Microstepping: 1/16
Max RPM: 800
Max Accel: 800 deg/s²
PID: Kp=3.0, Ki=0.2, Kd=0.08
```

### High-Precision Application (Robotic Arm)
```
Motor Current: 1400mA
Microstepping: 1/32
Max RPM: 400
Max Accel: 300 deg/s²
PID: Kp=2.5, Ki=0.15, Kd=0.1
```

### Low-Power Application (Battery Operated)
```
Motor Current: 1000mA
Microstepping: 1/8
Max RPM: 300
Max Accel: 200 deg/s²
Enable CoolStep in TMC Diagnostics
```

## Safety Tips

⚠️ **Important Safety Information**

- Always connect power supply before USB
- Never exceed 28V input voltage
- Ensure motor is properly secured before testing
- Use appropriate motor current for your motor size
- Keep fingers away from moving parts
- Disconnect power when making wiring changes

## Support

Need help? Check these resources:
- [Hardware Description](hardware_description.md) - Complete hardware specs
- [Firmware Architecture](firmware_architecture.md) - Firmware details
- [Configuration Tool Guide](../software/README.md) - GUI usage
- [Troubleshooting Guide](TROUBLESHOOTING.md) - Common issues
- [FAQ](FAQ.md) - Frequently asked questions

---

**Congratulations!** Your closed-loop stepper motor driver is now operational. 🎉

For advanced features and multi-axis coordination, see the [End User Guide](END_USER_GUIDE.md).
