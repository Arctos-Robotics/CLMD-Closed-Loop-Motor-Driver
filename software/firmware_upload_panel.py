from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QGroupBox,
                             QLabel, QPushButton, QTextEdit, QFileDialog,
                             QProgressBar, QComboBox, QLineEdit)
from PyQt5.QtCore import QThread, pyqtSignal, Qt
from PyQt5.QtGui import QFont
import subprocess
import os
import sys

class FirmwareUploadThread(QThread):
    """Background thread for firmware upload to avoid blocking GUI"""
    progress = pyqtSignal(str)
    finished = pyqtSignal(bool, str)
    
    def __init__(self, port, firmware_path, erase_flash=False):
        super().__init__()
        self.port = port
        self.firmware_path = firmware_path
        self.erase_flash = erase_flash
    
    def run(self):
        try:
            # Find esptool
            esptool_cmd = self.find_esptool()
            if not esptool_cmd:
                self.finished.emit(False, "esptool.py not found. Install with: pip install esptool")
                return
            
            self.progress.emit("🔌 Connecting to ESP32...")
            
            # Erase flash if requested
            if self.erase_flash:
                self.progress.emit("🗑️  Erasing flash memory...")
                erase_cmd = [
                    esptool_cmd,
                    "--port", self.port,
                    "--chip", "esp32s3",
                    "erase_flash"
                ]
                result = subprocess.run(erase_cmd, capture_output=True, text=True)
                if result.returncode != 0:
                    self.finished.emit(False, f"Flash erase failed: {result.stderr}")
                    return
                self.progress.emit("✅ Flash erased successfully")
            
            # Flash firmware
            self.progress.emit("📤 Uploading firmware...")
            self.progress.emit(f"   File: {os.path.basename(self.firmware_path)}")
            
            flash_cmd = [
                esptool_cmd,
                "--port", self.port,
                "--chip", "esp32s3",
                "--baud", "921600",
                "--before", "default_reset",
                "--after", "hard_reset",
                "write_flash",
                "-z",
                "--flash_mode", "dio",
                "--flash_freq", "80m",
                "--flash_size", "detect",
                "0x0", self.firmware_path
            ]
            
            # Run esptool and capture output
            process = subprocess.Popen(
                flash_cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1
            )
            
            # Stream output
            for line in process.stdout:
                line = line.strip()
                if line:
                    if "%" in line or "Writing" in line:
                        self.progress.emit(f"   {line}")
                    elif "Hash" in line:
                        self.progress.emit("   Verifying...")
                    elif "Leaving" in line:
                        self.progress.emit("   Resetting device...")
            
            process.wait()
            
            if process.returncode == 0:
                self.progress.emit("✅ Firmware uploaded successfully!")
                self.progress.emit("🔄 Device is resetting...")
                self.finished.emit(True, "Upload complete!")
            else:
                self.finished.emit(False, "Upload failed. Check connection and try again.")
        
        except Exception as e:
            self.finished.emit(False, f"Error: {str(e)}")
    
    def find_esptool(self):
        """Find esptool.py in system"""
        # Try common locations
        if sys.platform == "win32":
            # Windows
            possible_paths = [
                "esptool.py",
                "esptool",
                os.path.join(sys.prefix, "Scripts", "esptool.py"),
                os.path.join(sys.prefix, "Scripts", "esptool.exe"),
            ]
        else:
            # Linux/Mac
            possible_paths = [
                "esptool.py",
                "esptool",
                os.path.join(sys.prefix, "bin", "esptool.py"),
                "/usr/local/bin/esptool.py",
            ]
        
        for path in possible_paths:
            try:
                result = subprocess.run([path, "version"], 
                                      capture_output=True, 
                                      timeout=2)
                if result.returncode == 0:
                    return path
            except:
                continue
        
        return None


