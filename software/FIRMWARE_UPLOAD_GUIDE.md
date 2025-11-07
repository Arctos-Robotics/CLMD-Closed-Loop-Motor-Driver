# 📤 Firmware Upload Feature Guide

## Overview

The Firmware Upload panel allows you to flash new firmware to your ESP32-S3 device directly from the configuration tool GUI - no command line needed!

## Features

✅ **One-Click Upload** - Flash firmware with a single button click
✅ **Auto-Detection** - Automatically find firmware.bin in your project
✅ **Progress Tracking** - Real-time upload progress and logging
✅ **Port Selection** - Easy COM port selection with auto-refresh
✅ **Flash Erase** - Optional full flash erase for clean installs
✅ **Error Handling** - Clear error messages and troubleshooting

## Prerequisites

### 1. Install esptool
```bash
pip install esptool
```

This is automatically installed if you use:
```bash
pip install -r requirements.txt
```

### 2. Build Firmware
Before uploading, you need to build the firmware:
```bash
cd firmware
idf.py build
```

The firmware binary will be created at:
`firmware/build/firmware.bin`

## How to Use

### Step 1: Open Firmware Upload Panel
1. Launch the config tool: `python main.py`
2. Click on the "📤 Firmware Upload" tab

### Step 2: Select Firmware File

**Option A: Auto-Find (Recommended)**
1. Click "🔍 Auto-Find" button
2. Tool will automatically locate `firmware/build/firmware.bin`

**Option B: Manual Selection**
1. Click "Browse..." button
2. Navigate to your firmware binary file
3. Select the `.bin` file

### Step 3: Select COM Port
1. Choose your device's COM port from dropdown
2. Click "🔄 Refresh" if your port isn't listed
3. Port format: `COM3 - USB Serial Device`

### Step 4: Configure Options (Optional)
- ☑️ **Erase flash before upload**: Check this for major updates or if experiencing issues
  - Recommended for first-time flash
  - Recommended when changing partition tables
  - Not needed for regular firmware updates

### Step 5: Upload Firmware
1. **Disconnect** from device in Dashboard tab (if connected)
2. Click "📤 Upload Firmware" button
3. Wait for upload to complete (typically 30-60 seconds)
4. Device will automatically reset

### Step 6: Reconnect
1. Wait 5 seconds for device to boot
2. Go to Dashboard tab
3. Click "Connect"
4. Verify new firmware is running

## Upload Process

The upload follows these steps:

1. **Connecting** (10%) - Establish connection with ESP32 bootloader
2. **Erasing** (20%) - Erase flash if option selected
3. **Uploading** (30-90%) - Write firmware to flash memory
4. **Verifying** (95%) - Verify written data
5. **Resetting** (100%) - Reset device to run new firmware

## Troubleshooting

### Error: "esptool.py not found"
**Solution**: Install esptool
```bash
pip install esptool
```

### Error: "Failed to connect to ESP32"
**Solutions**:
1. Check USB cable connection
2. Try a different USB port
3. Hold BOOT button while clicking Upload
4. Check if another program is using the port
5. Try lower baud rate (edit firmware_upload_panel.py, change 921600 to 115200)

### Error: "Permission denied" (Linux/Mac)
**Solution**: Add user to dialout group
```bash
sudo usermod -a -G dialout $USER
# Log out and back in
```

Or use sudo:
```bash
sudo python main.py
```

### Error: "Firmware file not found"
**Solutions**:
1. Build firmware first: `cd firmware && idf.py build`
2. Check file path is correct
3. Use "Auto-Find" button

### Upload Stuck at "Connecting..."
**Solutions**:
1. Press and hold BOOT button on ESP32
2. Click Upload
3. Release BOOT button after "Connecting..." appears
4. Some boards enter bootloader automatically, others need manual BOOT

### Device Not Responding After Upload
**Solutions**:
1. Wait 10 seconds for boot
2. Press RESET button on board
3. Disconnect and reconnect USB
4. Check serial monitor for boot messages

