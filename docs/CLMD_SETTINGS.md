# CLMD Settings in Studio

Open **Settings → CLMD Settings** in an Arctos Studio distribution that includes CLMD support. Studio supplies this interface, the testing tools and its flashing runtime separately from the board repository.

## Test over board USB or CAN

Choose **Test connection** at the top of the panel:

- **CAN** uses Studio's existing Closed Loop / CAN connection at 500 kbit/s. Choose the target node, normally 1–6 for the robot.
- **Board USB** uses the motor board's own port. Connect it under USB parameters. The target node is read from that board and cannot accidentally address another CAN node. CANable is not required.

Both routes provide jog, timed speed, homing, zero, enable/hold, disable and emergency stop. Position tests use motor-equivalent degrees; speed tests use motor RPM. In Joint control mode position feedback is derived from the output encoder. Use one motion controller at a time when doing manual tests.

USB tests use the same native firmware command handler as CAN, with node/opcode/checksum/length validation. The worker supplies STATUS heartbeat while connected. Failed/lost motion acknowledgements trigger STOP without automatically retrying a move. Closing a USB test sends STOP; loss of communication is bounded by firmware watchdogs. Timed tests disable at the end. Real movement requires the separate motor supply and a healthy commissioned board.

**Drive parameters** writes current and hold ratio through the selected test connection. **USB parameters** reads and persists the full board configuration. Settings/zero require a disabled motor. Joint direction, reduction, reference and output-loop setup are described in [JOINT_ENCODER.md](JOINT_ENCODER.md).

## Enable and drive without MKS Settings (2.0.3)

Choose **Board USB**, connect the **CLMD PCB USB port** under USB parameters, and use Jog, Speed test or Home. For CAN, choose **CAN** and connect Studio to the CANable adapter. Both paths now enable and confirm the drive before sending a test move; MKS Settings is unnecessary. Failed enable never sends a move. Existing emergency/encoder/limit faults require correcting the cause and using **Enable / recover** explicitly. Tests do not silently clear those faults.

Firmware 2.0.3 defaults to stationary motor hold at startup after configuration, driver, encoder and input checks. It never runs a move or homes at boot. **Enable stationary hold at startup** in USB parameters can be saved Off after disabling the motor. Stop/Disable cancel any pending automatic startup for this boot. Default hold requires the separate motor supply; a failed USB-only startup is attempted once, and a later test/Enable retries when power is available. Joint Control also requires reference alignment. Enable is idempotent while already enabled: it does not reset an ongoing move.

Connect board USB and motor supply for direct USB motion. USB does not provide winding power. The CANable USB connection powers the adapter; it does not replace the motor supply. Windows must enumerate the ESP32 USB device (VID 303A / PID 1001) for direct PCB communication; selecting the CANable serial port cannot communicate with the PCB USB protocol. Use a data-capable cable.

## Motor & Encoder Lab

Select **Board USB** or **CAN**, choose/connect the target, then open **Motor & Encoder Lab** and enable monitoring. Raw CAN diagnostics require firmware **2.0.2**. USB monitoring targets about 20 samples/s; CAN polling targets about 5 samples/s and uses several sequential request/reply frames. These rates depend on connection latency. USB is preferable for detailed logging.

- **Stationary capture** measures valid/invalid samples, motor-angle span, standard deviation and gaps for 1–30 seconds. Keep the magnet and motor still. **Hand rotation capture** measures actual TLE motor travel while disabled. Both stay disabled; they never enable the motor. Display zero only changes the local plot reference.
- Raw 15-bit angle, SSC safety word, frame/position validity, actual motor RPM, driver status and MIN/MAX ADC/state are visible. Invalid encoder samples clear the local travel reference instead of displaying a fabricated angle. The TLE always measures the motor, including in joint Control mode.
- Hold a Hall input in its no-magnet state, capture it, then repeat with the magnet. The suggested midpoint/hysteresis is copied to the USB form only when requested; **Apply** is still required to persist it. Observe both input states and confirm polarity before homing.
- With motor power connected and the mechanism commissioned, run a measured speed test or **Find speed limit**. A sweep increases requested speed by about 1.5 times per stage, waits for the configured ramp and plateau, compares measured RPM, disables between stages and stops on failure. It does not change the saved speed ceiling. Firmware may clamp the requested speed to that ceiling; a resulting mismatch is a test result, not proof of a motor limit. Use the global Stop button to cancel. Do not start a sweep on an attached mechanism unless its travel is clear.
- Export encoder CSV or the measured test-results table. Invalid rows remain in CSV with blank derived angles. A capture with communication gaps does not establish uninterrupted travel. Host timestamps measure receipt time, not precise sensor acquisition time. CAN values are sequential snapshots and are not a simultaneous sample.

