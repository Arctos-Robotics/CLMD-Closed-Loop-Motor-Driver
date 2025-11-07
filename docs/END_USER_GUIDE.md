# 📦 Closed-Loop Stepper Driver - End User Guide

## For Users Who Just Want to Use the Tool

**Good news!** You don't need to install ESP-IDF or any development tools!

---

## 📥 What You Get

### Option 1: Pre-Packaged Release (Recommended)

Download `ClosedLoopDriver_Release.zip` which includes:

```
ClosedLoopDriver_Release/
├── ClosedLoopDriver.exe          ← Just run this! ⭐
├── firmware/
│   ├── firmware.bin               ← Pre-built firmware
│   ├── bootloader.bin
│   └── partition-table.bin
├── docs/                          ← User manuals
└── README.txt                     ← Quick start guide
```

**No Python, no ESP-IDF, no installation needed!**

### Option 2: Source Code (For Developers)

If you want to modify the firmware or tool:
- Full source code
- Requires ESP-IDF for firmware development
- Requires Python for config tool development

---

## 🚀 Quick Start (End Users)

### Step 1: Download Release Package

1. Download `ClosedLoopDriver_Release.zip`
2. Extract to any folder (e.g., `C:\ClosedLoopDriver\`)
3. That's it!

### Step 2: Connect Hardware

1. Connect ESP32-S3 board via USB
2. Windows installs driver automatically
3. Note the COM port (e.g., COM3)

### Step 3: Upload Firmware (One-Time)

1. **Run `ClosedLoopDriver.exe`**
2. Go to **"📤 Firmware Upload"** tab
3. Click **"Browse"** → Select `firmware/firmware.bin`
4. Select your **COM port**
5. Click **"📤 Upload Firmware"**
6. Wait 30 seconds
7. Done! ✅

### Step 4: Use the Tool

1. Go to **Dashboard** tab
2. Select COM port
3. Click **"Connect"**
4. Enable motor and test!

---

## 💾 What's Required?

### For End Users (Just Using the Tool)

**Required**:
- ✅ Windows 10/11 (64-bit)
- ✅ USB cable
- ✅ ESP32-S3 board with firmware

**NOT Required**:
- ❌ Python installation
- ❌ ESP-IDF
- ❌ Development tools
- ❌ Command line knowledge

### For Developers (Modifying Firmware)

**Required**:
- ✅ ESP-IDF (for building firmware)
- ✅ Python 3.7+ (for config tool)
- ✅ Development knowledge

---

## 📦 Distribution Options

### Option A: Single Executable (Easiest)

**What**: One `.exe` file + firmware binaries
**Size**: ~50-100 MB
**Pros**: 
- No installation
- Just double-click to run
- Works on any Windows PC

**How to Create**:
```bash
# Run this once:
package_config_tool.bat
```

This creates `ClosedLoopDriver.exe` - distribute this + firmware folder.

### Option B: Python Package (Smaller)

**What**: Python scripts + requirements.txt
**Size**: ~1 MB
**Pros**: Smaller download
**Cons**: Users need Python installed

**How to Use**:
```bash
pip install -r requirements.txt
python main.py
```

### Option C: Complete Release Package (Best)

**What**: Everything bundled
**Size**: ~50-100 MB
**Includes**:
- Standalone .exe
- Pre-built firmware
- Documentation
- Installer script

**How to Create**:
```bash
python build_release.py
```

This creates `ClosedLoopDriver_Release.zip` - ready to distribute!

---

## 🎯 Recommended Distribution Method

### For GitHub Release

1. **Build release package**:
   ```bash
   python build_release.py
   ```

2. **Upload to GitHub**:
   - Create new release
   - Upload `ClosedLoopDriver_Release.zip`
   - Add release notes

3. **Users download and extract**:
   - No installation needed
   - Just run `ClosedLoopDriver.exe`

### For Website/Direct Distribution

1. **Create standalone exe**:
   ```bash
   package_config_tool.bat
   ```

2. **Package with firmware**:
   ```
   ClosedLoopDriver/
   ├── ClosedLoopDriver.exe
   ├── firmware/
   │   └── firmware.bin
   └── README.txt
   ```

3. **Zip and distribute**

---

## 📊 File Sizes

| Package Type | Size | Contents |
|--------------|------|----------|
| Standalone .exe | ~50-100 MB | Config tool only |
| Firmware binaries | ~500 KB | Pre-built firmware |
| Complete package | ~50-100 MB | Everything |
| Source code | ~5 MB | For developers |

---

## 🔧 Creating the Standalone Package

### Method 1: Automated (Recommended)

```bash
# Install PyInstaller (one-time)
pip install pyinstaller

# Run packaging script
package_config_tool.bat

# Result: ClosedLoopDriver.exe created
```

### Method 2: Manual

```bash
cd config_tool

# Install PyInstaller
pip install pyinstaller

# Create executable
pyinstaller --onefile --windowed --name "ClosedLoopDriver" main.py

# Find executable in dist/ folder
```

### Method 3: Complete Release

```bash
# Build everything
python build_release.py

# Result: ClosedLoopDriver_Release.zip created
```

---

## 📝 What to Include in Distribution

### Minimum Package

```
ClosedLoopDriver/
├── ClosedLoopDriver.exe       ← Standalone tool
├── firmware.bin                ← Pre-built firmware
└── README.txt                  ← Quick start guide
```

### Complete Package

```
ClosedLoopDriver_Release/
├── ClosedLoopDriver.exe       ← Standalone tool
├── firmware/
│   ├── firmware.bin           ← Main firmware
│   ├── bootloader.bin         ← Bootloader
│   └── partition-table.bin    ← Partition table
├── docs/
│   ├── UserGuide.pdf          ← Complete manual
│   ├── Troubleshooting.pdf    ← Help guide
│   └── PID_Tuning.pdf         ← Tuning guide
├── README.txt                  ← Quick start
└── install.bat                 ← Optional installer
```

---

## 🎓 For Different User Types

### Hobbyist/End User

**Give them**:
- `ClosedLoopDriver.exe`
- `firmware.bin`
- Simple README

**They need**:
- Just double-click and use
- No technical knowledge required

### System Integrator

**Give them**:
- Complete release package
- Full documentation
- Example configurations

**They need**:
- Professional documentation
- Configuration examples
- Support information

### Developer/OEM

**Give them**:
- Source code
- Build instructions
- API documentation

**They need**:
- Full source access
- Development environment setup
- Customization guides

---

## ✅ Checklist for Distribution

Before distributing, ensure:

- [ ] Firmware builds successfully
- [ ] Config tool packages as .exe
- [ ] .exe runs on clean Windows PC (no Python)
- [ ] Firmware upload works
- [ ] All features tested
- [ ] Documentation complete
- [ ] README.txt included
- [ ] Version number set
- [ ] License file included
- [ ] Contact/support info provided

---

## 🚀 Quick Distribution Workflow

```bash
# 1. Build firmware
cd firmware
idf.py build

# 2. Package config tool
cd ..
package_config_tool.bat

# 3. Create release package
python build_release.py

# 4. Test on clean PC
# - Copy ClosedLoopDriver_Release.zip to test PC
# - Extract and run ClosedLoopDriver.exe
# - Verify it works without Python/ESP-IDF

# 5. Distribute
# - Upload to GitHub releases
# - Or host on website
# - Or send directly to users
```

---

## 💡 Pro Tips

1. **Test on Clean PC**: Always test the package on a PC without Python/ESP-IDF
2. **Include Version**: Add version number to filename (e.g., `ClosedLoopDriver_v1.0.exe`)
3. **Sign Executable**: Consider code signing to avoid Windows warnings
4. **Compress**: Use 7-Zip for smaller download size
5. **Checksum**: Provide SHA256 checksum for verification

---

## 📞 Support for End Users

Users should NOT need to:
- Install Python
- Install ESP-IDF
- Use command line
- Know programming

They ONLY need to:
- Download .exe
- Run it
- Upload firmware
- Use the tool

**That's it!** 🎉

---

## Summary

**For End Users**: Just download and run `ClosedLoopDriver.exe`
**For Developers**: Full source code available
**For Distribution**: Use `build_release.py` to create complete package

**No ESP-IDF or Python required for end users!** ✅
