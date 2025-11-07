# Closed Loop Motor Configuration Tool

Modern PyQt5 GUI application for configuring and testing the closed-loop stepper motor driver with visual board representation.

## Quick Start

### Windows
```bash
run.bat
```

### Linux/Mac
```bash
chmod +x run.sh
./run.sh
```

### Manual Installation
```bash
# Create virtual environment
python3 -m venv venv

# Activate
source venv/bin/activate  # Linux/Mac
venv\Scripts\activate     # Windows

# Install dependencies
pip install -r requirements.txt

# Run
python main.py
```

## Features

### 🎯 Visual Dashboard (Left Panel)
- **Board Visualizer**: Real-time circular position indicator showing motor angle with rotating needle
- **Status LEDs**: Visual indicators for Moving, Error, Homed, and Enabled states
- **Live Position**: Large display showing current position in degrees (updates 10 Hz)
- **Connection Panel**: Auto-detect COM ports with one-click connect/disconnect
- **Command Log**: Real-time command history with responses for debugging

### 🎮 Control Tab
- **Motor Control**: Large color-coded buttons (Enable/Disable/Emergency Stop)
- **Quick Jog**: Six preset jog buttons (-45°, -10°, -1°, +1°, +10°, +45°)
- **Absolute Move**: Move to specific position with speed control (0-1000 RPM)
- **Relative Move**: Move by distance with speed control
- **Set Zero**: Define current position as zero reference point

### 📊 PID Tuning Tab
- **PID Parameters**: Adjust Kp, Ki, Kd with sliders and numeric inputs
- **Real-time Plotting**: Matplotlib-based step response visualization
- **Step Response Test**: Automated testing with configurable step size and duration
- **Performance Metrics**: 
  - Overshoot percentage
  - Settling time (2% criterion)
  - Steady-state error
- **Live Data Recording**: Capture position vs time during moves
- **Preset Management**: Reset to default PID values

### 🔧 Peripherals Tab
- **RGB LED Control**: 
  - Color preview widget
  - Individual R/G/B sliders (0-255)
  - 8 preset colors (Off, Red, Green, Blue, Yellow, Cyan, Magenta, White)
  - Live color preview before applying
- **Endstop Monitoring**:
  - Real-time J_MIN and J_MAX status
  - Visual indicators (green = not triggered, red = triggered)
  - Auto-refresh at 5 Hz
- **Encoder Diagnostics**:
  - Raw encoder value (0-32767)
  - Communication status check
  - Manual refresh button

### ⚡ TMC Diagnostics Tab
- **CoolStep Control**: Enable/disable automatic power savings (50-75% reduction)
- **Current Settings**: Adjust run and hold current (100-2000 mA)
- **Real-time Monitoring**:
  - Driver status register
  - Temperature monitoring with color coding
  - StallGuard load indicator
- **Diagnostic Log**: Event logging for troubleshooting

### ⚙️ Settings Tab
- **Device Identity**: Configure CAN ID (1-6) for multi-axis systems
- **Motor Configuration**: Set current (0-2000 mA) and microstepping (1-256)
- **Save to Flash**: Persist all configuration to non-volatile memory
- **Apply Changes**: Send configuration to device in real-time

### 📤 Firmware Upload Tab
- **One-Click Upload**: Flash firmware directly from GUI
- **Auto-Find**: Automatically locate firmware.bin in project
- **Progress Tracking**: Real-time upload progress with detailed logging
- **Port Selection**: Easy COM port selection with auto-refresh
- **Flash Erase**: Optional full flash erase for clean installs
- **Error Handling**: Clear error messages and troubleshooting tips

See [FIRMWARE_UPLOAD_GUIDE.md](FIRMWARE_UPLOAD_GUIDE.md) for detailed instructions.

## User Interface

The application features a modern dark theme with:
- **Split-panel layout**: Dashboard on left, tabbed controls on right
- **Tabbed interface**: Control, PID Tuning, Peripherals, Settings
- **Color-coded buttons**: Green (enable), Orange (disable), Red (stop)
- **Real-time updates**: Position and status refresh at 10 Hz
- **Intuitive controls**: Large buttons with clear labels and emoji icons
- **Visual feedback**: Board visualization with rotating position indicator
- **Professional plotting**: Matplotlib integration for PID tuning graphs
- **Responsive design**: Resizable panels adapt to screen size

