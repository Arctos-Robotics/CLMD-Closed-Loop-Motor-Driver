# Troubleshooting

Stop/disable before changing motor wiring, sensor wiring or configuration. Read and save configuration/diagnostics in Studio (`GET_CONFIG` and `STATUS` on the protocol) before changing values; the state that failed is useful evidence.

## Connection and setup

| Symptom | Check / next action |
| --- | --- |
| No USB port | Use a data cable, try a direct port, inspect power. Hold BOOT and tap RESET to enter download mode, then refresh ports. |
| Port access denied | Close Studio, Lab, flash tools and serial monitors. Only one program may own the port. |
| Port opens but commands fail | Check the firmware version in Studio; firmware may be an older bench build. Install the correct current package only after checking board compatibility. |
| Flash reports hash mismatch | Re-extract an unchanged trusted package. Do not bypass manifest verification. |
| Flash cannot connect | Use BOOT/RESET sequence, check the download-mode port and close other clients. See [Flashing](FLASHING.md). |
| Encoder works but driver fault remains on USB | The TMC stage needs motor supply. Logic/encoder success with USB alone does not verify driver power. |
| CAN transmits but gets no reply | Check 500 kbit/s, node ID, H/L, ground reference, two end terminators and adapter mode. Confirm responses, not just TX. |
| Studio CLMD controls unavailable | Use a Studio distribution with CLMD support, connect the correct board USB/CAN target and inspect its health. |
| Settings fail to apply | Disable first, observe bounds and hold ≤ run. Earlier successful writes are already saved; read back the full result. |
| New app boots with motion inhibited | Inspect `settings_ok` / `joint_settings_ok`. Explicitly save valid settings; do not erase NVS as a routine fix. |

## Faults

Use USB `STATUS` for the fault number and detailed health. F1 over CAN reports the high-level fault state, not the detailed fault number.

| Code | Meaning | Inspect |
| --- | --- | --- |
| 1 | Emergency stop | Intentional STOP/F7; clear explicitly only after the machine is ready |
| 2 | Motor encoder | Magnet, alignment, sensor wiring/CRC/plausibility and actual shaft motion |
| 3 | Motor driver | Separate motor supply, TMC UART, driver status, short/thermal conditions |
| 4 / 5 | MIN / MAX protection | Actual switch state, direction, ADC threshold/hysteresis and physical travel |
| 6 | Host or bus loss | Valid addressed heartbeat, host stalls, CAN bus-off and bus wiring |
| 7 | Stall / direction mismatch | Coil pairs, polarity, motor/encoder signs, available torque/current and load |
| 8 | Homing ceiling | MIN transition, home direction, timeout/travel limits and clearance |
| 9 | Control / configuration / ambiguous input | Settings validity, input readings, control interval and host logs |
| 10 | Joint feedback / reference | Fresh valid MT6701 + TLE data, wiring and disabled alignment in Control |
| 11 | Excess motor/output discrepancy | Ratio/sign, real backlash/compliance, magnet, allowed correction and bound |

Fix the cause, disable, restore reference/homing as needed, then explicitly clear protection and enable. Do not repeatedly clear and replay the same motion command. After recovery, movement away from an active physical limit is allowed under the firmware's directional rules.

## Output encoder

For the tested five-pin YYC-MT6701CT, wire MD explicitly to 3.3 V with the module supply. Floating/grounded MD failed the tested I²C configuration. Confirm GPIO17 SDA / GPIO18 SCL, common ground, address 0x06 or the module's programmed 0x46, and 3.3 V signal pull-ups. See [JOINT_ENCODER.md](JOINT_ENCODER.md).

Successful reads are not proof of a fitted magnet or calibrated angle. In Monitor, invalid joint feedback leaves motor-only control available; in Control it inhibits/stops motion. Re-align while disabled after a reference loss. Check `joint_valid` and `joint_aligned` before treating numeric values as measurements.

## Ask for help

Open a hardware, firmware or support issue using this repository's templates. Include product/PCB/firmware versions, wiring photos or a clear pin map, motor/supply/load, configuration, status, adapter/OS, and exact reproduction steps. Remove device backups, personal paths, identifiers and unrelated logs before posting.
