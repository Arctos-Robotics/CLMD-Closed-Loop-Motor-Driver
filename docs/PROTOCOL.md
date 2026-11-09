# USB and CAN contract — 2.0.3

## USB

Board USB Serial/JTAG, 115200 nominal, ASCII commands terminated by newline. Replies are `OK ...`, `ERROR ...`, a single JSON object, or `FRAME node hex`. Only one client should own the port. Disable DTR/RTS on opening to avoid unwanted resets.

| Command | Effect |
| --- | --- |
| VERSION | Firmware version and capability tokens |
| GET_CONFIG | Persisted motor/joint configuration JSON |
| STATUS | Live motor, limits, driver and joint JSON; supplies host heartbeat |
| SET key value | Validate/persist one field while motor disabled |
| ENABLE / DISABLE | Explicit drive enable / immediate disable |
| STOP | Immediate disable and latched emergency stop |
| JOINT ALIGN | Capture motor/joint reference while disabled, with both readings fresh |
| MT6701 START [hz address method] | Begin disabled-motor diagnostic; defaults 100000, 6, 1 |
| MT6701 READ / MT6701 STOP | Raw structured sample / end diagnostic |
| CAN node hex | Inject a complete native frame including checksum; returns FRAME |

Capabilities include `MT6701_I2C_V1 MT6701_TEST_V2 JOINT_FEEDBACK_V1 JOINT_CONTROL_V1 MOTOR_DIAG_V1 CURRENT_STORE_V1 STARTUP_HOLD_V1`. Older 2.x diagnostic firmware lacks joint configuration/control. Injection exercises the native handler, not physical CAN wiring.

`STATUS.position_counts` is always actual TLE motor position. Joint fields: mode, valid, raw, calibrated absolute angle, aligned, error code, failure count, age, equivalent motor coordinate, measured output travel, ideal output travel, signed difference and target output travel. Degrees are output degrees. Numeric placeholders while invalid/unaligned must not be treated as measurements: check **joint_valid** and **joint_aligned**. `encoder_ok` also gates motor-derived data. `settings_ok` / `joint_settings_ok` report valid configuration storage.

For normal setup and testing, use Arctos Studio. The commands above define the board's USB contract for custom hosts; no Python host tools are included here.

## Native CAN

Standard 11-bit frames at **500 kbit/s**, node IDs 1–127. ID 0 broadcasts supported controls without replies. Checksum is `(arbitration_id + sum(all preceding data bytes)) & 255`. Exact lengths/checksums are validated. Unsupported commands return `[command, 0, checksum]`. There are no unsolicited position broadcasts.

| Hex command | Request before checksum | Response / behavior |
| --- | --- | --- |
| 30 | `30` | signed int32 carry BE + uint16 within-turn BE; total 8 bytes |
| 31 | `31` | signed int48 cumulative coordinate BE; total 8 bytes |
| 32 | `32` | signed int16 measured motor RPM **LE**; total 4 bytes |
| 34 | `34` | Input mask bit0 MIN, bit1 MAX; total 3 bytes |
| 3A | `3A` | Enabled flag; total 3 bytes |
| F1 | `F1` | 0 fault, 1 stopped, 2 moving, 5 homing |
| F3 | `F3 enabled` | Enable/disable; ACK 1 accepted / 0 rejected |
| F4 / F5 | `op speed_BE16 acc position_signed24_BE` | Relative / absolute position |
| F6 | `F6 dir_speed_high speed_low acc` | Packed 12-bit motor RPM; high bit CW; zero stops and holds |
| F7 | `F7` | Immediate stop/disable, latched emergency fault; broadcast supported |
| 90 | `90 00 direction speed_BE16 01` | Active-low homing parameters; speed cap 30 RPM |
| 91 | `91` | Bounded homing using MIN |
| 92 | `92` | Coordinate zero while **disabled**; does not claim homed |
| 83 | `83 current_BE16` | Persist RMS run request 50–1641 mA, disabled only |
| 9B | `9B ratio` | Persist hold ratio 0–8 →10–90%, disabled only |
| 8A | `8A node` | Persist node, disabled; reply uses old node |
| 82 | `82 04` | SR_CLOSE only |
| 84 | `84 00` | 256 subdivision only |
| 3D | `3D` | Clear protection while disabled and healthy |

30/31 positions are **16384 counts per motor turn** in Off/Monitor. In Control they are joint-equivalent motor coordinates; after Studio divides by its ratio they represent measured output position. Commands remain in equivalent motor counts in every mode. Velocity feedback remains actual motor RPM. F4/F5 speed requests up to 3000 RPM are accepted but clamped to configured limits; outer position control adds its own motor-speed ceiling. Acceleration byte maps to `min(accel_rpm_s, 50 + 10 × byte)`; zero selects configured ramp.

