# Order, assemble or modify CLMD

The maintainer's board price is **$20 USD**. Confirm quantity, supply/assembly variant, shipping, tax and optional output-encoder pricing with the supplier when ordering. This repository does not contain a live store link or a universal manufacturer quote.

## Order directly from a manufacturer

You can use the included files to request PCB fabrication and assembly. You may order manufacture for personal or commercial use under the [hardware license](../LICENSES/CERN-OHL-S-2.0.txt), including its source-access and notice obligations when conveying covered products.

For PCBWay assembly, use its quote workflow to submit the Gerbers, BOM and placement file from the October export together. [PCBWay assembly ordering guide](https://www.pcbway.com/helpcenter/pcb_assembly_ordering/How_to_Upload_PCB_Assembly_PCBA__Inquiry.html)

| Manufacturer input | Included file |
| --- | --- |
| PCB layers/outline/drills | [PCB v3 Gerber ZIP](../hardware/pcb-v3/Fab%20Files/Gerber_PCB1_2026-10-23.zip) |
| Component list | [PCB v3 BOM](../hardware/pcb-v3/Fab%20Files/BOM_Board1_PCB1_2026-10-23.xlsx) |
| Placement/orientation | [PCB v3 pick-and-place](../hardware/pcb-v3/Fab%20Files/PickAndPlace_PCB1_2026_10_23.xlsx) |
| Mechanical integration | [PCB v3 STEP model](../hardware/pcb-v3/Design%20Files/3D_PCB.step.zip) |
| Editable schematic and PCB layout | [PCB v3 EasyEDA project](../hardware/pcb-v3/Design%20Files/ProPrj_Arctos1_2026-10-23.epro2) |

1. Check the [hardware revision mapping](VERSIONS.md) against the board you intend to build. The October PCB v3 export is the current manufacturing set.
2. Upload the matching Gerber archive to your manufacturer's PCB quotation tool. Review the actual layer stack, board outline, drill sizes and copper specification from the design.
3. Request assembly using the matching BOM/placement files. If the manufacturer needs CSV rather than XLSX, export the sheets and verify units, side, coordinates and rotation in its placement preview.
4. Review every component substitution, polarity and both-sided placement. Ask the supplier about stencil, assembly, sourcing and programming charges before accepting the quote.
5. Assemble a pilot board and inspect rails/shorts before committing to a batch. See [hardware acceptance steps](../hardware/README.md).
6. Fit the motor shaft magnet and, if using output control, a separate output encoder and magnet. Flash via USB or the Studio Firmware tab, then commission the actual assembly.

## Modify and contribute

Edit the hardware in a compatible EasyEDA version and publish matching schematic/Gerber/BOM/placement files with a clear revision note. Firmware can be changed for your application under GPL-3.0-or-later. Retain notices and identify modifications; contributions use the applicable reciprocal open-source terms. See [CONTRIBUTING.md](../CONTRIBUTING.md).

Manufacturing and selling covered boards are allowed. Follow the hardware license's Complete Source/Source Location and notice requirements; distributed covered firmware follows the GPL. See [LICENSING.md](../LICENSING.md).

The STEP model is supplied as a lossless `.step.zip` download for browser-upload compatibility. Extract the ZIP before opening the STEP file in CAD.
