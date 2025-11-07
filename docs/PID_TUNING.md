# PID Tuning Guide

This guide will help you optimize the PID controller for your closed-loop stepper motor system to achieve the best performance.

## Understanding PID Control

The PID controller continuously adjusts motor position based on three terms:

- **P (Proportional)**: Responds to current error
- **I (Integral)**: Eliminates steady-state error
- **D (Derivative)**: Dampens oscillations and overshoot

```
Output = Kp × Error + Ki × ∫Error dt + Kd × dError/dt
```

## Default Parameters

The firmware ships with these default PID values:
- **Kp**: 2.0 (Proportional gain)
- **Ki**: 0.1 (Integral gain)
- **Kd**: 0.05 (Derivative gain)

These work well for most NEMA 17 motors with 1/16 microstepping.

## Tuning Process

### Step 1: Prepare for Tuning

1. Connect to your driver via USB
2. Enable the motor
3. Go to "PID Tuning" tab in configuration tool
4. Set step size to 45° (good starting point)
5. Set duration to 3 seconds

### Step 2: Baseline Test

1. Use default PID values (Kp=2.0, Ki=0.1, Kd=0.05)
2. Click "Start Test"
3. Observe the step response plot
4. Note the metrics:
   - **Overshoot**: Should be <15%
   - **Settling Time**: Should be <1.5s
   - **Steady Error**: Should be <0.5°

### Step 3: Tune Proportional Gain (Kp)

**Effect**: Kp determines how aggressively the system responds to error.

**Procedure**:
1. Set Ki=0, Kd=0 (disable I and D terms)
2. Start with Kp=1.0
3. Run step test
4. Gradually increase Kp until you see oscillation
5. Reduce Kp by 20-30%

**Symptoms**:
- **Too Low**: Slow response, large steady-state error
- **Too High**: Oscillation, overshoot, instability
- **Just Right**: Fast response with minimal overshoot

**Typical Range**: 1.0 to 5.0

### Step 4: Tune Derivative Gain (Kd)

**Effect**: Kd dampens oscillations and reduces overshoot.

**Procedure**:
1. Keep Kp from Step 3, Ki=0
2. Start with Kd=0.01
3. Run step test
4. Increase Kd until overshoot is minimized
5. If system becomes sluggish, reduce Kd slightly

**Symptoms**:
- **Too Low**: Overshoot, oscillation
- **Too High**: Slow response, noise sensitivity
- **Just Right**: Minimal overshoot, fast settling

**Typical Range**: 0.01 to 0.2

### Step 5: Tune Integral Gain (Ki)

**Effect**: Ki eliminates steady-state error over time.

**Procedure**:
1. Keep Kp and Kd from previous steps
2. Start with Ki=0.05
3. Run step test
4. Increase Ki until steady-state error is eliminated
5. If oscillation appears, reduce Ki

**Symptoms**:
- **Too Low**: Steady-state error remains
- **Too High**: Oscillation, overshoot, instability
- **Just Right**: Zero steady-state error, stable

**Typical Range**: 0.05 to 0.5

### Step 6: Fine-Tuning

1. Make small adjustments to all three parameters
2. Test with different step sizes (10°, 45°, 90°)
3. Test with different speeds (200, 400, 600 RPM)
4. Verify performance under load
5. Save configuration to flash when satisfied

## Tuning Methods

### Method 1: Ziegler-Nichols (Classic)

1. Set Ki=0, Kd=0
2. Increase Kp until sustained oscillation occurs (Ku)
3. Measure oscillation period (Tu)
4. Calculate:
   - Kp = 0.6 × Ku
   - Ki = 1.2 × Ku / Tu
   - Kd = 0.075 × Ku × Tu

### Method 2: Manual Tuning (Recommended)

Follow Steps 1-6 above for best results with this system.

### Method 3: Trial and Error

Start with defaults and adjust based on observed behavior:

| Problem | Solution |
|---------|----------|
| Slow response | Increase Kp |
| Overshoot | Increase Kd or decrease Kp |
| Oscillation | Decrease Kp or Ki, increase Kd |
| Steady error | Increase Ki |
| Noise/jitter | Decrease Kd |

## Performance Metrics

### Overshoot
- **Excellent**: <5%
- **Good**: 5-10%
- **Acceptable**: 10-15%
- **Poor**: >15%

### Settling Time (2% criterion)
- **Excellent**: <0.5s
- **Good**: 0.5-1.0s
- **Acceptable**: 1.0-2.0s
- **Poor**: >2.0s