## Advanced Usage

### Custom Firmware Locations
If your firmware is in a non-standard location:
1. Click "Browse..."
2. Navigate to your custom location
3. Select the `.bin` file

### Multiple Devices
To flash multiple devices:
1. Upload to first device
2. Disconnect USB
3. Connect second device
4. Refresh ports
5. Select new port
6. Upload again

### Partition Table Updates
If you modified partition table:
1. ☑️ Check "Erase flash before upload"
2. Upload firmware
3. Device will have clean flash with new partitions

## Technical Details

### Upload Parameters
- **Chip**: ESP32-S3
- **Baud Rate**: 921600 (fast upload)
- **Flash Mode**: DIO
- **Flash Frequency**: 80MHz
- **Flash Address**: 0x0 (start of flash)

### What Gets Flashed
The tool flashes the complete firmware image which includes:
- Bootloader
- Partition table
- Application firmware
- NVS (if included in build)

### Safety Features
- Automatic disconnect before upload
- Progress monitoring
- Error detection and reporting
- Automatic device reset after upload
- Cancel button for emergency stop

## Comparison: GUI vs Command Line

| Feature | GUI Tool | Command Line |
|---------|----------|--------------|
| Ease of Use | ⭐⭐⭐⭐⭐ | ⭐⭐ |
| Speed | Same | Same |
| Progress Display | ✅ Real-time | ❌ Limited |
| Error Messages | ✅ Clear | ⚠️ Technical |
| Port Selection | ✅ Dropdown | ❌ Manual |
| Auto-Find | ✅ Yes | ❌ No |
| Requires Terminal | ❌ No | ✅ Yes |

## Tips & Best Practices

1. **Always build before upload** - Ensure firmware is up-to-date
2. **Disconnect first** - Close serial connections before upload
3. **Use Auto-Find** - Faster than manual browsing
4. **Check logs** - Review upload log for any warnings
5. **Wait for reset** - Give device 5-10 seconds to boot
6. **Backup configs** - Save settings before major updates
7. **Test incrementally** - Upload and test after each change

## Integration with Development Workflow

### Typical Development Cycle
```
1. Edit firmware code
2. Build: idf.py build
3. Open config tool
4. Go to Firmware Upload tab
5. Click Auto-Find
6. Click Upload
7. Wait for completion
8. Reconnect and test
9. Repeat
```

### Quick Update Workflow
```
1. Keep config tool open
2. Edit and build firmware
3. Switch to Firmware Upload tab
4. Click Upload (file path remembered)
5. Test immediately
```

## FAQ

**Q: Do I need to disconnect before upload?**
A: Yes, the tool will disconnect automatically, but it's good practice to disconnect manually first.

**Q: Can I upload while motor is running?**
A: No, always stop motor and disconnect before upload.

**Q: Will upload erase my saved settings?**
A: No, unless you check "Erase flash". Settings in NVS are preserved during normal upload.

**Q: How long does upload take?**
A: Typically 30-60 seconds depending on firmware size and baud rate.

**Q: Can I use this with other ESP32 boards?**
A: This tool is configured for ESP32-S3. For other chips, modify the chip parameter in firmware_upload_panel.py.

**Q: What if upload fails halfway?**
A: Device may be in bootloader mode. Try uploading again or press RESET button.

## Support

If you encounter issues:
1. Check troubleshooting section above
2. Review upload log for error messages
3. Try command line upload to isolate issue:
   ```bash
   esptool.py --port COM3 --chip esp32s3 write_flash 0x0 firmware.bin
   ```
4. Check ESP32 forums for specific errors

## Future Enhancements

Planned features:
- [ ] Batch upload to multiple devices
- [ ] Firmware version detection
- [ ] Automatic build before upload
- [ ] OTA (Over-The-Air) updates via WiFi
- [ ] Firmware backup/restore
- [ ] Custom partition flashing

---

**Enjoy hassle-free firmware updates!** 🚀
