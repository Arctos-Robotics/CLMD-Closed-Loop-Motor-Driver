# Frequently asked questions

## Can I inspect and modify the hardware?

Yes. The snapshot includes the editable EasyEDA project, schematic, fabrication outputs, assembly files and a STEP model, alongside firmware and tools. See [hardware provenance](../hardware/README.md) and [licensing](../LICENSING.md). The original hardware revision labels are retained.

## Do I need an external encoder?

The on-board TLE5012B motor encoder and a suitable motor-shaft magnet are required. An MT6701 output encoder is optional. Motor-only mode is the default.

## Can USB alone run the motor?

No. USB powers logic/encoder diagnostics. Motor movement needs the separate motor supply.

## Which motors can I use?

Use a compatible two-phase bipolar stepper and a mechanical/magnet setup that lets the motor encoder measure its shaft. Current requests are limited to 50–1641 mA RMS in this firmware. Set 200 or 400 full steps per turn to match the motor. Compatibility and useful speed/torque must be checked for the actual winding, supply, load and cooling; a NEMA frame size alone does not establish it.

## Does it support STEP/DIR inputs, G-code or Wi-Fi control?

The documented public control path is native CAN, with USB configuration/diagnostics and native-handler injection. The on-board STEP/DIR nets connect the MCU to the TMC2209; they do not establish external pulse-input support. This firmware does not expose a documented G-code, Wi-Fi or Bluetooth control service.

## Is this FOC or a torque-controlled servo?

The released position controller uses encoder error, ramps and braking to command the TMC2209 internal velocity generator. It does not implement a field-oriented current loop or a calibrated torque-control interface.

## Is it silent StealthChop operation?

The TMC2209 has multiple capabilities, but the current release configures SpreadCycle at 256 subdivision. The repository describes the shipped configuration rather than advertising all chip features as enabled.

## Is 1200 RPM guaranteed? Is the accuracy ±0.1°?

1200 RPM is a configurable firmware ceiling. 0.1° is the default optional output settling tolerance. Neither is an independently measured product guarantee. See [performance and evidence](PERFORMANCE.md).

## Does the output encoder remove backlash?

It exposes output travel and permits a position loop that can command extra motor travel to take up play. The release includes a simulated 24:1 backlash/reversal test and a [recorded 45:1 printed-gearbox experiment](bench/). In the physical experiment, output-encoder endpoint RMS error decreased from 0.0648° to 0.0142° with output feedback. Independent measurements are still needed to establish absolute accuracy on a particular gearbox.

## Can I replace another MKS-style CAN driver directly?

Review the [implemented protocol subset](PROTOCOL.md). Matching several opcodes does not establish complete drop-in compatibility. Arctos Studio supplies its own CLMD integration; validate your host's commands, scaling, heartbeat and fault handling.

## Does it remember position after power loss?

Configuration is persistent. Multi-turn position/reference is not reconstructed after power loss from a single-turn sensor. Home a mechanism that needs an absolute multi-turn reference.

## Can I build and sell my own boards?

Yes. Hardware uses CERN-OHL-S-2.0 and firmware uses GPL-3.0-or-later. Commercial use and sales are permitted with the applicable source-sharing, notices and reciprocal terms. See [LICENSE](../LICENSE) and [LICENSING.md](../LICENSING.md). Check the hardware revision mapping and component substitutions before fabrication.

## Where can I buy CLMD, and what does it cost?

The maintainer supplies a $20 USD driver-board price. Confirm quantity, shipping/tax and optional-encoder pricing when ordering. Use the [ordering guide](ORDERING.md) to request manufacture from the included files; no live shop link is supplied.

## Do I need Python files or a separate testing GUI?

No. Install Arctos Studio with CLMD Settings for setup, tests and flashing. This repository contains the hardware, C firmware, assets and documentation. Developers build with ESP-IDF, which has its own toolchain dependencies.