### Steady-State Error
- **Excellent**: <0.1°
- **Good**: 0.1-0.3°
- **Acceptable**: 0.3-0.5°
- **Poor**: >0.5°

## Application-Specific Tuning

### Robotic Arm (Smooth, Precise)
```
Kp: 2.5
Ki: 0.15
Kd: 0.1
Max RPM: 400
Max Accel: 300 deg/s²
```
**Characteristics**: Low overshoot, smooth motion, good accuracy

### CNC Machine (Fast, Responsive)
```
Kp: 3.5
Ki: 0.25
Kd: 0.08
Max RPM: 800
Max Accel: 800 deg/s²
```
**Characteristics**: Fast response, higher overshoot acceptable

### Camera Gimbal (Ultra-Smooth)
```
Kp: 2.0
Ki: 0.08
Kd: 0.15
Max RPM: 300
Max Accel: 200 deg/s²
```
**Characteristics**: Maximum smoothness, minimal vibration

### 3D Printer (Balanced)
```
Kp: 3.0
Ki: 0.2
Kd: 0.06
Max RPM: 600
Max Accel: 500 deg/s²
```
**Characteristics**: Good balance of speed and accuracy

## Advanced Techniques

### Anti-Windup

The firmware includes integral anti-windup to prevent integral term from growing too large:

```c
if (integral > output_limit) integral = output_limit;
if (integral < -output_limit) integral = -output_limit;
```

This is automatically handled, but you can adjust `pid_output_limit` in settings if needed.

### Derivative Filtering

To reduce noise sensitivity, consider:
1. Lowering Kd if motor jitters
2. Increasing encoder filtering (hardware)
3. Using software low-pass filter on derivative term

### Gain Scheduling

For systems with varying loads:
1. Tune PID for typical load
2. Adjust Kp based on load (higher for heavy loads)
3. Use TMC2209 StallGuard to detect load changes

## Troubleshooting

### Motor Oscillates Continuously
- **Cause**: Kp or Ki too high
- **Solution**: Reduce Kp by 20%, reduce Ki by 50%

### Motor Doesn't Reach Target
- **Cause**: Kp too low, or position error limit too strict
- **Solution**: Increase Kp, check position error limit setting

### Motor Overshoots Significantly
- **Cause**: Kp too high, Kd too low
- **Solution**: Reduce Kp by 10%, increase Kd by 50%

### Steady-State Error Remains
- **Cause**: Ki too low or zero
- **Solution**: Increase Ki gradually (start with 0.1)

### Motor Vibrates or Jitters
- **Cause**: Kd too high, noise in encoder signal
- **Solution**: Reduce Kd, check encoder magnet alignment

### Slow Response Time
- **Cause**: Kp too low, max RPM too low
- **Solution**: Increase Kp, increase max RPM setting

## Testing Checklist

Before finalizing PID parameters, test:

- ✅ Small moves (1-10°)
- ✅ Medium moves (45-90°)
- ✅ Large moves (180-360°)
- ✅ Different speeds (100, 400, 800 RPM)
- ✅ Different accelerations (200, 500, 1000 deg/s²)
- ✅ With and without load
- ✅ Continuous operation (thermal stability)
- ✅ Limit switch triggering
- ✅ Emergency stop response

## Saving Configuration

Once you're satisfied with PID parameters:

1. Go to "Settings" tab
2. Click "Save to Flash"
3. Verify settings persist after power cycle
4. Document your final values for reference

## Example Tuning Session

```
Initial Test (Defaults):
Kp=2.0, Ki=0.1, Kd=0.05
Overshoot: 12%, Settling: 1.2s, Error: 0.2°
→ Acceptable, but can improve

Increase Kp:
Kp=2.5, Ki=0.1, Kd=0.05
Overshoot: 18%, Settling: 0.9s, Error: 0.2°
→ Faster but too much overshoot

Increase Kd:
Kp=2.5, Ki=0.1, Kd=0.08
Overshoot: 8%, Settling: 1.0s, Error: 0.2°
→ Much better!

Increase Ki:
Kp=2.5, Ki=0.15, Kd=0.08
Overshoot: 9%, Settling: 1.0s, Error: 0.05°
→ Excellent! Save these values.
```

## Resources

- [Firmware Architecture](firmware_architecture.md) - PID implementation details
- [Configuration Tool Guide](../software/README.md) - Using PID tuning tab
- [Hardware Description](hardware_description.md) - Encoder specifications

---

**Pro Tip**: Start conservative and gradually increase gains. It's easier to add performance than to fix an unstable system!
