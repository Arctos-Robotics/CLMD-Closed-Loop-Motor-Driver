# Configuration Tool - Feature Overview

## 🎨 Modern UI Design

### Dark Theme
- Professional dark background (#1e1e1e)
- Blue accent colors (#4a9eff)
- High contrast for readability
- Smooth gradients and rounded corners

### Layout
- **Split-panel design**: Resizable dashboard and control panels
- **Tabbed interface**: Organized by function (Control, PID, Peripherals, Settings)
- **Responsive**: Adapts to different screen sizes
- **Intuitive**: Clear visual hierarchy and grouping

## 📊 Advanced Features

### 1. Real-time PID Tuning
**Purpose**: Optimize closed-loop performance without recompiling firmware

**Features**:
- Interactive sliders for Kp, Ki, Kd adjustment
- Live parameter updates to device
- Step response testing with configurable parameters
- Real-time plotting with Matplotlib
- Performance metrics calculation:
  - Overshoot percentage
  - Settling time (2% criterion)
  - Steady-state error
- Data recording and visualization
- Preset management (reset to defaults)

**Workflow**:
1. Set initial PID values
2. Apply to device
3. Run step response test
4. Analyze plot and metrics
5. Adjust parameters based on response
6. Iterate until optimal performance
7. Save to flash

**Use Cases**:
- Initial system tuning
- Adapting to different loads
- Optimizing for speed vs stability
- Troubleshooting oscillations
- Educational demonstrations

### 2. RGB LED Control
**Purpose**: Visual feedback and status indication

**Features**:
- Color preview widget showing selected color
- Individual R/G/B sliders (0-255 range)
- 8 preset colors for quick selection
- Live preview before applying
- Direct hardware control

**Preset Colors**:
- Off (0,0,0) - Motor disabled
- Red (255,0,0) - Error state
- Green (0,255,0) - Idle/Ready
- Blue (0,0,255) - Moving
- Yellow (255,255,0) - Homing
- Cyan (0,255,255) - Configuration mode
- Magenta (255,0,255) - Custom state
- White (255,255,255) - Test mode

**Use Cases**:
- Status indication for robotic arm
- Visual debugging
- User feedback
- Multi-axis coordination (different colors per axis)
- Demo/presentation mode

### 3. Endstop Monitoring
**Purpose**: Real-time limit switch status for safety and homing

**Features**:
- Live J_MIN and J_MAX status
- Large visual indicators (● symbols)
- Color-coded: Green (not triggered), Red (triggered)
- Auto-refresh at 5 Hz
- No polling overhead on firmware

**Use Cases**:
- Homing sequence validation
- Safety system testing
- Mechanical limit verification
- Troubleshooting switch wiring
- Travel range calibration

### 4. Encoder Diagnostics
**Purpose**: Verify encoder communication and alignment

**Features**:
- Raw encoder value display (0-32767)
- Communication status check
- Manual refresh button
- Value range validation

**Use Cases**:
- Encoder installation verification
- Magnet alignment check (should read ~16384 at 180°)
- SPI communication troubleshooting
- Mechanical coupling verification
- Calibration validation

### 5. Board Visualization
**Purpose**: Intuitive position feedback

**Features**:
- Circular encoder representation
- Rotating needle showing current angle
- Large position display in degrees
- Status LEDs (Moving, Error, Homed, Enabled)
- Smooth animations

**Visual Elements**:
- Blue gradient board background
- Dark encoder circle
- Red position indicator needle
- Color-coded status LEDs
- Professional appearance

### 6. Command Logging
**Purpose**: Debugging and operation tracking

**Features**:
- Real-time command history
- Response display
- Auto-scroll to latest
- Monospace font for readability
- Timestamp support (future)

**Logged Information**:
- All USB commands sent
- Device responses
- Connection events
- Error messages
- Configuration changes

## 🎯 User Experience Enhancements

### Connection Management
- Auto-detect COM ports
- Auto-refresh every 2 seconds
- One-click connect/disconnect
- Connection status indicator
- Graceful disconnect on close

### Button Design
- Large, easy-to-click buttons (40-50px height)
- Color-coded for function:
  - Green: Enable/Start
  - Orange: Disable/Pause
  - Red: Stop/Emergency
  - Blue: Normal actions
- Emoji icons for quick recognition
- Disabled state when not applicable
- Hover effects for feedback

### Input Controls
- Numeric spinboxes with range validation
- Sliders for visual adjustment
- Suffix labels (°, RPM, mA, etc.)
- Synchronized slider/spinbox values
- Reasonable default values

### Visual Feedback
- Real-time position updates (10 Hz)
- Status LED animations
- Color changes on state transitions
- Progress indication during moves
- Error highlighting

## 🔧 Technical Implementation

### Architecture
```
main.py
├── main_window.py (Main application window)
│   ├── dashboard_widget.py (Left panel)
│   │   ├── BoardVisualizer (Custom painted widget)
│   │   ├── Connection controls
│   │   └── Command log
│   └── Tabbed panels (Right side)
│       ├── control_panel.py (Motor control)
│       ├── pid_tuning_panel.py (PID tuning + plotting)
│       ├── peripherals_panel.py (LED, endstops, encoder)
│       └── settings_panel.py (Configuration)
└── serial_protocol.py (USB-CDC communication)
```

### Key Technologies
- **PyQt5**: GUI framework
- **Matplotlib**: Real-time plotting
- **pyserial**: Serial communication
- **NumPy**: Data processing for metrics
- **Custom painting**: Board visualization

### Performance
- Position updates: 10 Hz (100ms interval)
- Endstop polling: 5 Hz (200ms interval)
- Plot updates: 20 Hz during recording (50ms interval)
- Port refresh: 0.5 Hz (2000ms interval)
- Minimal CPU usage when idle

### Communication Protocol
All commands are ASCII text over USB-CDC at 115200 baud:
- `SET_PID <kp> <ki> <kd>` - Update PID parameters
- `LED <r> <g> <b>` - Set RGB LED color
- `GET_ENDSTOPS` - Query limit switch states
- `GET_ENCODER` - Get raw encoder value
- Plus all standard motor control commands

## 🚀 Future Enhancements

### Planned Features
- [ ] Multi-axis control (6 motors simultaneously)
- [ ] Motion sequence recording/playback
- [ ] Data export to CSV
- [ ] Configuration profiles (save/load)
- [ ] Firmware update via USB
- [ ] Auto-tuning wizard (Ziegler-Nichols)
- [ ] Bode plot analysis
- [ ] Trajectory planning visualization
- [ ] CAN bus monitoring
- [ ] Network control (TCP/IP)

### UI Improvements
- [ ] Keyboard shortcuts for all actions
- [ ] Customizable themes
- [ ] Dockable panels
- [ ] Multi-monitor support
- [ ] Touch-friendly mode
- [ ] Accessibility features

### Advanced Analysis
- [ ] FFT analysis for vibration detection
- [ ] Step response comparison (before/after)
- [ ] PID parameter history
- [ ] Performance benchmarking
- [ ] Statistical analysis

## 📚 Documentation

### User Guides
- Quick start guide (README.md)
- Feature overview (this file)
- PID tuning tutorial
- Troubleshooting guide
- Video tutorials (planned)

### Developer Docs
- Architecture overview
- Protocol specification
- Adding new features
- Custom widget development
- Testing procedures

## 🎓 Educational Value

This tool is excellent for:
- Learning PID control theory
- Understanding closed-loop systems
- Visualizing control system behavior
- Experimenting with parameters
- Teaching robotics concepts
- Demonstrating motor control
- Prototyping robotic systems

## 💡 Tips & Tricks

### PID Tuning
1. Start with Kp only (Ki=0, Kd=0)
2. Increase Kp until oscillation
3. Reduce Kp by 50%
4. Add Ki to eliminate steady-state error
5. Add Kd to reduce overshoot
6. Fine-tune all three together

### LED Usage
- Use different colors for different axes
- Blink patterns for status (future)
- Brightness indicates speed (future)
- Color transitions for smooth feedback

### Endstop Testing
- Test before homing
- Verify both switches independently
- Check for noise/bouncing
- Ensure proper pull-up resistors

### Encoder Verification
- Check raw value changes smoothly
- Verify full rotation (0-32767)
- Test at different speeds
- Confirm no communication errors

## 🏆 Advantages Over Basic Tools

Compared to simple serial terminals:
- ✅ Visual feedback (not just text)
- ✅ Real-time plotting
- ✅ Interactive controls
- ✅ Parameter validation
- ✅ Professional appearance
- ✅ Easier to use
- ✅ Better for demonstrations
- ✅ Integrated diagnostics
- ✅ No command memorization needed
- ✅ Suitable for non-technical users

## 🎯 Target Users

- **Robotics Engineers**: System integration and tuning
- **Students**: Learning control systems
- **Hobbyists**: Building robotic projects
- **Researchers**: Prototyping and experimentation
- **Manufacturers**: Production testing and calibration
- **Educators**: Teaching demonstrations