class FirmwareUploadPanel(QWidget):
    command_sent = pyqtSignal(str)
    
    def __init__(self, protocol):
        super().__init__()
        self.protocol = protocol
        self.upload_thread = None
        self.init_ui()
    
    def init_ui(self):
        layout = QVBoxLayout()
        
        # Instructions
        info_group = QGroupBox("📋 Firmware Upload Instructions")
        info_layout = QVBoxLayout()
        
        instructions = QLabel(
            "⚠️ IMPORTANT: You must BUILD the firmware before uploading!\n\n"
            "Steps:\n"
            "1. Build firmware first (click 'Build Firmware' button below)\n"
            "2. After build completes, click 'Auto-Find' to locate firmware.bin\n"
            "3. Choose your COM port\n"
            "4. Click 'Upload Firmware'\n\n"
            "Note: Device will disconnect during upload. Reconnect after completion."
        )
        instructions.setWordWrap(True)
        instructions.setStyleSheet("QLabel { color: #ffeb3b; font-weight: bold; }")
        info_layout.addWidget(instructions)
        
        # Build firmware button
        build_btn_layout = QHBoxLayout()
        self.build_firmware_btn = QPushButton("🔨 Build Firmware (Opens Terminal)")
        self.build_firmware_btn.setMinimumHeight(40)
        self.build_firmware_btn.setStyleSheet("""
            QPushButton {
                background-color: #f57c00;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #fb8c00;
            }
        """)
        self.build_firmware_btn.setToolTip("Opens terminal in firmware folder to run: idf.py build")
        self.build_firmware_btn.clicked.connect(self.open_build_terminal)
        build_btn_layout.addWidget(self.build_firmware_btn)
        
        self.open_firmware_folder_btn = QPushButton("📁 Open Firmware Folder")
        self.open_firmware_folder_btn.setMinimumHeight(40)
        self.open_firmware_folder_btn.clicked.connect(self.open_firmware_folder)
        build_btn_layout.addWidget(self.open_firmware_folder_btn)
        
        info_layout.addLayout(build_btn_layout)
        
        info_group.setLayout(info_layout)
        layout.addWidget(info_group)
        
        # File selection
        file_group = QGroupBox("📁 Firmware File")
        file_layout = QVBoxLayout()
        
        file_select_layout = QHBoxLayout()
        self.file_path_edit = QLineEdit()
        self.file_path_edit.setPlaceholderText("Select firmware binary file...")
        self.file_path_edit.setReadOnly(True)
        file_select_layout.addWidget(self.file_path_edit)
        
        self.browse_btn = QPushButton("Browse...")
        self.browse_btn.setMinimumHeight(35)
        self.browse_btn.clicked.connect(self.browse_firmware)
        file_select_layout.addWidget(self.browse_btn)
        
        file_layout.addLayout(file_select_layout)
        
        # Quick path buttons
        quick_layout = QHBoxLayout()
        quick_layout.addWidget(QLabel("Quick Select:"))
        
        self.auto_find_btn = QPushButton("🔍 Auto-Find")
        self.auto_find_btn.setToolTip("Automatically find firmware.bin in project")
        self.auto_find_btn.clicked.connect(self.auto_find_firmware)
        quick_layout.addWidget(self.auto_find_btn)
        
        quick_layout.addStretch()
        file_layout.addLayout(quick_layout)
        
        file_group.setLayout(file_layout)
        layout.addWidget(file_group)
        
        # Port selection
        port_group = QGroupBox("🔌 Connection Settings")
        port_layout = QHBoxLayout()
        
        port_layout.addWidget(QLabel("COM Port:"))
        self.port_combo = QComboBox()
        self.port_combo.setMinimumHeight(35)
        self.refresh_ports()
        port_layout.addWidget(self.port_combo)
        
        self.refresh_ports_btn = QPushButton("🔄 Refresh")
        self.refresh_ports_btn.setMinimumHeight(35)
        self.refresh_ports_btn.clicked.connect(self.refresh_ports)
        port_layout.addWidget(self.refresh_ports_btn)
        
        port_group.setLayout(port_layout)
        layout.addWidget(port_group)
        
        # Options
        options_group = QGroupBox("⚙️ Upload Options")
        options_layout = QVBoxLayout()
        
        from PyQt5.QtWidgets import QCheckBox
        self.erase_flash_check = QCheckBox("Erase flash before upload (recommended for major updates)")
        options_layout.addWidget(self.erase_flash_check)
        
        options_group.setLayout(options_layout)
        layout.addWidget(options_group)
        
        # Upload button
        upload_layout = QHBoxLayout()
        self.upload_btn = QPushButton("📤 Upload Firmware")
        self.upload_btn.setMinimumHeight(50)
        self.upload_btn.setStyleSheet("""
            QPushButton {
                background-color: #1976d2;
                font-size: 12pt;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #2196f3;
            }
            QPushButton:disabled {
                background-color: #3d3d3d;
            }
        """)
        self.upload_btn.clicked.connect(self.start_upload)
        upload_layout.addWidget(self.upload_btn)
        
        self.cancel_btn = QPushButton("❌ Cancel")
        self.cancel_btn.setMinimumHeight(50)
        self.cancel_btn.setEnabled(False)
        self.cancel_btn.clicked.connect(self.cancel_upload)
        upload_layout.addWidget(self.cancel_btn)
        
        layout.addLayout(upload_layout)
        
        # Progress bar
        self.progress_bar = QProgressBar()
        self.progress_bar.setMinimumHeight(25)
        self.progress_bar.setTextVisible(True)
        self.progress_bar.setFormat("Ready")
        layout.addWidget(self.progress_bar)
        
        # Log output
        log_group = QGroupBox("📝 Upload Log")
        log_layout = QVBoxLayout()
        
        self.log_text = QTextEdit()
        self.log_text.setReadOnly(True)
        self.log_text.setFont(QFont("Consolas", 9))
        self.log_text.setMinimumHeight(250)
        log_layout.addWidget(self.log_text)
        
        log_btn_layout = QHBoxLayout()
        clear_log_btn = QPushButton("Clear Log")
        clear_log_btn.clicked.connect(lambda: self.log_text.clear())
        log_btn_layout.addWidget(clear_log_btn)
        log_btn_layout.addStretch()
        log_layout.addLayout(log_btn_layout)
        
        log_group.setLayout(log_layout)
        layout.addWidget(log_group)
        
        self.setLayout(layout)
        
        self.log("Firmware Upload Panel initialized")
        self.log("Install esptool if not present: pip install esptool")
    
    def log(self, message):
        """Add message to log"""
        self.log_text.append(message)
        # Auto-scroll to bottom
        scrollbar = self.log_text.verticalScrollBar()
        scrollbar.setValue(scrollbar.maximum())
    
    def refresh_ports(self):
        """Refresh available COM ports"""
        import serial.tools.list_ports
        
        current_port = self.port_combo.currentText()
        self.port_combo.clear()
        
        ports = serial.tools.list_ports.comports()
        for port in ports:
            self.port_combo.addItem(f"{port.device} - {port.description}")
        
        # Restore previous selection if available
        if current_port:
            index = self.port_combo.findText(current_port, Qt.MatchStartsWith)
            if index >= 0:
                self.port_combo.setCurrentIndex(index)
    
    def browse_firmware(self):
        """Open file dialog to select firmware"""
        file_path, _ = QFileDialog.getOpenFileName(
            self,
            "Select Firmware Binary",
            "",
            "Binary Files (*.bin);;All Files (*.*)"
        )
        
        if file_path:
            self.file_path_edit.setText(file_path)
            self.log(f"Selected: {file_path}")
            file_size = os.path.getsize(file_path) / 1024
            self.log(f"File size: {file_size:.1f} KB")
    
    def check_idf_installed(self):
        """Check if ESP-IDF is installed (checks common locations)"""
        # First try if idf.py is in PATH (Linux/Mac or ESP-IDF CMD on Windows)
        try:
            result = subprocess.run(['idf.py', '--version'], 
                                  capture_output=True, 
                                  timeout=2,
                                  text=True)
            if result.returncode == 0:
                version = result.stdout.strip()
                return True, version
        except:
            pass
        
        # On Windows, check if ESP-IDF is installed in common locations
        if sys.platform == "win32":
            possible_paths = [
                os.path.expandvars(r'%USERPROFILE%\.espressif'),
                r'C:\Espressif',
                os.path.expandvars(r'%PROGRAMFILES%\Espressif'),
            ]
            
            for base_path in possible_paths:
                if os.path.exists(base_path):
                    # Found Espressif folder, ESP-IDF is likely installed
                    return True, "Installed (use ESP-IDF CMD to build)"
        
        return False, None
    
    def open_build_terminal(self):
        """Open terminal in firmware folder for building"""
        firmware_path = os.path.abspath("../firmware")
        
        if not os.path.exists(firmware_path):
            self.log("❌ Firmware folder not found!")
            self.log(f"   Expected: {firmware_path}")
            return
        
        # Check if ESP-IDF is installed
        idf_installed, version = self.check_idf_installed()
        
        self.log("=" * 60)
        self.log("🔨 BUILD FIRMWARE")
        self.log("=" * 60)
        self.log("")
        
        if idf_installed:
            self.log(f"✅ ESP-IDF detected: {version}")
        else:
            self.log("⚠️  ESP-IDF not detected in current environment")
            self.log("   (This is normal if running from regular terminal)")
        
        self.log(f"📁 Firmware path: {firmware_path}")
        self.log("")
        
        if sys.platform == "win32":
            self.log("📝 WINDOWS BUILD INSTRUCTIONS:")
            self.log("")
            self.log("1. Open 'ESP-IDF 5.x CMD' from Start Menu")
            self.log("   (NOT regular CMD - it won't work!)")
            self.log("")
            self.log("2. Navigate to firmware folder:")
            self.log(f"   cd {firmware_path}")
            self.log("")
            self.log("3. Build firmware:")
            self.log("   idf.py build")
            self.log("")
            self.log("4. Wait 2-5 minutes (first time)")
            self.log("")
            self.log("5. After build completes, return to this tool")
            self.log("   and click 'Auto-Find' to locate firmware.bin")
            self.log("")
            self.log("⏱️  First build: 2-5 minutes")
            self.log("   Subsequent builds: 30-60 seconds")
            self.log("")
            
            if not idf_installed:
                self.log("=" * 60)
                self.log("⚠️  ESP-IDF NOT FOUND")
                self.log("=" * 60)
                self.log("")
                self.log("If you haven't installed ESP-IDF yet:")
                self.log("")
                self.log("1. Download installer:")
                self.log("   https://dl.espressif.com/dl/esp-idf/")
                self.log("")
                self.log("2. Run: esp-idf-tools-setup-online.exe")
                self.log("")
                self.log("3. Choose 'Express Installation'")
                self.log("   Select ESP32-S3 support")
                self.log("")
                self.log("4. Wait 10-15 minutes for installation")
                self.log("")
        else:
            self.log("📝 LINUX/MAC BUILD INSTRUCTIONS:")
            self.log("")
            self.log("1. Open terminal")
            self.log("")
            self.log("2. Setup ESP-IDF environment:")
            self.log("   . ~/esp/esp-idf/export.sh")
            self.log("")
            self.log("3. Navigate to firmware folder:")
            self.log(f"   cd {firmware_path}")
            self.log("")
            self.log("4. Build firmware:")
            self.log("   idf.py build")
            self.log("")
            self.log("5. Wait 2-5 minutes (first time)")
            self.log("")
        
        self.log("=" * 60)
        self.log("Opening file explorer to firmware folder...")
        self.log("=" * 60)
        
        try:
            if sys.platform == "win32":
                # Windows - open file explorer to firmware folder
                os.startfile(firmware_path)
                self.log("✅ File explorer opened!")
                self.log("")
                self.log("Now open 'ESP-IDF CMD' from Start Menu and follow")
                self.log("the instructions above.")
            elif sys.platform == "darwin":
                # macOS - open Finder
                subprocess.Popen(['open', firmware_path])
                self.log("✅ Finder opened!")
            else:
                # Linux - open file manager
                subprocess.Popen(['xdg-open', firmware_path])
                self.log("✅ File manager opened!")
        except Exception as e:
            self.log(f"❌ Failed to open file explorer: {e}")
            self.log(f"   Manually navigate to: {firmware_path}")
    
    def open_firmware_folder(self):
        """Open firmware folder in file explorer"""
        firmware_path = os.path.abspath("../firmware")
        
        if not os.path.exists(firmware_path):
            self.log("❌ Firmware folder not found!")
            return
        
        try:
            if sys.platform == "win32":
                os.startfile(firmware_path)
            elif sys.platform == "darwin":
                subprocess.Popen(['open', firmware_path])
            else:
                subprocess.Popen(['xdg-open', firmware_path])
            
            self.log(f"📁 Opened: {firmware_path}")
        except Exception as e:
            self.log(f"❌ Failed to open folder: {e}")
    
    def auto_find_firmware(self):
        """Automatically find firmware.bin in project"""
        # Try common locations
        possible_paths = [
            "../firmware/build/firmware.bin",
            "../../firmware/build/firmware.bin",
            "../firmware/build/bootloader/bootloader.bin",
        ]
        
        for path in possible_paths:
            full_path = os.path.abspath(path)
            if os.path.exists(full_path) and "firmware.bin" in full_path:
                self.file_path_edit.setText(full_path)
                self.log(f"✅ Auto-found: {full_path}")
                file_size = os.path.getsize(full_path) / 1024
                self.log(f"File size: {file_size:.1f} KB")
                return
        
        self.log("❌ Could not auto-find firmware.bin")
        self.log("")
        self.log("⚠️  You need to BUILD the firmware first!")
        self.log("")
        self.log("Option 1: Click '🔨 Build Firmware' button above")
        self.log("Option 2: Run manually in terminal:")
        self.log("   cd firmware")
        self.log("   idf.py build")
        self.log("")
        self.log("After build completes, click 'Auto-Find' again")
    
    def start_upload(self):
        """Start firmware upload process"""
        # Validate inputs
        firmware_path = self.file_path_edit.text()
        if not firmware_path:
            self.log("❌ Error: No firmware file selected")
            return
        
        if not os.path.exists(firmware_path):
            self.log("❌ Error: Firmware file not found")
            return
        
        port_text = self.port_combo.currentText()
        if not port_text:
            self.log("❌ Error: No COM port selected")
            return
        
        # Extract port name (before the dash)
        port = port_text.split(" - ")[0].strip()
        
        # Disconnect from device if connected
        if self.protocol.is_connected():
            self.log("🔌 Disconnecting from device...")
            self.protocol.disconnect()
        
        # Disable controls
        self.upload_btn.setEnabled(False)
        self.cancel_btn.setEnabled(True)
        self.browse_btn.setEnabled(False)
        self.auto_find_btn.setEnabled(False)
        self.port_combo.setEnabled(False)
        self.erase_flash_check.setEnabled(False)
        
        # Reset progress
        self.progress_bar.setValue(0)
        self.progress_bar.setFormat("Uploading...")
        
        self.log("=" * 60)
        self.log("🚀 Starting firmware upload...")
        self.log(f"   Port: {port}")
        self.log(f"   File: {firmware_path}")
        self.log(f"   Erase flash: {self.erase_flash_check.isChecked()}")
        self.log("=" * 60)
        
        # Start upload thread
        self.upload_thread = FirmwareUploadThread(
            port, 
            firmware_path,
            self.erase_flash_check.isChecked()
        )
        self.upload_thread.progress.connect(self.on_upload_progress)
        self.upload_thread.finished.connect(self.on_upload_finished)
        self.upload_thread.start()
    
    def cancel_upload(self):
        """Cancel ongoing upload"""
        if self.upload_thread and self.upload_thread.isRunning():
            self.log("⚠️  Cancelling upload...")
            self.upload_thread.terminate()
            self.upload_thread.wait()
            self.on_upload_finished(False, "Upload cancelled by user")
    
    def on_upload_progress(self, message):
        """Handle progress updates from upload thread"""
        self.log(message)
        
        # Update progress bar based on message
        if "Connecting" in message:
            self.progress_bar.setValue(10)
        elif "Erasing" in message:
            self.progress_bar.setValue(20)
        elif "Uploading" in message:
            self.progress_bar.setValue(30)
        elif "%" in message:
            # Try to extract percentage
            try:
                percent = int(message.split("%")[0].split()[-1])
                self.progress_bar.setValue(30 + int(percent * 0.6))
            except:
                pass
        elif "Verifying" in message:
            self.progress_bar.setValue(95)
        elif "Resetting" in message:
            self.progress_bar.setValue(100)
    
    def on_upload_finished(self, success, message):
        """Handle upload completion"""
        self.log("=" * 60)
        if success:
            self.log(f"✅ {message}")
            self.log("Device will restart automatically")
            self.log("Wait 5 seconds, then reconnect in Dashboard tab")
            self.progress_bar.setFormat("✅ Upload Complete!")
            self.progress_bar.setValue(100)
        else:
            self.log(f"❌ {message}")
            self.progress_bar.setFormat("❌ Upload Failed")
            self.progress_bar.setValue(0)
        self.log("=" * 60)
        
        # Re-enable controls
        self.upload_btn.setEnabled(True)
        self.cancel_btn.setEnabled(False)
        self.browse_btn.setEnabled(True)
        self.auto_find_btn.setEnabled(True)
        self.port_combo.setEnabled(True)
        self.erase_flash_check.setEnabled(True)
    
    def set_connected(self, connected):
        """Update UI based on connection status"""
        # Upload is available regardless of connection
        pass
