# 📥 Installing ESP-IDF (ESP32 Development Framework)

## What is ESP-IDF?

ESP-IDF (Espressif IoT Development Framework) is the official development framework for ESP32 chips. You need it to build firmware for your ESP32-S3 board.

## Quick Answer

**You're seeing**: `'idf.py' is not recognized as an internal or external command`

**Solution**: Install ESP-IDF first, then build firmware.

---

## Windows Installation (Recommended Method)

### Step 1: Download Installer

1. Go to: **https://dl.espressif.com/dl/esp-idf/**
2. Download: **`esp-idf-tools-setup-online.exe`** (latest version, ~5MB)
   - Example: `esp-idf-tools-setup-online-5.1.2.exe`

### Step 2: Run Installer

1. **Double-click** the downloaded `.exe` file
2. Click "Yes" if Windows asks for permission
3. Choose **"Express Installation"** (recommended)
4. Select **ESP32-S3** in the chip selection
5. Click **"Install"**

### Step 3: Wait for Installation

- Takes **10-15 minutes**
- Downloads and installs:
  - Python 3.x
  - Git
  - CMake
  - Ninja build system
  - ESP32 toolchain
  - ESP-IDF framework

### Step 4: Verify Installation

1. Open **Start Menu**
2. Look for **"ESP-IDF 5.x CMD"** or **"ESP-IDF 5.x PowerShell"**
3. Click to open it
4. Run:
   ```bash
   idf.py --version
   ```
5. Should show:
   ```
   ESP-IDF v5.1.2
   ```

### Step 5: Build Firmware

1. In the **ESP-IDF CMD** window, navigate to firmware folder:
   ```bash
   cd C:\Users\Dell\Desktop\CLMD\firmware
   ```

2. Build:
   ```bash
   idf.py build
   ```

3. Wait 2-5 minutes for first build

4. Success! You'll see:
   ```
   Project build complete.
   ```

---

## Important Notes for Windows

### ⚠️ Always Use ESP-IDF CMD

**DON'T use regular CMD or PowerShell!**

Regular Windows terminals don't have ESP-IDF in their PATH.

**DO use**:
- "ESP-IDF 5.x CMD" from Start Menu ✅
- "ESP-IDF 5.x PowerShell" from Start Menu ✅

### Where is ESP-IDF Installed?

Default location:
```
C:\Espressif\frameworks\esp-idf-v5.x\
```

### Shortcuts Created

After installation, you'll have in Start Menu:
- **ESP-IDF 5.x CMD** - Command prompt with ESP-IDF
- **ESP-IDF 5.x PowerShell** - PowerShell with ESP-IDF
- **ESP-IDF Tools** - Configuration tools

---

## Linux Installation

### Ubuntu/Debian

```bash
# 1. Install prerequisites
sudo apt-get update
sudo apt-get install git wget flex bison gperf python3 python3-pip \
    python3-venv cmake ninja-build ccache libffi-dev libssl-dev \
    dfu-util libusb-1.0-0

# 2. Create ESP directory
mkdir -p ~/esp
cd ~/esp

# 3. Clone ESP-IDF
git clone --recursive https://github.com/espressif/esp-idf.git

# 4. Install ESP-IDF
cd ~/esp/esp-idf
./install.sh esp32s3

# 5. Setup environment (run every time you open terminal)
. $HOME/esp/esp-idf/export.sh

# 6. Verify
idf.py --version
```

### Make it Permanent (Optional)

Add to `~/.bashrc`:
```bash
echo 'alias get_idf=". $HOME/esp/esp-idf/export.sh"' >> ~/.bashrc
```

Then run `get_idf` whenever you need ESP-IDF.

---

## macOS Installation

### Using Homebrew

```bash
# 1. Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 2. Install prerequisites
brew install cmake ninja dfu-util

# 3. Create ESP directory
mkdir -p ~/esp
cd ~/esp

# 4. Clone ESP-IDF
git clone --recursive https://github.com/espressif/esp-idf.git

# 5. Install ESP-IDF
cd ~/esp/esp-idf
./install.sh esp32s3

# 6. Setup environment
. $HOME/esp/esp-idf/export.sh

# 7. Verify
idf.py --version
```

