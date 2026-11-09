# Flash firmware from Arctos Studio

Use **Settings → CLMD Settings → Firmware**. Studio supplies the flash runtime; there are no separate Python tools or testing GUIs to install from this repository.

## Install or upgrade

1. Secure the mechanism and disconnect motor power during initial flashing. Connect a USB-C data cable to the CLMD board.
2. Select the **board's USB port**, not the CANable adapter port. If needed hold BOOT, press/release RESET, release BOOT and refresh ports.
3. Use Studio's bundled package, or choose an extracted matching versioned firmware folder. The repository includes [2.0.3 images and manifest](../firmware/releases/2.0.3/).
4. Click **Verify package** and select the installation mode below.
5. Choose a private backup location and click **Flash firmware over USB**. Keep USB connected until Studio reports completion.

| Mode | Writes | Use |
| --- | --- | --- |
| Application only | `firmware.bin` at 0x10000 | Existing compatible bootloader/partition layout |
| New board / full installation | `bootloader.bin` at 0x0, `partition-table.bin` at 0x8000, `firmware.bin` at 0x10000 | Fresh board or confirmed matching full installation |

The panel verifies file hashes, backs up all 2 MB, programs and explicitly reads back the image. A failed/incomplete backup prevents writing. Separate-image flashing does not deliberately erase NVS. App-only mode assumes the installed partition layout is compatible; use the correct hardware/flash variant.

No FTDI adapter, external programmer or ESP-IDF installation is needed for the Studio workflow. Flashing is through the board's native USB; CAN flashing and wireless board OTA are not implemented.

## After flashing

Reconnect under **USB parameters**, disable the drive, and read settings/diagnostics. Confirm version, node ID, current, polarity and valid configuration. Firmware 2.0.3 defaults to stationary hold after health checks; no move/home runs at boot. Save `startup_enabled=0` while disabled if needed.

Commission current, direction, limits and homing on the actual mechanism. Flashing does not establish mechanical compatibility or output-loop accuracy.

## Updates from GitHub

The repository includes a stable [firmware/latest.json](../firmware/latest.json), versioned manifest and a separate small firmware download for the next Studio integration. **The online GitHub download option will be wired into Studio after the repository URL is supplied.** It is not active just because these files exist.

Once integrated, Studio will fetch the selected release, verify it and use the same backup-first USB flash path. Users will download only the firmware update package rather than the entire hardware repository. See the [update contract](FIRMWARE_UPDATES.md).

## Recovery and rollback

Close other port owners and try the BOOT/RESET sequence if connection fails. Keep board-specific full backups private; never upload them. Restore a full backup only to the same board using a deliberate recovery process. Before downgrading to firmware older than 2.0.1, save run/hold requests ≤1000 mA because older validators reject higher values. Recheck saved settings and startup behavior after rollback. [Troubleshooting](TROUBLESHOOTING.md).
