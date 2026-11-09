# Features, performance and hardware details

CLMD 1.5 brings motor control, feedback, communications and USB setup onto a **$20 board**. The second encoder is the key robotics feature: close the position loop around the actual gearbox output rather than estimating it solely from the motor.

## Feature and benefit table

| Area | Feature | Why it matters |
| --- | --- | --- |
| Cost | $20 board figure supplied by the maintainer | Affordable feedback-driven joints; confirm order conditions |
| Output positioning | Optional MT6701, Monitor/Control modes | See gearbox error and compensate reversal play using measured output |
| Motor positioning | TLE5012B, checked SSC data | Built-in shaft feedback and invalid-reading supervision |
| Installation | Native USB-C / ROM bootloader | No FTDI or external programming board |
| Flash workflow | Studio Firmware tab | Verify, back up, program and read back through a button-driven workflow |
| Current | Up to 1641 mA RMS request | Full nominal register scale with the fitted sense resistors |
| Multi-axis wiring | VIN/GND/CANH/CANL backbone | Four conductors carry power and communications between driver locations |
| Diagnostics | USB/CAN motor lab, Hall readings, CSV exports | Inspect movement, current settings and limits while commissioning |
| Fabrication | Editable source, Gerbers, BOM, placement, STEP | Order from a manufacturer and adapt the design |
| Access | Reciprocal open-source hardware/firmware licenses | Personal/commercial use, modification and manufacture under source-sharing terms |

## Encoder resolution: a fair comparison

| Angle representation | Positions per revolution | Nominal angular increment | Relative to 12-bit |
| --- | --- | --- | --- |
| 12 bit | 4096 | 0.087891° | 1× |
| 14 bit | 16384 | 0.021973° | 4× |
| **TLE5012B absolute angle: 15 bit** | **32768** | **0.010986°** | **8×** |
| 16-bit absolute angle, comparison only | 65536 | 0.005493° | 16× |

The TLE5012B's absolute angle representation has twice the positions of a 14-bit representation and eight times those of 12-bit. Its interface also exposes **16-bit sine/cosine data**, which is different from 16-bit absolute angle. The released CAN coordinate scale is **16384 counts per motor turn**, and internal positions may be fractional. Quantization alone does not establish accuracy. The optional MT6701 output sensor is 14-bit.

Manufacturer source: [Infineon TLE5012B datasheet, page 1](https://www.infineon.com/assets/row/public/documents/24/49/infineon-tle5012b-exxxx-datasheet-en.pdf).

## Current: why the board tops out near 1.64 A RMS

The fitted R22/R23 sense resistors are 0.12 Ω. With VSENSE=0 and the nominal 0.02 Ω internal resistance:

```text
I_RMS(full scale) = 0.325 / ((0.12 + 0.02) × sqrt(2))
                 ≈ 1.6415 A RMS
```

| Current quantity | Value |
| --- | --- |
| Firmware request range | 50–1641 mA RMS |
| Default request | Run 1000 mA / hold 200 mA |
| Nominal current at 1000-mA request | Approximately 975 mA RMS |
| Nominal full scale at 1641-mA request | Approximately 1641.5 mA RMS |
| TMC2209 silicon capability | Up to 2 A RMS / 2.8 A peak under suitable conditions |

The fitted board's nominal scale differs from the chip's headline rating. Read `current_max_ma`, `run_actual_ma` and `hold_actual_ma` for nominal register-derived values; these are calculations, not measured winding current. Continuous operation near full scale depends on PCB/sense-resistor dissipation, motor load and cooling. [Source calculation](../firmware/main/current.c).

## TMC technologies

| Technology | Hardware capability | Firmware 2.0.3 |
| --- | --- | --- |
| SpreadCycle | Dynamic chopper regulation | Enabled |
| StealthChop2 | Quiet motor operation | Not enabled; requires a firmware/configuration change |
| CoolStep | Adaptive current reduction | Not enabled; no CLMD energy-saving percentage claimed |
| StallGuard4 | Load/stall detection for StealthChop | Not the current motion-supervision path |
| MicroPlyer / 256 stepping | Interpolation/fine step control | 256 subdivision used; no interpolation gain claimed over that setting |
| Internal pulse generator | Register-commanded velocity | Used by the controller |
| Run and hold current | Separate programmed scales | Used, persisted and read back |
| Short/thermal diagnostics | Driver protection and flags | Used by driver health monitoring |

Reference: [Analog Devices TMC2209 product and datasheet](https://www.analog.com/en/products/tmc2209.html). These capabilities are reasons to build on the TMC platform; the current release does not enable every chip feature.

## Electrical and firmware protection

| Protection | Implementation in the included design / release | Boundary |
| --- | --- | --- |
| Input reverse polarity | D3 SS54 series diode on motor VIN | Blocks reverse input polarity; verify physical assembly |
| Input overcurrent | F1 BSMD1812-200-30V resettable fuse | Fuse response/rating differs from a precision current limit |
| Motor current regulation | TMC2209 chopper + programmed sense-resistor scale | Nominal scale is not a board thermal qualification |
| Motor-output short / driver overtemperature | TMC2209 hardware protection and diagnostic flags | Firmware health faults stop enabled operation |
| USB transient/ESD suppression | U3–U5 LESD5Z5.0CT1G devices | USB protection, not a motor-input voltage clamp |
| Logic power management | Buck/LDO regulation and USB/board rail selection | Regulation is not arbitrary overvoltage immunity |
| Motor/encoder/limits/host faults | CRC/plausibility checks, endstops, bounded homing, watchdogs | Commission settings for the actual mechanism |
| Motor-input overvoltage cutoff/clamp | No dedicated active cutoff/TVS identified in this export | Do not advertise general high-voltage protection or exceed qualified input conditions |

The schematic's +24V net label and individual chip limits do not establish a full board operating-voltage rating. See [power and wiring](WIRING.md#power).

## Four wires across the robot

```text
Controller / PSU ==== CLMD axis 1 ==== CLMD axis 2 ==== CLMD axis 3
                    VIN · GND · CANH · CANL
```

This is a four-conductor **backbone**, not a four-wire total for motors and sensors. Each axis still has local winding/sensor connections. Power and CAN use their designated board connectors; this does not imply one combined four-pin socket. Native addressing supports 1–127, while actual bus loading, cable length, timing and supply/cable current capacity determine the practical node count. Use two end terminators. Studio's standard robot mapping uses six axes.

## Backlash correction: what makes CLMD useful

Gearbox clearance is invisible to a motor-only encoder. The MT6701 measures after that clearance. Control mode uses the measured output for position/holding and can advance the motor through reversal play. Monitor mode lets you observe the difference first, without changing motor positioning.

This corrects the **positioning consequence** of backlash; it does not eliminate physical play, increase mechanical stiffness, or guarantee zero tracking error under arbitrary changing loads. The included reversal result is a 24:1 simulation. See [commissioning](JOINT_ENCODER.md) and [physical test evidence](PERFORMANCE.md).
