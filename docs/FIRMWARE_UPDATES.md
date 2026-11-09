# GitHub firmware updates for Arctos Studio

This defines the download layout for future Studio integration with the [CLMD repository](https://github.com/Arctos-Robotics/CLMD-Closed-Loop-Motor-Driver). For manual installation, see the [flashing guide](FLASHING.md).

## Published layout

| File | Purpose |
| --- | --- |
| `firmware/latest.json` | Stable channel: version, relative manifest path, manifest hash and firmware ZIP asset/hash |
| `firmware/releases/2.0.3/manifest.json` | Compatibility, image offsets, file hashes and corresponding-source reference |
| `firmware/releases/2.0.3/*.bin` | Versioned bootloader, partition table and application |
| GitHub Release asset `clmd-firmware-2.0.3.zip` | Small standalone package for Studio, with images, manifest, GPL text and dependency notices |
| Git tag `firmware-v2.0.3` | Corresponding source for that binary release; create this when publishing |

Channel paths are relative to the repository root. A Studio integration can resolve the default branch's `firmware/latest.json` and the matching GitHub Release asset.

## Download and flash sequence

1. Fetch the stable channel from the configured GitHub repository over HTTPS. Show the offered version to the user; do not flash merely because a new version exists.
2. Fetch the versioned manifest and check its SHA-256 against the channel. Confirm ESP32-S3, 2 MB flash, compatible board profile and image offsets.
3. Fetch the firmware ZIP asset and verify its hash. Extract into a temporary/cache folder using checked relative paths, then verify every package-file hash. Hashes detect corruption; they are not publisher signatures.
4. Reuse Studio's existing verified backup/program/readback path. Keep motor power/settings precautions and the user-selected app-only/full mode.
5. Reconnect/read back the firmware and saved settings. Keep the prior download/board backup available for deliberate rollback.

The update workflow downloads the release and installs it over USB. Studio owns the download and flashing integration.

## Publish a new firmware version

Build from the pinned SDK plus patch; run relevant tests and record bench scope. Add a new `firmware/releases/VERSION/` folder without replacing old versioned images. Generate a manifest with SHA-256 values and source reference, package a small ZIP including its license/notices, and publish the matching source tag and GitHub Release asset.

Publish the source/tag and firmware asset **before** advancing `firmware/latest.json`. Set channel hashes from the exact uploaded files. Keep source, binaries and notices accessible together for GPL compliance. Avoid distributing board-specific settings/backups.

The current firmware bundle is staged locally beside the repository ZIP for the initial upload. Its manifest is compatible with Studio's existing package verifier (`chip`, `flash_bytes`, `files`, and standard image names); the stable channel is the new future-download contract.
