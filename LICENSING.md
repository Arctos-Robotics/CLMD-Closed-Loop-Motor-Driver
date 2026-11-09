# Open-source licensing

CLMD is open source, including commercial use. Its reciprocal licenses keep distributed covered derivatives open.

| Material | License |
| --- | --- |
| CLMD firmware source, C tests and project build files | [GPL-3.0-or-later](LICENSE) |
| Hardware designs, schematic, PCB/fabrication/assembly files and CAD renders | [CERN-OHL-S-2.0](LICENSES/CERN-OHL-S-2.0.txt) |
| Project documentation and other project-owned assets | GPL-3.0-or-later |
| ESP-IDF patch | Apache-2.0, [patch license](firmware/patches/LICENSE) |
| Third-party code linked into firmware | Its original terms; [notices](firmware/releases/2.0.3/notices/) |

## Commercial use and reciprocity

- Personal, educational and commercial use are permitted. No commercial-use fee or separate permission is imposed by these licenses.
- You may manufacture and sell boards. When conveying covered hardware/products, provide Complete Source or notify recipients of its Source Location, and follow CERN-OHL-S's notices and reciprocal obligations.
- When conveying covered firmware binaries, provide access to Corresponding Source as required by the GPL. Distributed covered modifications retain the applicable reciprocal license and modification notices.
- Private use does not automatically require publishing every internal change. The complete license texts determine the obligations for the particular distribution.
- These licenses do not give trademark rights or imply endorsement. Third-party works retain their own licenses; a directory-wide notice does not replace an upstream notice.

The GPL reference is [SPDX GPL-3.0-or-later](https://spdx.org/licenses/GPL-3.0-or-later.html). The hardware reference is the [Open Hardware Repository](https://ohwr.org/licences/). The root LICENSE contains the complete GPL text; both full license texts are included.

## Notices and earlier copies

Copyright (c) 2025 Closed-Loop Stepper Motor Driver Project; copyright (c) 2026 CLMD contributors. Existing applicable attribution is retained. This publication replaces the proposed noncommercial policy with open-source terms; no noncommercial restriction is added. Earlier license grants to recipients are not withdrawn. The [previous published license and attribution](LICENSES/LEGACY-LICENSE.txt) are retained verbatim for reference to earlier material.

Firmware binaries are unchanged from the verified 2.0.3 build. Publication organization and licensing notices have changed. Source, configuration, pinned SDK information and the required SDK patch are included or identified so the firmware can be rebuilt. GitHub binary releases must identify the corresponding source tag and keep it available alongside the download. See [publishing](PUBLISHING.md).
