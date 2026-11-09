# Performance you can evaluate

CLMD combines motor feedback, USB flashing, CAN integration and optional output feedback on a **$20 driver board**. Firmware 2.0.3 supports the fitted board's nominal **1.64 A RMS** current scale. The central robotics benefit is using a second encoder to correct the joint-position error introduced by gearbox play. This page separates implementation values from physical measurements.

## Published capabilities

| Quantity | Value / behavior | What the number means |
| --- | --- | --- |
| Motor position units | 16384 counts/turn; about 0.02197° per wire count | Protocol scale, not absolute accuracy; internal feedback may be fractional |
| Motor encoder angle representation | 15 bit; about 0.01099° per sensor increment | TLE5012B absolute angle; its 16-bit sine/cosine interface is a separate quantity |
| Motor control task | Nominal 200 Hz | Scheduled task rate; not a measured worst-case timing guarantee |
| Output acquisition | Nominal 100 Hz | When MT6701 is enabled; transfer/task time affects cadence |
| Output sensor increment | 360/16384 ≈ 0.02197° | MT6701 quantization, independent of gearbox reduction |
| Motor-only settling tolerance | 10 wire counts ≈ 0.220° | Controller completion tolerance, not measured positioning error |
| Output settling tolerance | Default 0.1°; configurable | Output Control setting, not measured repeatability |
| Run RMS current request | 50–1641 mA | Full scale calculates to 1641.5 mA with 0.12-ohm sense resistors; not a measured thermal rating |
| Motor speed ceiling | Configurable to 1200 RPM | Firmware cap, not proof every motor/load achieves it |
| Acceleration ceiling | 50–600 RPM/s; default 400 | Motor ramp setting |
| CAN bitrate | 500 kbit/s | Bus configuration, not axis synchronization accuracy |
| Default host timeout | 1500 ms | Valid addressed host traffic required during motion; also while enabled in output Control |

Sources: [firmware reference](FIRMWARE.md), [protocol](PROTOCOL.md), [joint reference](JOINT_ENCODER.md), and the included [production source](../firmware/main/core.c).

## Evidence included with this release

| Evidence | Result | Scope |
| --- | --- | --- |
| [C core log](verification/core.log) | PASS | Host tests of motion, limits, homing, watchdog, faults and wire encoding |
| [Joint log](verification/joint.log) | PASS | **Simulated** 24:1 gearbox with backlash/reversals and feedback guards |
| [Encoder codec log](verification/mt6701.log) | PASS | Host decoding tests |
| [I²C log](verification/mt6701_i2c.log) | PASS | Mocked driver transactions/failure paths |
| [USB flash record](verification/gui-flash-verification.json) | Backup, programming and explicit readback passed | Actual Studio Qt Firmware tab; 2.0.3 app-only update |
| [USB motion record](verification/usb-motion-verification.json) | +5° motor jog at 30 RPM; final feedback-target error +0.219727° | Actual panel, direct PCB USB, motor powered; disabled after test |
| [Physical CAN motion record](verification/can-motion-verification.json) | −5° motor jog at 30 RPM; final feedback-target error −0.197754° | Actual CANable path; USB STATUS independently read state, using the same motor sensor |
| [Startup record](verification/startup-verification.json) | Saved startup hold On/Off and explicit disable checked | Default healthy startup hold; no boot movement command |
| [Current test log](verification/current.log) | PASS | Nominal full-scale and request quantization checks |

The physical jog results use the board's motor encoder and the configured 10-wire-count completion tolerance. They verify enable, command, feedback and disable paths, not independent loaded metrology. The 2.0.3 records supersede earlier USB-handler-only CAN checks. Historical 2.0.0 joint communication captures and the 2.0.2 stationary capture are described separately in [RELEASE_NOTES.md](RELEASE_NOTES.md); their test conditions do not establish calibrated output accuracy.

The release record also contains GUI/integration tests and the firmware build result. Read [RELEASE_NOTES.md](RELEASE_NOTES.md) for their exact scope. Passing a simulation or disabled-board check does not establish loaded performance.

## Filmed gearbox bench test

With output feedback enabled on the 45:1 printed gearbox, settled output error went from 0.065° to 0.014° RMS across six endpoints. This was measured with the joint encoder on the filmed setup using a diagnostic build. The [test notes and traces](bench/) give the calculation, settings and measurement scope.

## Measure your assembly

Record the physical board identifier, firmware hash, motor model/step angle, supply voltage at the board, run/hold requests, cooling/ambient, load/inertia, gearbox ratio, encoder magnet setup, host/adapter and all relevant configuration. Use the [benchmark report template](BENCHMARK_REPORT.md).

1. **Positioning:** Use an independent angle/linear reference with stated uncertainty. Run repeated moves from both directions at several positions. Report mean signed error, maximum absolute error, spread, settling time, and sample count. Reading the same feedback encoder used by the controller is not an independent accuracy test.
2. **Speed under load:** Increase requested RPM in controlled steps. Log requested/measured speed, position tracking, faults and supply voltage. Label the highest passing speed by load and duration. If the test reaches its configured ceiling, call it a tested speed rather than a measured maximum.
3. **Thermal endurance:** Hold the intended duty cycle and load until temperature stabilizes. Record ambient, measurement method, motor/driver temperatures, runtime, cooling and failures. Driver warning flags do not measure winding temperature.
4. **Gearbox reversal:** In Monitor mode, log motor-derived travel and measured output travel through repeated reversals under consistent load. Their difference includes compliance, ratio and magnet errors as well as backlash. Validate output Control separately against an independent reference.
5. **Fault response:** On a secured bench, validate host loss, limit input, encoder loss and stop behavior. Record observed delay and final mechanical behavior; do not assume software disables bring every mechanism to rest.

Use Studio's joint CSV export for comparison logs. Keep raw data with the report. Repeat over the operating range you plan to claim.

## Comparing value fairly

Compare complete systems for the same motor, voltage, current, cooling, load and motion profile. Include the cost of any encoder, communication adapter and configuration tools required by each solution. Compare current, firmware and features actually shipping on the tested boards. Publish prices with date/currency and state whether shipping/tax is included.

CLMD's value story is **a $20 board with native USB, CAN, and dual-encoder capability for feedback-corrected output positioning**. Numerical superiority should follow a reproducible matched test. See the [feature tables](FEATURES.md) for encoder resolution, TMC capabilities and protection details.
