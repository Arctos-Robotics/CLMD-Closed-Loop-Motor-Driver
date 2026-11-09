# Build and modify CLMD hardware

Use the [PCB v3 export dated 2026-10-23](pcb-v3/) for the current editable design, fabrication files, assembly files and mechanical model. Fresh [front](../assets/board-front.png) and [rear](../assets/board-back.png) renders are generated from that October STEP model. The table below documents the September PCB v2 reference.

| File | Use |
| --- | --- |
| [EasyEDA project](pcb-v2/Design%20Files/ProPrj_Arctos1_2026-09-22.eprj2) | Edit schematic/PCB in a compatible EasyEDA version |
| [Schematic PDF](pcb-v2/Design%20Files/SCH_Schematic1_2026-09-22.pdf) | Inspect the circuit and connector pin numbers |
| [STEP model](pcb-v2/Design%20Files/3D_PCB1_2026-09-22.step.zip) | Check mechanical fit in CAD |
| [Front render](pcb-v2/Design%20Files/Front.png) / [back render](pcb-v2/Design%20Files/Back.png) | Locate parts and silkscreen |
| [Gerber archive](pcb-v2/Fab%20Files/Gerber_PCB1_2026-09-22.zip) | PCB fabrication |
| [BOM](pcb-v2/Fab%20Files/BOM_Board1_PCB1_2026-09-22.xlsx) | Component sourcing/assembly |
| [Pick-and-place](pcb-v2/Fab%20Files/PickAndPlace_PCB1_2026_09_22.xlsx) | Assembly placement |

## Revision provenance

The current October board model is marked CLMD **V1.5**; this publication uses CLMD **1.5**. CLMD 1.4 was the previous launch label. The October manufacturing export is **PCB v3**. The September PCB v2 schematic is labeled **V1.3** and remains a circuit reference. See [versions and compatibility](../docs/VERSIONS.md).

Use the connector labels and pin numbers from the applicable design files when assembling or wiring a board.

## Main circuit blocks

- ESP32-S3R8 controller with external 2 MB flash.
- TMC2209 drive stage with 0.12-ohm sense resistors.
- TLE5012BE1000 motor-shaft magnetic sensor.
- SN65HVD230DR CAN transceiver in this export.
- TPS54331 5 V buck and TLV75733 3.3 V regulator.
- USB-C data/power, MIN/MAX, CAN, motor and expansion connectors.

Confirm exact orderable parts in the BOM and schematic. In particular, the older export named a different CAN transceiver; substitutions need electrical review. The released firmware uses SpreadCycle, a maximum 1641 mA RMS request, and the documented PCB v2 GPIO mapping. Chip feature lists do not establish enabled firmware features or board operating ratings.

## Fabrication workflow

1. Open the original editable project in a compatible EasyEDA version. Check project integrity, schematic-to-PCB connectivity, DRC and mounting dimensions against your motor/magnet setup.
2. Inspect the Gerbers and drills in your manufacturer's viewer. Confirm actual layer stack, copper, board outline, mask, holes and placement orientation. Do not infer fabrication requirements from a render or choose a different layer count without updating the design.
3. Use the matching BOM and pick-and-place. Review substitutions, polarity, connector orientation, both-sided assembly and access to the rear encoder.
4. Review the power/thermal path for the intended supply, motor and duty cycle. Existing files do not establish a validated continuous current or full voltage range.
5. Assemble a pilot board and inspect it before ordering a batch. Check shorts, regulator rails and disabled ENN, then flash the matching 2 MB firmware via USB.
6. Verify motor encoder, input states and CAN responses before energizing a motor. Commission low-speed unloaded behavior before load/speed/thermal acceptance tests.

Use [WIRING.md](../docs/WIRING.md) and [FLASHING.md](../docs/FLASHING.md) for end-user setup. Record changes, board markings and acceptance results when publishing a variant.

Hardware uses [CERN-OHL-S-2.0](../LICENSES/CERN-OHL-S-2.0.txt), permitting personal and commercial manufacture under its source-access, notice and reciprocity terms. See [licensing](../LICENSING.md) and [ordering](../docs/ORDERING.md).

The STEP model is supplied as a lossless `.step.zip` download for browser-upload compatibility. Extract the ZIP before opening the STEP file in CAD.
