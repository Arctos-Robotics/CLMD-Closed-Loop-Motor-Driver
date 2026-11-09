# CLMD firmware

ESP32-S3 firmware **2.0.3** for CLMD 1.5. The preserved release manifest records the original PCB v2 target. See [versions and compatibility](../docs/VERSIONS.md) for the October hardware export.

| Path | Purpose |
| --- | --- |
| `main/` | Production C source |
| `tests/` | C host behavioral/model tests and SDK stubs |
| `patches/` | Required bounded I²C STOP patch for ESP-IDF 5.5.1 |
| `sdkconfig`, `sdkconfig.defaults` | Build configuration |
| `releases/2.0.3/` | USB-flashable images and integrity manifest |
| `latest.json` | Stable machine-readable update channel for future Studio integration |
| `releases/2.0.3/notices/` | Upstream licenses for components in the firmware build |

For end users, flash, configure and test in **Arctos Studio → Settings → CLMD Settings**. No separate GUI or Python host tools are included here. See [flashing](../docs/FLASHING.md) and [first setup](../docs/QUICK_START.md).

Developers: [build instructions](../docs/BUILD.md), [firmware behavior](../docs/FIRMWARE.md), [protocol](../docs/PROTOCOL.md) and [update contract](../docs/FIRMWARE_UPDATES.md).

Project firmware uses GPL-3.0-or-later; third-party notices and the Apache-2.0 SDK patch retain their respective terms.
