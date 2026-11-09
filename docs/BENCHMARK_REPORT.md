# CLMD benchmark report template

Copy this file for a measured result. Replace every field; omit tests not performed. Keep raw data beside the report.

## Test setup

| Field | Recorded value |
| --- | --- |
| Date / tester | |
| Product and PCB identifier | |
| Firmware version / application SHA-256 | |
| Motor model / step angle / winding connection | |
| Supply and measured board voltage | |
| Run / hold current requests | |
| Gearbox ratio / output mechanism | |
| Applied load / inertia | |
| Cooling / ambient temperature | |
| Encoder and magnet mounting | |
| Host / CAN adapter / bitrate | |
| Configuration JSON / raw log paths | |
| Independent instrument / uncertainty | |

## Procedure

Describe motion distance, direction, speed/acceleration, approach direction, repetitions, settling criterion, runtime and logging rate. State whether output feedback is Off, Monitor or Control. For comparisons, record the same setup for the other driver and its firmware/settings.

## Results

| Metric | Measured result | Samples / duration | Conditions |
| --- | --- | --- | --- |
| Mean signed positioning error | | | |
| Maximum absolute error | | | |
| Repeatability spread (define statistic) | | | |
| Settling time (define threshold) | | | |
| Highest passing loaded speed | | | |
| Board / motor steady temperature | | | |
| Reversal dead travel / correction | | | |
| Observed fault response | | | |

## Interpretation

State measurement uncertainty, failures, excluded samples and the range actually tested. Distinguish independent measurements from controller feedback, and physical measurements from simulation. Link raw data and an identifiable setup photo/video where available.