---

## Troubleshooting

### Issue: "Python not found"

**Windows**: Installer should install Python automatically. If not:
- Download Python 3.8+ from python.org
- Check "Add Python to PATH" during installation

**Linux/Mac**: Install Python 3:
```bash
sudo apt-get install python3 python3-pip  # Ubuntu/Debian
brew install python3                       # macOS
```

---

### Issue: "Git not found"

**Windows**: Installer should install Git. If not:
- Download from git-scm.com

**Linux**: 
```bash
sudo apt-get install git
```

**Mac**:
```bash
brew install git
```

---

### Issue: Installation fails

**Solution**: Try offline installer:
1. Download **`esp-idf-tools-setup-offline.exe`** (larger, ~500MB)
2. Includes all tools, no internet needed during install

---

### Issue: "Permission denied" (Linux/Mac)

**Solution**: Don't use sudo for ESP-IDF installation
```bash
# Wrong:
sudo ./install.sh

# Correct:
./install.sh esp32s3
```

---

## After Installation

### First Build

1. **Open ESP-IDF terminal**:
   - Windows: "ESP-IDF CMD" from Start Menu
   - Linux/Mac: Run `. ~/esp/esp-idf/export.sh` first

2. **Navigate to firmware**:
   ```bash
   cd /path/to/CLMD/firmware
   ```

3. **Build**:
   ```bash
   idf.py build
   ```

4. **Wait** 2-5 minutes

5. **Success!** You'll have `build/firmware.bin`

### Using the Config Tool

After ESP-IDF is installed:

1. Open config tool: `python main.py`
2. Go to "📤 Firmware Upload" tab
3. Click "🔨 Build Firmware"
4. **Important**: Use ESP-IDF CMD (Windows) or source export.sh first (Linux/Mac)
5. Run: `idf.py build`
6. After build, click "🔍 Auto-Find" in GUI
7. Upload!

---

## Quick Reference

### Windows Workflow

```
1. Open "ESP-IDF CMD" from Start Menu
2. cd C:\Users\Dell\Desktop\CLMD\firmware
3. idf.py build
4. Wait 2-5 minutes
5. Use config tool to upload
```

### Linux/Mac Workflow

```
1. Open terminal
2. . ~/esp/esp-idf/export.sh
3. cd /path/to/CLMD/firmware
4. idf.py build
5. Wait 2-5 minutes
6. Use config tool to upload
```

---

## Verification Checklist

After installation, verify:

- [ ] `idf.py --version` works
- [ ] Shows ESP-IDF v5.x.x
- [ ] Can navigate to firmware folder
- [ ] `idf.py build` starts building
- [ ] No "command not found" errors

---

## Getting Help

### Official Resources

- **ESP-IDF Documentation**: https://docs.espressif.com/projects/esp-idf/
- **Installation Guide**: https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/
- **ESP32 Forum**: https://esp32.com/

### Common Issues

- **Windows**: Always use "ESP-IDF CMD", not regular CMD
- **Linux/Mac**: Always run `. ~/esp/esp-idf/export.sh` first
- **All**: First build takes 2-5 minutes, be patient!

---

## Summary

```
┌─────────────────────────────────────────────────────────┐
│  ESP-IDF INSTALLATION SUMMARY                           │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  Windows:                                               │
│  1. Download esp-idf-tools-setup-online.exe            │
│  2. Run installer (Express Installation)                │
│  3. Wait 10-15 minutes                                  │
│  4. Use "ESP-IDF CMD" from Start Menu                   │
│                                                         │
│  Linux/Mac:                                             │
│  1. Clone ESP-IDF repository                            │
│  2. Run ./install.sh esp32s3                            │
│  3. Source export.sh before building                    │
│                                                         │
│  Verify:                                                │
│  idf.py --version                                       │
│                                                         │
│  Build:                                                 │
│  cd firmware && idf.py build                            │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

---

**Once ESP-IDF is installed, you'll be able to build and upload firmware!** 🚀

**Installation time**: 10-15 minutes
**First build time**: 2-5 minutes
**Worth it**: Absolutely! ✅
