# Versions and compatibility

Product, hardware export and firmware versions identify different things.

| Identifier | What this snapshot contains | Authority |
| --- | --- | --- |
| CLMD 1.5 | Current public board version | V1.5 rear silkscreen in the October STEP model and latest experimenter identification |
| PCB v3 | Current manufacturing export, dated 2026-10-23 | [October design and manufacturing files](../hardware/pcb-v3/) |
| CLMD 1.4 | Previous launch presentation label | Superseded by the V1.5 identifier on the current model |
| PCB v2 / schematic V1.3 | September reference export and original 2.0.3 build label | Archived hardware and unchanged release manifest |
| CLMD Studio 2.0.3 | Stable firmware source and binary version | `CLMD_VERSION`, USB VERSION and release manifest |
| Arctos Studio 3.3 | Setup, testing and flashing interface | Distributed separately from this hardware/firmware repository |

The project video notes describe the 1.4 to 1.5 change as silkscreen-only. The current images are rendered from the included October STEP, which explicitly bears V1.5. This presentation update does not renumber firmware or rewrite historical build identifiers.

Use the October PCB v3 files together for manufacturing. The October EasyEDA archive includes the same schematic page records as September apart from editor metadata. The September PDF remains a schematic reference. Match the connector mapping, MCU, flash size and signals in [WIRING.md](WIRING.md) to the physical board. The stable 2.0.3 binaries and manifests retain their original bytes and PCB v2 build label.

## Selecting software

Use `firmware/` for stable corresponding source and `firmware/releases/2.0.3/` for supplied images and manifest. The newer diagnostic bench firmware in the development workspace is not substituted for this stable release. The [bench data](bench/) is identified separately.

The [USB/CAN contract](PROTOCOL.md) is the firmware interface. Arctos Studio supplies the setup, testing and flashing GUI separately.

The supplied binary targets ESP32-S3, 2 MB flash, DIO/80 MHz, bootloader at 0x0, partition table at 0x8000 and application at 0x10000. App-only upgrading assumes a compatible installed bootloader and partition layout. Other hardware/flash layouts require a reviewed build, not a renamed binary.
