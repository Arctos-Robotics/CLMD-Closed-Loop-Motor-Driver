# CLMD Studio firmware 2.0.3

The current source is `firmware/`. Earlier v1.x bench firmware is preserved separately. This version targets CLMD PCB v2, ESP32-S3R8, TMC2209 and TLE5012B, with an optional five-pin MT6701CT output encoder.

## Motor operation

- Boot holds ENN disabled during initialization, then **enables stationary hold by default** once configuration, driver power/UART, motor encoder and inputs pass. No move or homing runs. The saved `startup_enabled=0` setting leaves it disabled. An unsuccessful boot attempt is not repeatedly retried; a CLMD test or Enable retries after motor power is connected. Stop/Disable cancel pending startup for the current boot; faults do not automatically resume movement. Joint Control requires a fresh aligned reference before hold can enable.
- TLE5012B SSC angle/safety/CRC is checked at nominal **200 Hz**. Position unwrap uses **16384 wire counts per motor turn**. Invalid or physically implausible readings invalidate motor position and stop enabled motion.
- Position control uses encoder error, acceleration limiting and braking distance to command the TMC2209 internal velocity generator at **256 subdivision**, SpreadCycle. Default motor-only settling tolerance is 10 wire counts (about 0.22 motor degrees).
- Speed commands request motor velocity with encoder/stall supervision; there is no FOC current controller or guaranteed calibrated speed regulation.
- Current requests are 50–1641 mA RMS. R22/R23 are 0.12 Ω, giving a nominal full scale of about 1641.5 mA RMS at VSENSE=0. The default 1000 request corresponds to about 975 mA RMS. The chip rating is up to 2 A RMS; this board cannot reach that value through register scaling with its fitted sense resistors. Full-scale current is not a qualified continuous thermal rating: check sense-resistor power and driver cooling under load. Driver writes verify UART/register operation. Driver communication, thermal and short-circuit errors stop motion.
- Position/speed commands have a local **120-second** ceiling. Homing is bounded by configured time (up to **25 seconds**) and motor travel (default **12 turns**). Homing releases an initially active MIN switch, seeks, backs off 5 motor degrees and relatches at up to 5 RPM; seek speed is capped at 30 RPM.
- Configured actual motor-coordinate limits, MIN/MAX endstops, stall/direction checks and the optional joint guards apply to motion. Motor and output position across power loss require homing; single-turn absolute sensors cannot reconstruct lost multi-turn travel.

For output feedback and the optional backlash correction loop, see [JOINT_ENCODER.md](JOINT_ENCODER.md). Off and Monitor modes retain TLE motor positioning. Control mode reports joint-equivalent coordinates in normal CAN position replies so Studio's existing ratio conversion yields measured output position. Raw TLE motor position remains available over USB and CAN E5.

## Watchdogs and recovery

Default host timeout is **1500 ms** while moving, and while enabled in Joint control mode (including stationary holding). Addressed, checksum-valid CAN requests or USB `STATUS` supply the heartbeat. Other node traffic and invalid frames do not. Arctos Studio supplies periodic read-only queries independently of Digital Twin. A separate task disables ENN if the control task stalls beyond approximately 50 ms.

CAN bus-off disables motion and initiates bus recovery without resuming movement. F7 or USB STOP disables immediately, ahead of the command queue, and invalidates old queued commands. Explicit recovery is required. `DISABLE` also disables, preserving an existing latched fault.

Fault recovery: fix the cause, disable, re-establish any lost encoder reference, then explicitly clear/enable. Never auto-repeat a rejected motion request. Joint reference alignment requires the motor disabled and fresh valid motor and joint readings. A completed homing cycle establishes the reference automatically. Configuration changes require a disabled motor.

## Configuration

USB `GET_CONFIG` returns all keys. `SET key value` validates the complete candidate, persists that change and acknowledges success. The GUI writes changed fields in an order that keeps intermediate configurations valid, then reads the result back. Writes are individually persistent; a failed multi-field apply can leave earlier changes saved and is reported as a partial apply.

Run and hold requests persist with USB-only power while disabled. TMC programming/readback is mandatory before every disabled-to-enabled transition, including automatically enabled moves. `STATUS.current_pending` reports that driver configuration has not yet been verified. Persisted settings are published before acknowledgement so immediate USB/CAN readback sees the saved request.

`current_max_ma`, `run_actual_ma` and `hold_actual_ma` in GET_CONFIG are read-only derived values, not SET keys. The actual-current values are nominal register calculations, not current-sensor measurements.

