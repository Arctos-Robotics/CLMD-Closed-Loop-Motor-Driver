# First setup in Arctos Studio

Use an Arctos Studio distribution that includes CLMD Settings. The GUI is supplied with Studio; this repository contains the board design and firmware.

## What you need

- An assembled CLMD matching the [documented hardware](VERSIONS.md).
- USB-C data cable and Arctos Studio with CLMD Settings.
- Compatible bipolar stepper with a correctly centered motor-shaft magnet.
- Suitable current-limited motor supply; see [power guidance](WIRING.md#power).
- Optional CAN adapter for a multi-axis bus; optional MT6701 at the gearbox output.

## 1. Connect and flash

Keep motor power off. Connect the board's USB port, open **Settings → CLMD Settings → Firmware**, and select that port. Choose the bundled firmware or an extracted [versioned firmware folder](../firmware/releases/2.0.3/). For a new board select the full bootloader/partitions/application installation; for a compatible upgrade use application only. Click **Flash firmware over USB**. [Detailed guide](FLASHING.md).

If the ROM port is missing, hold BOOT, tap RESET and release BOOT, then refresh the port list. No FTDI adapter or separate programmer is needed.

## 2. Inspect and configure

Connect the board under **USB parameters**. Disable before changing settings. Verify firmware version, encoder health, input state and saved current/node/polarity. USB supplies logic and encoder diagnostics; absence of motor power can produce a driver fault without indicating an encoder failure.

Turn the shaft slowly by hand and verify smooth motor-angle/travel changes in Studio's **Motor & Encoder Lab**. Match `fullsteps` to the motor: 200 for 1.8° or 400 for 0.9°. Select suitable run/hold current, with hold ≤ run. Configure motor/encoder signs and real limits. Do not copy one assembly's polarity blindly. Read back settings after applying.

Firmware 2.0.3 defaults to stationary hold at startup after health checks; it issues no move or home. Save **Enable stationary hold at startup** Off (`startup_enabled=0`) if disabled startup is required.

## 3. Power and make a small move

Power off before attaching coils/sensors. Secure the mechanism, connect motor power, then choose **Board USB** for direct testing or **CAN** for the configured bus. Use one motion-control session per board.

With healthy feedback/driver/inputs and mechanical clearance, run a small low-speed jog. Studio enables/checks the drive before the test; it does not silently clear latched faults. Confirm direction/travel and stop/disable behavior before increasing speed or installing the intended load. Commission homing/limits next. [User guide](USAGE.md).

## 4. Add output feedback

Start motor-only, then connect MT6701 and select **Monitor** with the true ratio/sign. Align while disabled and observe actual versus ideal output travel. Commission **Control** after those measurements are sound. [Joint guide](JOINT_ENCODER.md).