These are desktop diagnostics, not a high-rate oscilloscope or a guarantee of loaded positioning performance. Joint comparison and outer-loop setup remain in the separate Joint feedback tab.

## Current setting

The TMC2209 chip is rated for up to **2 A RMS / 2.8 A peak**, subject to thermal design. The saved PCB v2 schematic has **R22 and R23 = 0.12 Ω**. With VSENSE=0:

```text
full-scale RMS current = 0.325 V / (0.12 Ω + 0.02 Ω) / sqrt(2)
                       ≈ 1.6415 A RMS
```

Firmware 2.0.1 accepts up to **1641 mA**, representing full scale; the previous artificial 1000-mA cap is removed. The default remains 1000 mA. GET_CONFIG also reports `current_max_ma`, `run_actual_ma` and `hold_actual_ma`, because current-register steps quantize the actual nominal setting. For example, a 1000-mA request gives approximately 975 mA RMS. Low hold requests can still produce the minimum current-scale value (about 51 mA); disable removes drive.

Changing only the software to request 2000 mA would saturate the register at about 1.64 A with these resistors. Reaching 2 A RMS would require a hardware sense-resistor/design change and thermal qualification. Higher current needs adequate PCB/heatsink cooling; the full-scale setting is not a guaranteed continuous operating rating. Driver thermal protection remains active. Older firmware is limited to 1000 mA in the USB form until upgraded.

Reference: [Analog Devices TMC2209 Rev. 1.09](https://www.analog.com/media/en/technical-documentation/data-sheets/TMC2209_datasheet_rev1.09.pdf), sections 8 and 9, electrical characteristics and thermal guidance.

With **2.0.2**, run and hold requests persist while the motor is disabled even when the board has **USB power only**. TMC programming is deferred when necessary; **current pending** means the stored request has not yet been applied/verified in the driver. Every subsequent enable, including a move that enables automatically, must configure and verify the TMC first. USB-only driver fault 3 does not invalidate healthy encoder readings and does not imply that motor current is flowing.

The current panel reads back each successful USB/CAN write. The USB form shows unsaved edits separately from actual saved values. On a partial failure it reports the failure and preserves the requested draft; inspect saved values and retry after correcting the cause. **Read saved current** refreshes the drive fields. Settings are writable only while disabled.

## Flash a brand-new board

1. Disconnect motor power and connect board USB. Open the **Firmware** tab; no working firmware connection is needed.
2. Select the board USB port. If no ROM port appears, hold BOOT, press/release RESET, then release BOOT and refresh the port list.
3. Leave the bundled firmware folder selected, or choose an extracted matching CLMD package and click **Verify package**.
4. Select **New board — bootloader + partitions + application**. For an ordinary upgrade with the existing matching partition layout, choose **application only** instead.
5. Choose where device backups should be saved, then click **Flash firmware over USB**. The GUI closes its board connection, verifies all image hashes, backs up all 2 MB, writes the selected images and explicitly verifies the flash contents. A failed/incomplete backup prevents writing.
6. Keep USB connected until the result is shown. The board follows its saved startup hold setting (enabled by default in 2.0.3). Disable it, then connect under USB parameters to assign its unique CAN ID, configure polarity/limits and commission the mechanism.

The panel bundles a pinned source-only esptool 4.12.0 runtime, so an ESP-IDF installation or command prompt is unnecessary. The normal ROM bootloader must still be accessible. Flashing is over USB; CAN firmware updates are not implemented.

Full installation writes 0x0, 0x8000 and 0x10000 without deliberately erasing NVS. App-only writes 0x10000. The GUI does not write the merged factory image over the configuration area, automatically restore a move or home. Startup hold follows the saved setting. A fresh board starts with factory settings and optional joint feedback Off.