## Joint extensions

Read requests E0–E3, E5–E8 contain the opcode plus checksum, addressed only. Replies are 8 bytes. E4 is an addressed alignment command returning the normal 3-byte ACK. These are CLMD extensions, not MKS-compatible settings commands.

Flags: bit0 joint mode enabled, bit1 sample fresh/valid, bit2 fresh and aligned, bit3 Control mode, bit7 integer overflow. Treat values as unavailable when their required validity/reference flag is clear. Query E0 for status before interpreting others. E5 has no flags; verify motor health using STATUS / normal position query.

| Opcode | Reply bytes before checksum |
| --- | --- |
| E0 | `E0 flags raw14_BE16 age_ms_BE16 error_boolean` |
| E1 | `E1 flags absolute_joint_millidegrees_signed32_BE 00` |
| E2 | `E2 flags (measured − ideal)_millidegrees_signed32_BE 00` |
| E3 | `E3 flags joint_equivalent_motor_counts_signed32_BE 00` |
| E4 | `E4 result`; disabled-only align |
| E5 | `E5 actual_TLE_motor_counts_signed48_BE` |
| E6 | `E6 flags ideal_output_travel_millidegrees_signed32_BE 00` |
| E7 | `E7 flags measured_output_travel_millidegrees_signed32_BE 00` |
| E8 | `E8 flags target_output_travel_millidegrees_signed32_BE 00` |

E2/E3/E6/E7/E8 require alignment and valid motor feedback for meaningful comparisons. Values in separate request/reply frames are not an atomic simultaneous sample. For logging, USB STATUS returns one controller snapshot; actual sensor acquisitions are sequential.

## Motor diagnostic extensions (2.0.3)

Addressed read requests E9–EC contain only opcode plus checksum; replies are 8 bytes. All integers below are big endian. These read-only queries do not enable motion.

| Opcode | Reply bytes before checksum |
| --- | --- |
| E9 | `E9 flags raw15_BE16 SSC_safety_BE16 fault` |
| EA | `EA MIN_ADC_BE16 MAX_ADC_BE16 mask core_mode` |
| EB | `EB driver_status_BE32 flags fault` |
| EC | `EC run_request_mA_BE16 hold_request_mA_BE16 maximum_mA_BE16` |

E9 flags: bit0 CRC/safety-valid frame acquired within 50 ms; bit1 valid motor position and fresh frame; bit2 enabled; bit3 inputs healthy; bit4 driver configured. Interpret the angle only when frame/position flags are valid. EA mask: bit0 MIN active, bit1 MAX active, bit2 inputs healthy, bit3 MIN enabled, bit4 MAX enabled. EB flags: bit0 enabled, bit1 driver configured. Reserved bits are zero. E5 supplies signed actual TLE motor counts, independent of joint control coordinates; query E9 to gate validity. RPM still uses command 32 and little-endian encoding.

USB STATUS additionally reports `encoder_frame_ok`, `encoder_frame_raw`, `encoder_dt_us`, `driver_configured` and `current_pending`. The frame-freshness bound is 50 ms. `current_pending` means the driver configuration has not been verified; it may be true even if no current edit was made, such as after boot without motor power. EC and GET_CONFIG return stored desired current, not a measurement of winding current. Current requests can be saved on USB power while disabled; every subsequent disabled-to-enabled transition programs/verifies the driver before enabling.

## Startup hold (2.0.3)

GET_CONFIG/SET `startup_enabled` use 0 disabled, 1 enabled (default). This setting is saved separately without changing the motor settings blob layout. STATUS adds `startup_enabled`, `startup_pending`, `startup_settings_ok`. Boot performs one verified stationary-enable attempt when feedback is ready. A USB-only driver failure does not repeatedly interrupt encoder monitoring; a later explicit enable/test retries. Stop/Disable cancel pending startup for the boot. Other latched faults do not trigger automatic restart. Joint Control requires fresh aligned output feedback. F3 1 on an already enabled healthy driver acknowledges success without resetting motion.

## Fault codes

| Code | Meaning |
| --- | --- |
| 0 | Healthy |
| 1 | Emergency stop |
| 2 | Motor encoder |
| 3 | Driver power/UART/status |
| 4 / 5 | MIN / MAX protection |
| 6 | Host timeout or bus loss |
| 7 | Stall or direction mismatch |
| 8 | Homing time/travel ceiling |
| 9 | Control interval, input ambiguity or configuration |
| 10 | Joint encoder invalid/stale or reference unavailable in Control |
| 11 | Motor/output discrepancy exceeds configured output-degree bound |

Faults latch and require explicit recovery after the cause is corrected. Monitor mode reports joint failures without introducing joint faults into motor control. Off mode requires no external encoder.
