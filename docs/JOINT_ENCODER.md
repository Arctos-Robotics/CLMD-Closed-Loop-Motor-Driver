# MT6701 gearbox feedback and backlash control

## Verified five-pin wiring

The tested module is marked **YYC-MT6701CT**. It exposes 5V, GND, MD, SDA and SCL; there is no CSN pin. Use connector pin numbers rather than assuming a viewing direction.

| Module terminal | CLMD PCB v2 connection |
| --- | --- |
| 5V (module power input) | U19 pin 1, **3.3 V** |
| MD | U19 pin 1, **3.3 V**, together with module power |
| GND | U19 pin 2, common ground (U17 pin 2 is also ground) |
| SDA | U19 pin 3, **GPIO17** |
| SCL | U19 pin 4, **GPIO18** |
| Unused | U19 pin 5, GPIO21 |

**MD must be HIGH for I²C/SSI.** Grounding MD selects ABZ/UVW and caused the earlier intermittent readings. Leaving MD floating also failed on this module. Explicit MD-to-3.3-V wiring produced 700/700 successful transfers in the diagnostic captures; the user subsequently confirmed operation with a magnet.

U17 pin 1 is a separate **5 V** output. ESP32 GPIOs are not 5-V tolerant. The verified connection above powers this module from 3.3 V despite the terminal label. Other module revisions may have different supply paths or pull-ups; a label alone does not establish signal voltage. The chip supply specification is 3.0–5.5 V. R9/R10 on the driver pull SDA/SCL to 3.3 V. Power off before rewiring.

USB VBUS supplies the board's logic regulator and both encoders; 12 V motor power is unnecessary for encoder diagnostics. A TMC2209 driver fault can occur when its separate motor supply is absent and does not invalidate a successful MT6701 transfer.

Fit a separate diametrically magnetized magnet centered above the output encoder. The manufacturer's reference uses approximately Ø6 × 2.5 mm with a 0.5–2 mm chip-to-magnet gap. A successful I²C read without a magnet is not a valid position measurement. I²C angle data has no CRC or magnetic-field quality flags, so firmware cannot certify magnet presence or angle accuracy from a successful transfer.

## Encoder roles and units

The TLE5012B measures the motor shaft and remains required in every mode. The MT6701 measures the gearbox output. Each encoder has its own direction; verify that positive motor movement produces positive calibrated joint movement.

`joint_ratio` is **motor turns per output turn**: enter **24** for a 24:1 reduction. Set the same ratio for the corresponding Studio axis. Studio applies the robot ratio once; firmware uses its copy to interpret output feedback. Do not multiply commands a second time.

With the motor disabled, `JOINT ALIGN` captures a motor and joint reference at the current position. From that reference:

```text
ideal output travel = (TLE motor travel in degrees) / joint_ratio
measured output travel = unwrapped calibrated MT6701 travel
difference = measured output travel - ideal output travel
joint-equivalent motor coordinate = motor reference + measured output travel × ratio
```

At 24:1, 24° of motor travel ideally gives 1° of output travel. If output travel is 0.8°, the reported difference is −0.2°. This difference includes backlash, compliance, ratio error, magnet error and alignment error; it is not an isolated backlash measurement. Measure reversal dead travel under consistent load to characterize backlash.

`joint_zero_deg` changes calibrated absolute angle as `(raw_degrees - zero) × sign`, wrapped to 0–360°. Alignment establishes relative travel and preserves the current motor coordinate; it does not silently zero a robot axis. Position across power cycles requires homing. Successful homing re-establishes the joint reference at coordinate zero.

## Modes

| `joint_mode` | Behavior | Missing MT6701 |
| --- | --- | --- |
| 0 Off (default) | Motor position control using TLE5012B; external GPIO bus unused | Motor driver works normally |
| 1 Monitor | Read MT6701, compare output with motor/ratio; TLE still controls positioning | Joint data becomes invalid; motor control continues |
| 2 Control | Position commands and position holding use joint-equivalent feedback; TLE still checks motor motion | Disables drive, latches fault 10, requires a new reference and explicit recovery |