## Requirements

- Python 3.7+
- PyQt5 (GUI framework)
- pyserial (Serial communication)
- matplotlib (Plotting support)
- numpy (Numerical operations)
- esptool (Firmware upload support)

## Troubleshooting

### Port not detected
- Check USB cable connection
- Verify ESP32-S3 is powered and enumerated
- Try different USB port or cable
- Check Device Manager (Windows) or `ls /dev/tty*` (Linux)

### Connection fails
- Ensure no other program is using the port (Arduino IDE, PuTTY, etc.)
- Verify baud rate is 115200
- Reset ESP32-S3 and try again
- Check USB drivers are installed

### Commands not working
- Verify motor is enabled (green LED on board)
- Check encoder connection and communication
- Review command log for error messages
- Ensure firmware is flashed correctly

### Position not updating
- Check encoder magnet alignment (2-4mm distance)
- Verify SPI connections to TLE5012B
- Test encoder communication in dashboard

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                    Main Window (1400x900)                       │
├──────────────────────┬──────────────────────────────────────────┤
│  Dashboard (Left)    │  Tabbed Panels (Right)                   │
│  ├─ Connection       │  ┌────────────────────────────────────┐  │
│  ├─ Board Viz        │  │ 🎮 Control Tab                     │  │
│  ├─ Position         │  │  ├─ Motor Control                  │  │
│  └─ Command Log      │  │  ├─ Quick Jog                      │  │
│                      │  │  ├─ Absolute/Relative Move         │  │
│                      │  │  └─ Set Zero                        │  │
│                      │  ├────────────────────────────────────┤  │
│                      │  │ 📊 PID Tuning Tab                  │  │
│                      │  │  ├─ PID Parameters (Kp, Ki, Kd)    │  │
│                      │  │  ├─ Step Response Test             │  │
│                      │  │  ├─ Real-time Plot                 │  │
│                      │  │  └─ Performance Metrics            │  │
│                      │  ├────────────────────────────────────┤  │
│                      │  │ 🔧 Peripherals Tab                 │  │
│                      │  │  ├─ RGB LED Control                │  │
│                      │  │  ├─ Endstop Monitoring             │  │
│                      │  │  └─ Encoder Diagnostics            │  │
│                      │  ├────────────────────────────────────┤  │
│                      │  │ ⚙️ Settings Tab                     │  │
│                      │  │  ├─ Device Identity                │  │
│                      │  │  ├─ Motor Configuration            │  │
│                      │  │  └─ Save to Flash                  │  │
│                      │  └────────────────────────────────────┘  │
└──────────────────────┴──────────────────────────────────────────┘
```

## Advanced Usage

### PID Tuning Workflow
1. Connect to device and enable motor
2. Go to "PID Tuning" tab
3. Set initial PID values (start with defaults: Kp=2.0, Ki=0.1, Kd=0.05)
4. Click "Apply PID Parameters"
5. Configure step test (e.g., 45° step, 3s duration)
6. Click "Start Test" to record step response
7. Analyze metrics (overshoot, settling time, steady error)
8. Adjust PID values based on response:
   - **Too much overshoot**: Decrease Kp or increase Kd
   - **Slow response**: Increase Kp
   - **Steady-state error**: Increase Ki
   - **Oscillation**: Decrease Kp or Ki, increase Kd
9. Repeat test until satisfied
10. Save configuration to flash

### RGB LED Patterns
Use the Peripherals tab to set custom LED colors:
- **Idle**: Green (0, 255, 0)
- **Moving**: Blue (0, 0, 255)
- **Error**: Red (255, 0, 0)
- **Homing**: Yellow (255, 255, 0)
- **Custom**: Any RGB combination

### Endstop Testing
1. Go to Peripherals tab
2. Watch J_MIN and J_MAX indicators
3. Manually trigger limit switches
4. Verify indicators turn red when triggered
5. Use for homing sequence validation

## Keyboard Shortcuts

- `Ctrl+Q`: Quit application
- `Space`: Emergency stop (when connected)
- `Tab`: Switch between tabs
- `Ctrl+1-4`: Quick tab switching
