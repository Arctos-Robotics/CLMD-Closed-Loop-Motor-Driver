# Build the firmware

End users configure, test and flash in Arctos Studio. These instructions are for firmware contributors.

## ESP-IDF build

Use **ESP-IDF v5.5.1**, target ESP32-S3, with its recursively pinned submodules. The included configuration selects 2 MB flash, DIO at 80 MHz and the application at 0x10000. Install/export the SDK using [Espressif's guide](https://docs.espressif.com/projects/esp-idf/en/v5.5.1/esp32s3/get-started/index.html).

Apply the required SDK patch from the SDK checkout, substituting the absolute path to this repository:

```sh
git apply --check /path/to/CLMD/firmware/patches/esp-idf-5.5.1-i2c-bounded-stop.patch
git apply /path/to/CLMD/firmware/patches/esp-idf-5.5.1-i2c-bounded-stop.patch
```

PowerShell paths can be quoted Windows paths. Apply the patch once to the supported 5.5.1 source. If `git apply --check` rejects it, determine whether it is already applied or the SDK differs; do not force an unknown patch. The correction bounds a NACK/STOP bus-busy wait to about 20 ms so a faulty I²C slave cannot indefinitely hang that path. It is part of the corresponding build source, not an optional speed tweak.

From an exported ESP-IDF shell at the repository root:

```sh
cd firmware
idf.py build
```

ESP-IDF uses its own Python/build tools; no custom Python host programs are distributed in this repository. The application output is `firmware/build/clmd_studio.bin`, with bootloader and partition output in that build tree. Package them under the public image names documented in [FIRMWARE_UPDATES.md](FIRMWARE_UPDATES.md).

## C host tests

The production core, current model, joint controller and encoder tests can run without ESP-IDF. With CMake and a host C compiler, from the repository root:

```sh
cmake -S firmware/tests -B build/host-tests -DCMAKE_BUILD_TYPE=Debug
cmake --build build/host-tests
ctest --test-dir build/host-tests --output-on-failure
```

The suite covers controller faults/homing/limits, current quantization, simulated 24:1 gearbox reversals, encoder decoding and mocked I²C transactions/failures. GUI testing belongs to Arctos Studio's repository. The board's bench verification records are in [verification/](verification/).

## Source and releases

The source/configuration matches the supplied 2.0.3 build; no firmware behavior was changed by repository cleanup. Source attribution and license notices have been added around the distribution. The external SDK is identified by its exact tag and required patch. ESP-IDF/FreeRTOS notices are retained in [firmware/releases/2.0.3/notices/](../firmware/releases/2.0.3/notices/).

New releases must publish the matching source tag, image hashes, update metadata and applicable notices. A successful host test does not certify loaded hardware performance. See [publishing](../PUBLISHING.md) and [update contract](FIRMWARE_UPDATES.md).
