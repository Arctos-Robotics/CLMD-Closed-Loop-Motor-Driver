# Contributing

Hardware improvements, firmware fixes, integrations, clear tutorials and reproducible gearbox/load measurements are welcome.

| Change | Include |
| --- | --- |
| Hardware | Editable source, matching fabrication exports, revision note and validation |
| Firmware | Focused change, relevant C behavioral/failure tests and build results |
| Performance | Setup, raw data, independent measurement method and stated uncertainty |
| Documentation | Correct links and clear end-user steps in Arctos Studio |

Current source is in `firmware/main/`; C host tests are in `firmware/tests/`. See [BUILD.md](docs/BUILD.md). Preserve checked startup, the saved startup-hold option, explicit fault recovery, watchdogs, bounded homing, actual motor limits and encoder validity guards. Keep motor-only operation available without an external encoder. Preserve compatible NVS storage; do not erase saved settings on startup or ordinary upgrades.

Arctos Studio owns the setup/flashing/testing GUI and is distributed separately. Keep this repository focused on the board, firmware, assets and user documentation.

Contributions must be yours to license under the applicable [open-source terms](LICENSING.md). Retain notices and identify changes. Firmware contributions use GPL-3.0-or-later; hardware contributions use CERN-OHL-S-2.0. Third-party exceptions retain their own terms.

Record separately what was simulated, checked on disabled hardware and physically tested under load. Do not commit device backups, credentials, SDK/build trees or personal paths. New firmware releases need matching source/configuration, binaries, notices and hashes; update the stable channel only after the versioned release is published. See [update publishing](docs/FIRMWARE_UPDATES.md).
