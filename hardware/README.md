# Hardware Design Files

This directory contains all the hardware design files for the closed-loop stepper motor driver.

## Files Included

### Schematic
- **SCH_Schematic1.pdf** - Complete circuit diagram with all components and connections

### PCB Design
- **Arctos.eprj** - EasyEDA project file (editable source)
- **Layout.png** - PCB layout visualization

### Manufacturing Files
- **BOM_Board1_Schematic1.xlsx** - Bill of Materials with part numbers and quantities
- **PickAndPlace_PCB1.csv** - Pick and place file for automated assembly

### 3D Visualizations
See `docs/images/` for:
- 3D_View.png - Top 3D rendering
- Bottom_View.png - Bottom 3D rendering

## Specifications

### Board Dimensions
- Approximately 50mm × 50mm (verify from layout)
- 2-layer PCB (minimum)
- 1.6mm thickness recommended

### Key Components

| Component | Part Number | Package | Quantity |
|-----------|-------------|---------|----------|
| MCU | ESP32-S3R8 | QFN-56 | 1 |
| Motor Driver | TMC2209-LA-T | QFN-28 | 1 |
| Encoder | TLE5012BE1000 | PG-DSO-8 | 1 |
| Buck Converter | TPS54331DR | SOIC-8 | 1 |
| LDO Regulator | TLV75733PDBVR | SOT-23-5 | 1 |
| CAN Transceiver | TJA1050T-JSM | SOIC-8 | 1 |
| Flash Memory | W25Q16JVUUIQ | USON-8 | 1 |

See BOM file for complete component list with part numbers.

## Manufacturing

### PCB Fabrication

Recommended specifications:
- **Layers**: 2 (minimum), 4 (recommended for better power distribution)
- **Thickness**: 1.6mm
- **Copper Weight**: 1oz (35μm) minimum, 2oz recommended for power traces
- **Surface Finish**: ENIG or HASL
- **Solder Mask**: Green (or your preference)
- **Silkscreen**: White

### Assembly

1. **SMD Components**: Reflow soldering recommended
   - Stencil for solder paste application
   - Reflow profile for lead-free solder
   - Smallest components: 0402 resistors/capacitors

2. **Through-Hole Components**: Hand soldering
   - Connectors (USB-C, power, motor, CAN)
   - Programming headers

3. **Bottom Side**: TLE5012B encoder mounted on bottom

### Testing

After assembly:
1. Visual inspection for solder bridges
2. Continuity test for power rails
3. Power-on test (measure 5V and 3.3V rails)
4. Program ESP32-S3 via USB
5. Test encoder communication
6. Test motor driver with small motor

## Design Notes

### Power Supply
- Input: 6-28V DC (optimized for 12-24V)
- Buck converter: VIN → 5V (TPS54331)
- LDO regulator: 5V → 3.3V (TLV75733)
- Reverse polarity protection via P-channel MOSFET

### Motor Driver
- TMC2209 with 120mΩ sense resistors
- Up to 2A RMS motor current
- StealthChop2 for silent operation
- UART configuration interface

### Encoder
- TLE5012B on bottom side
- 15-bit resolution (0.011° per count)
- SPI interface at up to 8 MHz
- Requires magnet on motor shaft (2-4mm distance)

### Communication
- CAN bus with 120Ω termination
- USB Type-C with ESD protection
- UART for TMC2209 configuration

### Protection
- Fuse on power input (BSMD1812-200-30V)
- ESD protection on USB (LESD5Z5.0CT1G)
- Reverse polarity protection (DMG2305UX-7)

## Modifications

### Increasing Current Capacity
To support higher motor currents:
1. Replace sense resistors (R22, R23) with lower values
2. Ensure adequate cooling for TMC2209
3. Update firmware current settings

### Adding Features
- **External I/O**: Use U10 connector (8-pin)
- **Additional Sensors**: GPIO pins available on headers
- **Display**: I2C or SPI display can be connected

### Variants
Possible design variants:
- Different motor driver (TMC5160 for higher current)
- Different encoder (AS5047 for higher resolution)
- Integrated display
- Wireless communication (ESP32-S3 has WiFi/BT)

## Ordering PCBs

### Recommended Manufacturers
- JLCPCB (China) - Low cost, good quality
- PCBWay (China) - Good for prototypes
- OSH Park (USA) - High quality, slower
- Eurocircuits (Europe) - Professional grade

### Typical Costs
- PCB only (5 pcs): $10-30
- PCB + Assembly: $100-200 (depending on quantity)
- Components: $30-50 per board

### Lead Time
- PCB fabrication: 2-5 days
- Assembly: 3-7 days
- Shipping: 5-15 days (varies by location)

## Support

For hardware questions:
- Review [Hardware Description](../docs/hardware_description.md)
- Check schematic PDF for details
- Open an issue on GitHub

## License

Hardware design files are licensed under CERN-OHL-P v2 or MIT License (your choice).

You are free to:
- Manufacture boards for personal or commercial use
- Modify the design
- Distribute the design files

Please provide attribution to the original project.

---

**Note**: Always verify component availability and pricing before ordering. Some components may have long lead times or be out of stock.