The outer loop uses measured output error with acceleration limiting and braking to command motor velocity. It can move the motor farther than the ideal ratio movement to take up backlash. It is a bounded position controller, not automatic identification of backlash or a torque/FOC controller. Velocity commands still specify **motor RPM**; mode 2 also enforces fresh/aligned joint data and discrepancy protection during velocity tests.

The position correction speed is capped by `joint_max_rpm` (default **60 motor RPM**), configured motor maximum and requested speed. Homing uses the separate homing cap. Actual TLE motor-coordinate limits and physical endstops remain active; reserve sufficient motor travel for correction.

`joint_max_error_deg` (default **0.5° at the output**) limits the difference between actual motor position/ratio and joint position from the reference. Exceeding it disables the drive with fault 11. This is a discrepancy bound, not a limit on target error. It must exceed `joint_tolerance_deg` and the expected backlash/compliance allowance, while staying appropriate for the mechanism. A limit below real backlash prevents correction.

`joint_tolerance_deg` defaults to **0.1° output**. The sensor's 14-bit quantization is about **0.022° per output count**, independent of reduction. Tolerance is not a guaranteed accuracy. Completed position holds resume correction if output drifts beyond twice the tolerance. Holding in mode 2 requires a continuing host heartbeat; loss disables the motor.

## Commissioning sequence

1. Start in **Off**. Verify TLE motor polarity, physical limits, currents and homing with the motor supply fitted. Assign each board a unique CAN node over USB, one board at a time.
2. Disable the motor. Set **Monitor**, the real reduction, I²C address (normally `6` / 0x06), output direction and optional absolute zero. Apply and verify.
3. Align the reference while stationary. Monitor measured output travel, ideal travel and their difference. Run small moves in both directions, then export the joint CSV. Correct sign or ratio errors before proceeding.
4. Disable. Choose a discrepancy limit above measured reversal play, a suitable tolerance and a low correction speed. Set **Control**, apply, wait for fresh readings and align again.
5. Explicitly recover/enable and make small position moves with clearance from limits. Confirm measured output settles near the target and the motor takes up reversal play. Record behavior under the intended load before increasing speed.
6. Disconnecting an encoder, restarting, changing joint parameters or starting a diagnostic session invalidates the reference. In Control, fix the cause, disable, align and explicitly recover. No automatic motion resumes after a fault or reboot.

All joint settings are stored on the driver, in a separate NVS blob. The reference and unwrapped turns are deliberately **not** persistent. No EEPROM/register programming is performed on the MT6701.

## GUI and diagnostics

In **Settings → CLMD Settings → USB parameters**, select the board port to read/edit its configuration. The **Gearbox / joint feedback** tab shows absolute angle, measured travel, ideal travel, signed difference, freshness and reference state. It can copy the selected Studio ratio into the USB form (Apply is still required), align over USB/CAN, and export joint readings. CAN monitoring is explicitly selected and requires final firmware; USB readings identify their own board node.

Use Arctos Studio's joint feedback/diagnostics. Production acquisition uses 100 kHz, explicit single-register transactions and the configured fixed address. A diagnostic session requires a disabled motor, blocks motion/settings and invalidates the production reference.

Production sampling is nominally 100 Hz inside the 200-Hz control task. Transfers and task load affect actual cadence. Stale samples, failed reads, implausible jumps or lost motor validity invalidate the reference. Freshness is at most 50 ms and tightens at low reduction/high speed to avoid ambiguous wrap. SDA/SCL stuck-low checks and the included ESP-IDF STOP timeout patch bound known failure paths. These checks do not detect every possible erroneous but plausible angle.

Manufacturer reference: [MagnTek MT6701 Rev. 1.8 datasheet](https://www.magntek.com.cn/upload/pdf/202312/MT6701_Rev.1.8_%E4%B8%AD%E6%96%87%E7%89%88.pdf), I²C reference on page 20, ABZ reference on page 11.
