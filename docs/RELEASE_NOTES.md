# 2.0.3 release

Prepared 2026-10-16 for CLMD PCB v2.

- CLMD USB/CAN jog, speed and homing tests now enable and confirm the motor before sending motion. Failed enable aborts the test; an explicit **Enable / recover** action handles corrected latched faults. MKS Settings is no longer needed to arm the drive.
- Stationary hold enables by default at boot after driver and feedback checks. A separate saved `startup_enabled` option can disable it. Existing configuration blobs remain compatible. No startup move or homing is issued.
- Stop/Disable cancel pending startup for the current boot. USB-only startup attempts do not repeatedly poll an unpowered driver. Joint Control requires its normal fresh/aligned reference. Repeated enable is idempotent and does not reset an active move.
- Readback publishes enabled state before ACK. Firmware flashing text now reflects saved startup-hold behavior.

Firmware 2.0.3 builds successfully. Five production C suites (including startup/fault readiness), **40 Qt/protocol tests**, diagnostics/flash regressions, 47 standalone Lab tests and Studio codec compatibility pass. The Lab consumes samples only from the selected USB/CAN transport when both are connected.

## Powered board verification

- The real Qt Firmware tab app-only update passed a full 2 MB backup, programming and explicit readback. Bootloader, partitions and all pre-update motor/joint settings were preserved.
- Default startup enabled stationary hold with healthy feedback/driver status and no movement command. Saved startup Off and On each survived a software reset. Repeated F3 enable while already enabled acknowledged successfully without resetting the target. Disable remained disabled and canceled pending startup.
- **Direct PCB USB / COM12:** the actual CLMD panel enabled a disabled motor, ran a **+5° motor jog at 30 RPM**, then disabled it. Final commanded-target error while enabled was **+0.219727°**, within the configured 10-count motor tolerance.
- **Physical CANable / COM8:** the same panel enabled a disabled motor and ran a **−5° motor jog at 30 RPM** on 2.0.3, then disabled it. USB STATUS independently recorded the target/position; final error while enabled was **−0.197754°**. No MKS Settings action was used in either test.
- All existing settings were preserved, including the user's current **1600/200 mA** requests, motor sign −1 and joint mode Off. Startup hold is saved On; the board was explicitly left **disabled** after checks. The next normal boot applies startup hold after checks, and a CLMD test enables automatically.

Windows initially detected only CANable; reconnecting the PCB USB produced the ESP32 COM12 device and allowed the final update/tests. The preliminary 2.0.2 CAN test is superseded by the final powered 2.0.3 evidence above. These short bench moves verify enable/command/feedback/disable paths, not loaded gearbox accuracy or thermal endurance. Private device backups and raw flashing logs are excluded from distributable archives. The Studio Python sources and bundled firmware are updated; the complete Studio EXE was not rebuilt.

# 2.0.2 release

Prepared 2026-10-16 for CLMD PCB v2.

- Fixed run/hold current reverting on USB-only power. Disabled settings are persisted without requiring a powered TMC UART transaction. Every disabled-to-enabled path must program and verify the driver first.
- Current writes over USB and CAN are read back. Saved values and unsaved USB drafts are distinct; failed/partial applies keep the requested draft for correction.
- Added integrated **Motor & Encoder Lab** under Settings → CLMD Settings: actual TLE raw angle/SSC diagnostics, stationary/manual captures, measured speed tests and staged speed sweeps, raw endstop ADC/state, Hall midpoint suggestions and CSV exports over USB/CAN.
- Added read-only native E9–EC diagnostic/current queries and fresh-frame gating. Timed USB motion also has a worker deadline independent of the GUI timer. Existing joint feedback/control remains optional.

Firmware build, production C host tests, Qt/protocol tests, diagnostics-model tests and backup-first flash tests pass. Physical CAN wiring, loaded speed sweeps and gearbox commissioning require bench validation. The Studio Python source integration is updated; a complete packaged Studio EXE must be rebuilt.

## 2.0.2 verification

- Actual Qt Firmware tab app-only update passed complete 2 MB backup, programming and explicit readback. Existing bootloader, partition layout and board configuration were preserved.
- On USB-only power, the real Qt USB form saved/read back **900 and 1200 mA**; Drive parameters saved/read back **1641 mA** through native command 83. Run/hold **1200/350 mA** survived a software reset. All original settings were then restored: **1000/200 mA**, motor sign −1, joint mode Off, drive disabled.
- Actual integrated TLE capture: **69/69 valid samples** in about five seconds, motor-angle span **0.043945°**, standard deviation **0.007897°**, largest received-sample gap **94 ms**. No motor motion command was sent. Magnet setup was not independently inspected; these observations do not establish calibrated accuracy or loaded stability.
- E9–EC replies and the shared CAN diagnostic decoder were checked against the real firmware through **USB frame injection**. This is not a physical CAN bus test.
- Five production C suites, **34 Qt/protocol tests**, **5 diagnostic-model tests**, **4 flash tests**, **47 standalone Lab regressions**, SDK patch validation and **18 firmware-generated Studio feedback fixtures** passed.

The device-specific flash backup remains private and is excluded from distribution. Evidence is in the release's `verification/` folder. No powered movement, high-current thermal run, physical CAN bus test or physical joint-loop acceptance was performed for this update.

## Earlier releases

# 2.0.1 release

Prepared 2026-10-16 for CLMD PCB v2.

