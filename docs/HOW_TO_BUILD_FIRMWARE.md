# 🔨 How to Build Firmware

## Quick Answer

You need to **build the firmware** before you can upload it. The `firmware.bin` file is created during the build process.

## Step-by-Step Guide

### Option 1: Using the Config Tool (Easiest)

1. **Open Config Tool**:
   ```bash
   cd config_tool
   python main.py
   ```

2. **Go to "📤 Firmware Upload" tab**

3. **Click "🔨 Build Firmware" button**
   - This opens a terminal in the firmware folder
   - You'll see the command prompt ready

4. **In the opened terminal, run**:
   ```bash
   idf.py build
   ```

5. **Wait for build to complete** (2-5 minutes first time, 30-60 seconds after)

6. **Back in Config Tool, click "🔍 Auto-Find"**
   - It will now find `firmware/build/firmware.bin`

7. **Upload!**

---

### Option 2: Manual Terminal (Traditional)

1. **Open terminal/command prompt**

2. **Navigate to firmware folder**:
   ```bash
   cd firmware
   ```

3. **Build firmware**:
   ```bash
   idf.py build
   ```

4. **Wait for completion**

5. **Verify firmware.bin exists**:
   ```bash
   # Windows
   dir build\firmware.bin
   
   # Linux/Mac
   ls -lh build/firmware.bin
   ```

6. **Now you can upload via GUI or command line**

---

## Prerequisites

### ESP-IDF Must Be Installed

Before you can build, you need ESP-IDF (Espressif IoT Development Framework):

#### Windows
1. Download ESP-IDF installer: https://dl.espressif.com/dl/esp-idf/
2. Run installer and follow prompts
3. It will install:
   - ESP-IDF framework
   - Python
   - CMake
   - Ninja build system
   - Toolchain

#### Linux/Mac
```bash
# Install prerequisites
sudo apt-get install git wget flex bison gperf python3 python3-pip python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0

# Clone ESP-IDF
mkdir -p ~/esp
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git

# Install ESP-IDF
cd ~/esp/esp-idf
./install.sh esp32s3

# Setup environment (add to ~/.bashrc)
. $HOME/esp/esp-idf/export.sh
```

### Verify ESP-IDF Installation

```bash
idf.py --version
```

Should show something like:
```
ESP-IDF v5.1.2
```

---

## Build Process Explained

### What Happens During Build

1. **Configuration** - Reads `sdkconfig` and `sdkconfig.defaults`
2. **CMake Generation** - Creates build files
3. **Compilation** - Compiles all `.c` files to `.o` objects
4. **Linking** - Links objects into ELF executable
5. **Binary Generation** - Creates flashable `.bin` files

### Build Output Location

After successful build, you'll find:

```
firmware/build/
├── firmware.bin          ← Main firmware (this is what you upload!)
├── firmware.elf          ← Debug symbols
├── bootloader/
│   └── bootloader.bin    ← ESP32 bootloader
├── partition_table/
│   └── partition-table.bin
└── ... (other build artifacts)
```

### Build Time

- **First build**: 2-5 minutes (compiles everything)
- **Incremental builds**: 30-60 seconds (only changed files)
- **Clean build**: 2-5 minutes (after `idf.py fullclean`)

---

## Common Build Issues

### Issue: "idf.py: command not found"

**Cause**: ESP-IDF not installed or environment not set up

**Solution**:
```bash
# Windows: Run "ESP-IDF Command Prompt" from Start Menu

# Linux/Mac: Source the export script
. $HOME/esp/esp-idf/export.sh
```

---

### Issue: "CMake Error: Could not find toolchain"

**Cause**: ESP-IDF toolchain not installed

**Solution**:
```bash
cd $IDF_PATH
./install.sh esp32s3
```

---

### Issue: "Python module not found"

**Cause**: Missing Python dependencies

**Solution**:
```bash
pip install -r $IDF_PATH/requirements.txt
```

---

### Issue: Build fails with errors

**Solution**: Check error messages, common fixes:
1. Clean build: `idf.py fullclean && idf.py build`
2. Update ESP-IDF: `cd $IDF_PATH && git pull`
3. Reinstall tools: `./install.sh esp32s3`

---

## Build Configuration

### Change Build Settings

```bash
idf.py menuconfig
```

This opens a configuration menu where you can:
- Change partition table
- Enable/disable features
- Adjust memory settings
- Configure WiFi/Bluetooth
- Set optimization level

### Quick Config Changes

Edit `firmware/sdkconfig.defaults` for permanent changes:
```ini
CONFIG_ESPTOOLPY_FLASHSIZE_8MB=y
CONFIG_PARTITION_TABLE_CUSTOM=y
CONFIG_FREERTOS_HZ=1000
```

---

## Build Commands Reference

```bash
# Standard build
idf.py build

# Clean build (removes all build artifacts)
idf.py fullclean

# Build and flash
idf.py build flash

# Build, flash, and monitor
idf.py build flash monitor

# Only flash (no build)
idf.py flash

# Monitor serial output
idf.py monitor

# Set target chip (one-time)
idf.py set-target esp32s3

# Show build size
idf.py size

# Show partition table
idf.py partition-table
```

---

## After Building

Once build completes successfully:

1. **Verify firmware.bin exists**:
   ```bash
   ls -lh build/firmware.bin
   ```

2. **Check file size** (should be 500KB - 2MB):
   ```bash
   # Windows
   dir build\firmware.bin
   
   # Linux/Mac
   ls -lh build/firmware.bin
   ```

3. **Upload via Config Tool**:
   - Open config tool
   - Go to "📤 Firmware Upload" tab
   - Click "🔍 Auto-Find"
   - Click "📤 Upload Firmware"

4. **Or upload via command line**:
   ```bash
   idf.py -p COM3 flash
   ```

---

## Quick Reference Card

```
┌─────────────────────────────────────────────────────────┐
│  FIRMWARE BUILD QUICK REFERENCE                         │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  1. Open terminal in firmware folder                    │
│     cd firmware                                         │
│                                                         │
│  2. Build firmware                                      │
│     idf.py build                                        │
│                                                         │
│  3. Wait 2-5 minutes (first time)                       │
│                                                         │
│  4. Check for success message:                          │
│     "Project build complete"                            │
│                                                         │
│  5. Firmware location:                                  │
│     firmware/build/firmware.bin                         │
│                                                         │
│  6. Upload via GUI or:                                  │
│     idf.py -p COM3 flash                                │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

---

## Troubleshooting Checklist

- [ ] ESP-IDF installed?
- [ ] Environment variables set? (run export.sh)
- [ ] In firmware folder? (cd firmware)
- [ ] Toolchain installed? (./install.sh esp32s3)
- [ ] Python dependencies? (pip install -r requirements.txt)
- [ ] Clean build tried? (idf.py fullclean && idf.py build)

---

## Need Help?

1. **Check ESP-IDF docs**: https://docs.espressif.com/projects/esp-idf/
2. **ESP32 forum**: https://esp32.com/
3. **GitHub issues**: https://github.com/espressif/esp-idf/issues

---

**Remember**: You only need to build once, then you can upload multiple times without rebuilding (unless you change the code)!

Happy building! 🔨
