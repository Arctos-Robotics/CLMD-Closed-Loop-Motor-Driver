# Using CLMD

Complete [wiring](WIRING.md) and [first setup](QUICK_START.md) first. This guide describes Studio firmware 2.0.3.

## Choose your interface in Arctos Studio

| Interface | Use |
| --- | --- |
| Board USB | Saved setup, direct motor tests, firmware flashing and detailed diagnostics |
| CAN | Multi-axis commands and Studio tests using the bus adapter at 500 kbit/s |
| Your native CAN host | Custom machine integration using the documented protocol |

Use a Studio release that includes CLMD Settings. There are no Studio application modules or separate testing GUIs to install from this repository.

## Configure the drive

Configuration changes require a disabled motor. `GET_CONFIG` reads all fields; `SET key value` validates and saves one change at a time. Read back after applying a set of changes.

Set the actual `fullsteps`, suitable `run_ma` / `hold_ma`, and conservative `max_rpm` / `accel_rpm_s`. Correctly set `motor_sign` and `encoder_sign`: positive commanded motor motion should produce increasing configured position. An incorrect sign can produce a stall/direction fault. Stop and inspect rather than repeatedly retrying.

Use `min_enabled` / `max_enabled`, measured thresholds and hysteresis for the sensors actually fitted. Verify input states in both conditions. If bench-testing with no endstops, explicitly disable the absent inputs while the shaft is free; restore and validate them before installing a mechanism. Set real `min_counts` / `max_counts` in **motor-coordinate counts**, with clearance for braking and any joint correction. The broad defaults are not physical travel limits.

Read [all settings and bounds](FIRMWARE.md#configuration). `require_home=1` is appropriate after a mounted mechanism's homing has been commissioned. It prevents ordinary motion before a home reference is established.

## Arctos Studio setup

Open **Settings → CLMD Settings**. Choose **Board USB** for direct board tests or **CAN** for Studio's bus interface. USB also reads/writes the full saved configuration; the Firmware tab flashes over USB. Assign axis nodes **1–6**, one unique node per driver, and use **500 kbit/s**. Enter the true motor-turns-per-output-turn ratio for each axis. Connecting or reading settings does not enable a motor.

For initial tests, select a single axis, low speed and a small movement with mechanical clearance. Confirm measured direction/travel. Test stop before expanding travel. Configure and verify one board at a time, then connect all axes. Six-axis simultaneous operation needs acceptance testing on the actual machine.

## Native CAN hosts

Use standard 11-bit arbitration ID equal to the node number. Each payload ends with:

```text
checksum = (arbitration_id + sum(all preceding payload bytes)) & 0xFF
```

For node 1, status request F1 is `F1 F2`; a position query is `31 32`. Replies use the node ID, so hosts must distinguish the reply opcode/length and reject checksum failures or unrelated traffic. Use the byte layouts and checks in [PROTOCOL.md](PROTOCOL.md) when writing your own host.

Typical motion sequence:

1. Read position, input states, enabled flag and status; inspect detailed USB status if faults need diagnosis.
2. After fixing a latched cause, disable and explicitly clear protection with 3D. Do not automatically clear faults in a retry loop.
3. Enable with F3 and check the ACK.
4. Send F4 relative or F5 absolute position, with speed and acceleration. Verify the acceptance ACK, then poll status and position until complete, fault, or your own finite deadline.
5. Continue valid addressed read requests throughout motion. Default host timeout is 1500 ms; a query every 250 ms leaves margin in a lightly loaded bus, but measure your host/bus timing.
6. Use F7 to stop/disable and latch an emergency fault. Disable with F3 00 when ending normal operation.

All command layouts, endianness and fault codes are in [PROTOCOL.md](PROTOCOL.md). An ACK reports command acceptance; it is not completion. F1 returns 0 fault, 1 stopped, 2 moving, 5 homing. Position feedback establishes whether a stopped result reached the intended target.

Commands use **16384 counts per motor turn**. For reduction R and output travel θ degrees:

```text
motor counts = θ × R × 16384 / 360
```

At 24:1, a 1° output move is about 1092 counts. Avoid applying the ratio twice when Studio already converts output coordinates. Speed commands are motor RPM; the byte layout for measured speed is little endian while several position/speed request fields are big endian.

This firmware implements the documented subset of MKS-style commands plus CLMD extensions. It is not a claim of full SERVO42/57 compatibility, hardware STEP/DIR input support, G-code execution, ROS integration, or drop-in printer/CNC compatibility.

## Homing and limits

Homing uses MIN with configurable direction, seek speed, time and motor-travel bounds. The sequence releases an initially active switch, seeks it, backs off 5 motor degrees, and relatches at up to 5 RPM. Seek speed is capped at 30 RPM. Default time/travel ceilings are 25 seconds and 12 motor turns.

Before sending 91, verify MIN manually and confirm `home_dir`, travel allowance and physical clearance. An opposite-direction MAX limit remains useful where fitted. Set limits in actual motor coordinates, even in output Control mode. A software fault that disables torque can allow a gravity-loaded axis to fall; provide the mechanism's required restraint/brake.

Single-turn magnetic encoders do not remember multi-turn travel across loss of power. Re-home after restarting a mechanism that needs an absolute multi-turn reference. Coordinate-zero command 92 requires a disabled motor and does not mark the axis homed.

## Optional gearbox feedback

Commission motor-only mode first. Then wire MT6701 and select **Monitor** with the real ratio/sign. Align while disabled and verify measured/ideal travel on small reversals. **Control** uses the measured output for position and holding and may move the motor farther to take up gearbox play. It needs fresh/aligned feedback and a continuing host heartbeat while enabled, including stationary holding.

See [JOINT_ENCODER.md](JOINT_ENCODER.md) for settings, validity flags, discrepancy limits, magnet setup and the full procedure. The default 0.1° output settling tolerance is a controller setting, not an accuracy guarantee.

## Stop, diagnose and recover

USB `STOP` or native F7 immediately disables and latches an emergency fault. `DISABLE` also disables but preserves an existing fault. Disabled motors provide no commanded holding torque. To recover: fix the cause, disable, restore any lost encoder/homing reference, then explicitly clear/enable. Configuration changes cannot be made while enabled.

Addressed valid traffic supplies heartbeat; traffic for other nodes does not. Host loss, bus-off, invalid motor feedback, driver failures and configured limit/guard conditions stop motion. A separate control-task watchdog disables ENN after approximately 50 ms without control progress. Those behaviors are useful protective measures; the board is not presented as a certified safety controller.

Use [Troubleshooting](TROUBLESHOOTING.md) for individual faults and [PERFORMANCE.md](PERFORMANCE.md) to validate your assembly under its intended load.
