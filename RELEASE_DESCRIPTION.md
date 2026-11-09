# CLMD 1.5 — PCB v3 and firmware 2.0.3

CLMD is an open-source stepper driver with motor position feedback and an optional second encoder at the gearbox output. It supports USB setup and flashing in Arctos Studio and CAN for connecting multiple axes.

- The **October PCB v3** set includes the editable EasyEDA project, STEP model, Gerbers, BOM and pick-and-place. [Hardware files](hardware/pcb-v3/)
- The **public 2.0.3 firmware** download includes the ESP32-S3 application, bootloader, partition table, manifest and notices. Its corresponding source is under `firmware/` at the `firmware-v2.0.3` tag. The manifest retains its original PCB v2 label. [Flashing guide](docs/FLASHING.md) · [Versions and compatibility](docs/VERSIONS.md)
- The **video bench data** covers open loop, motor feedback, gearbox output measurement and gearbox output feedback. With the output loop enabled, the joint encoder measured 0.014° RMS settled error versus 0.065° with motor feedback alone on this setup. The bench used a separate diagnostic build. [Method and CSV files](docs/bench/)

Start with the [quick-start guide](docs/QUICK_START.md) or the [PCB ordering guide](docs/ORDERING.md). See [licensing](LICENSING.md) for hardware, firmware and third-party notices.