- CLMD Settings moved to **Settings**, beside GRBL and MKS Settings; removed from Robot.
- Manual jog, timed speed, homing, zero, enable/hold, disable and stop now work over either board USB or Studio CAN. USB transactions use the same firmware command handler, with acknowledgement checks, heartbeat and stop-on-failure behavior.
- Added Firmware tab for blank-board full installation and app-only upgrades: bundled firmware and source-only esptool runtime, SHA-256 checks, complete 2 MB backup before writing, explicit read-back verification, no automatic drive enable. No working firmware connection is required.
- Raised the software current ceiling from 1000 to **1641 mA RMS**, matching the nominal maximum for R22/R23 = 0.12 Ω. Default stays 1000 mA. Reported actual current accounts for register quantization. TMC2209's 2 A RMS chip rating cannot override the fitted sense resistance or thermal limits.
- Existing motor/joint settings layouts and default optional mode are preserved.

Host current tests, USB command/stop failure tests, Settings-ribbon tests and flash integrity/backup tests pass. Firmware builds against ESP-IDF 5.5.1. Physical loaded motion and high-current thermal operation remain commissioning tests. Board update and final GUI verification evidence are included in the versioned package when captured.

The actual GUI app-only update on COM12 passed backup, programming and explicit read-back verification. Existing motor/joint settings were preserved, live USB monitoring passed, the board reported its 1641-mA ceiling and stayed disabled. The full-install path is covered by automated image/backup tests; no bootloader/partition rewrite was performed on this existing board. A frozen-executable probe loaded the bundled flasher runtime and firmware successfully. The complete Studio executable was not rebuilt; the source integration and primary Windows build spec are updated.

The 2.0.0 verification below is historical baseline evidence, not a new 2.0.1 motion or stability result.

---

# 2.0.0 release and verification record

Prepared 2026-10-16 for CLMD PCB v2 / ESP32-S3 / 2 MB flash.

## Changes

- Optional MT6701 output feedback in Off, Monitor and Control modes.
- Configurable reduction, output direction/zero, settling tolerance, motor/output discrepancy bound and correction speed, persisted separately from existing motor settings.
- Output position control with backlash take-up, reference alignment, fault-on-loss behavior and actual motor limit supervision.
- Normal Studio coordinates become joint-derived in Control; raw TLE feedback stays available.
- Robot → CLMD Settings: saved joint settings, measured/ideal comparison, alignment and CSV export; six Studio ratios and independent CAN node setup.
- Production MT6701 reads at nominal 100 Hz with fixed configured address, explicit single-register transactions and stuck-low prechecks.
- Pinned ESP-IDF 5.5.1 NACK/STOP wait corrected to be bounded; disabled USB-only diagnostics do not depend on periodic unpowered TMC polling.
- Existing motor settings layout preserved; invalid storage inhibits enable. Disabled-only coordinate zero.

## Verification

| Check | Result / scope |
| --- | --- |
| ESP32-S3 build | Final 2.0.0 builds with ESP-IDF 5.5.1 and required SDK correction |
| Production core host test | PASS: motion, wrap, inputs, homing, limits, watchdog, faults |
| Joint model/controller host test | PASS: simulated 24:1 gearbox, backlash and reversals; Monitor comparison, missing/stale/jump checks, discrepancy bound, reference and heartbeat guards |
| MT6701 codec host test | PASS: exhaustive angle-byte decoding; legacy SSI safety/CRC compatibility |
| I²C mocked driver | PASS: 12 profiles, exact transactions, fixed address, transfer errors, recovery, stuck SDA/SCL and cleanup |
| Studio integration/Qt suite | 21 tests passed, including real ribbon callback, signed feedback, settings readback/failure, joint comparison and disabled alignment guard |
| Existing Studio codec | PASS: 18 compiled-C feedback fixtures, five real F5 encodings, completion and heartbeat helpers |
| Motor & Encoder Lab | 47 regression tests passed |
| SDK patch test | PASS: application, idempotence, unknown target rejection |
| Physical MT6701 wiring | Prior captures 700/700 success with MD explicitly at 3.3 V; user then confirmed encoder works in this configuration |
| Final-board USB check | Flashed app hash verified; 300/300 fresh aligned joint samples, zero transfer failures, valid TLE/inputs, E0–E8 native reply/reference checks with motor disabled |
| Settings/reboot | Original motor settings unchanged; optional mode restored to Off, persisted through reboot, drive disabled |

The prior 700-sample communications captures had no magnet and therefore do not establish angle stability. The user's later functional verification is recorded separately. The final 300-sample capture spans **4.306641°**; magnet presence/stationarity during that capture was not recorded. It establishes successful communication, not stationary angular stability. Confirm the magnet setup before output control. See the package's disabled final-board verification JSON; it does not claim a physical CAN bus test. Fault 3 was present with the separate motor supply absent; encoder and configuration checks passed independently.

## Commissioning still required

The new joint control loop has been tested in simulation, not on the loaded gearbox. Establish real reduction, direction, reversal play, output tolerance and discrepancy limits before enabling it. There is no guaranteed backlash-free positioning, magnet diagnosis or loaded angular accuracy claim.

Physical multi-turn movement, homing and limits on the mounted mechanism; host/encoder loss during real motion; six nodes together; maximum-speed behavior; and sustained load/thermal endurance require bench acceptance. Software version 2.0.0 identifies this completed feature set; it does not certify a mechanism's performance.

Current hardware remains disabled after release verification. Default optional joint mode is Off. App-only flashing preserves existing motor polarity, thresholds and CAN ID. The release includes matching source, binaries, hashes, notices and backup-first flash tooling. Device-specific flash backups are excluded from distributable archives.

Earlier diagnostic builds and repository material are historical. Use current documentation for wiring; earlier advice to ground MD or use an unavailable CSN does not apply to the verified five-pin module.