The existing motor settings blob (version 2, namespace `clmd_studio`, key `settings`) remains layout-compatible. Joint settings use a separate versioned `joint` blob. Startup hold uses a separate uint8 `startup_enable` key, preserving the existing motor blob layout; a missing key defaults to enabled. Missing joint configuration defaults to Off. Corrupt or incompatible blobs are preserved and inhibit motion until a valid configuration is explicitly saved; `STATUS` exposes `settings_ok` and `joint_settings_ok`. There is no automatic NVS erase. Nothing is programmed into the MT6701's nonvolatile registers.

| Setting | Default | Meaning / bounds |
| --- | --- | --- |
| startup_enabled | 1 | Saved startup hold: 1 enabled after checks, 0 disabled; separate NVS key |
| node | 1 | CAN ID 1–127; Studio axes use 1–6 |
| run_ma / hold_ma | 1000 / 200 | RMS requests; hold ≤ run |
| fullsteps | 200 | 200 or 400 full steps per motor turn |
| max_rpm | 1200 | Motor ceiling, 5–1200 RPM |
| accel_rpm_s | 400 | Ramp ceiling, 50–600 RPM/s |
| watchdog_ms | 1500 | Host timeout, 500–10000 ms |
| motor_sign / encoder_sign | +1 / +1 | Each ±1; verify actual assembly |
| min_enabled / max_enabled | 1 / 0 | Enable each physical input |
| min_threshold / max_threshold | 874 / 874 | ADC midpoint, with hysteresis margins inside 0–4095 |
| hysteresis | 100 | ADC hysteresis, 10–500 counts |
| home_dir | 1 | 1 negative/CW, 0 positive/CCW, in configured motor coordinates |
| home_rpm / home_timeout_s | 30 / 25 | Seek RPM and total homing seconds |
| home_travel_turns | 12 | Maximum homing motor travel, >0 to 100 turns |
| require_home | 0 | Set 1 after commissioning a mounted robot |
| min_counts / max_counts | −8388608 / 8388607 | Actual motor-coordinate limits; reserve correction travel |
| joint_mode | 0 | 0 Off, 1 Monitor, 2 Control |
| joint_address | 6 | Seven-bit address 6 (0x06) or programmed 70 (0x46) |
| joint_sign | +1 | Calibrated output direction, ±1 |
| joint_ratio | 1 | Reduction, 1–100000 motor turns per output turn |
| joint_zero_deg | 0 | Absolute angle offset, −360 to +360 degrees |
| joint_tolerance_deg | 0.1 | Output settling tolerance, 0.022–5 degrees |
| joint_max_error_deg | 0.5 | Motor/output discrepancy bound, > tolerance, ≤30 degrees |
| joint_max_rpm | 60 | Position correction ceiling, 5–600 motor RPM |

The tested board's saved motor polarity is **−1**, encoder polarity +1. Factory defaults remain +1/+1; do not copy one assembly's signs blindly. Application flashing preserves stored polarity.

## Endstop inputs

MIN GPIO15 / ADC2 channel 4; MAX GPIO16 / ADC2 channel 5. Both are nominally sampled at 100 Hz, averaging 16 conversions. Two consecutive threshold crossings change the filtered state; three ambiguous samples inside the hysteresis band invalidate enabled input monitoring. Typical filtering delay is approximately 20–30 ms plus task/driver latency. Inputs are never driven as outputs.

On the tested KY-003 MIN sensor: no magnet ≈1737 ADC, magnet ≈10 ADC; threshold 874 with hysteresis 100 triggers ≤774 and releases ≥974. MAX is disabled by default. Recalibrate each board/sensor pair; this observation is not a guarantee for every KY-003/A3144 at 3.3 V over supply and temperature.

MIN protects the configured homing direction; MAX protects the opposite direction. After explicit recovery, movement away from an active limit is allowed. An ambiguous/disconnected signal is not universally distinguishable from a valid sensor state.

## Pin assignments

| Function | ESP32-S3 GPIO / connector |
| --- | --- |
| TLE5012B SCK / CS / DATA | 12 / 10 / 13 |
| TMC UART TX / RX | 39 / 40 |
| TMC STEP / DIR / ENN | 6 / 7 / 8 |
| TMC MS1 / MS2 | 1 / 2 |
| CAN TX / RX | 5 / 4 |
| MIN / MAX | 15 / 16 |
| LED red / green / blue | 11 / 9 / 14; active LOW |
| Optional MT6701 SDA / SCL | 17 / 18, U19 pins 3 / 4 |
| Expansion supply / ground | U19 pins 1 / 2, 3.3 V / GND |
| Spare expansion pin | U19 pin 5 / GPIO21, unused |
| Separate 5-V connector | U17 pin 1 +5 V, pin 2 GND |

See [protocol](PROTOCOL.md), [build/flash](BUILD.md) and [verification](RELEASE_NOTES.md).
