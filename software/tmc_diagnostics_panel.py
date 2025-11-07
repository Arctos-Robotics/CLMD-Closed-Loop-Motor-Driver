from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QGroupBox,
                             QLabel, QPushButton, QTextEdit, QSpinBox)
from PyQt5.QtCore import QTimer, pyqtSignal
from PyQt5.QtGui import QFont

class TMCDiagnosticsPanel(QWidget):
    command_sent = pyqtSignal(str)
    
    def __init__(self, protocol):
        super().__init__()
        self.protocol = protocol
        self.init_ui()
        
        # Auto-refresh timer
        self.refresh_timer = QTimer()
        self.refresh_timer.timeout.connect(self.refresh_status)
        self.refresh_timer.start(1000)  # Update every second
    
    def init_ui(self):
        layout = QVBoxLayout()
        
        # CoolStep control
        coolstep_group = QGroupBox("⚡ CoolStep Power Savings")
        coolstep_layout = QVBoxLayout()
        
        info_label = QLabel("CoolStep automatically reduces motor current when load is low.\n"
                           "Expected power savings: 50-75% when idle or lightly loaded.")
        info_label.setWordWrap(True)
        coolstep_layout.addWidget(info_label)
        
        btn_layout = QHBoxLayout()
        self.enable_coolstep_btn = QPushButton("✅ Enable CoolStep")
        self.enable_coolstep_btn.setMinimumHeight(40)
        self.enable_coolstep_btn.setStyleSheet("""
            QPushButton {
                background-color: #2e7d32;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #388e3c;
            }
        """)
        self.enable_coolstep_btn.clicked.connect(self.enable_coolstep)
        btn_layout.addWidget(self.enable_coolstep_btn)
        
        self.disable_coolstep_btn = QPushButton("❌ Disable CoolStep")
        self.disable_coolstep_btn.setMinimumHeight(40)
        self.disable_coolstep_btn.setStyleSheet("""
            QPushButton {
                background-color: #c62828;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #d32f2f;
            }
        """)
        self.disable_coolstep_btn.clicked.connect(self.disable_coolstep)
        btn_layout.addWidget(self.disable_coolstep_btn)
        
        coolstep_layout.addLayout(btn_layout)
        coolstep_group.setLayout(coolstep_layout)
        layout.addWidget(coolstep_group)
        
        # Current control
        current_group = QGroupBox("🔋 Motor Current Settings")
        current_layout = QHBoxLayout()
        
        current_layout.addWidget(QLabel("Run Current:"))
        self.run_current_spin = QSpinBox()
        self.run_current_spin.setRange(100, 2000)
        self.run_current_spin.setValue(1400)
        self.run_current_spin.setSuffix(" mA")
        self.run_current_spin.setMinimumHeight(35)
        current_layout.addWidget(self.run_current_spin)
        
        current_layout.addWidget(QLabel("Hold Current:"))
        self.hold_current_spin = QSpinBox()
        self.hold_current_spin.setRange(100, 2000)
        self.hold_current_spin.setValue(700)
        self.hold_current_spin.setSuffix(" mA")
        self.hold_current_spin.setMinimumHeight(35)
        current_layout.addWidget(self.hold_current_spin)
        
        self.set_current_btn = QPushButton("Apply Current")
        self.set_current_btn.setMinimumHeight(35)
        self.set_current_btn.clicked.connect(self.set_current)
        current_layout.addWidget(self.set_current_btn)
        
        current_group.setLayout(current_layout)
        layout.addWidget(current_group)
        
        # Status display
        status_group = QGroupBox("📊 TMC2209 Status")
        status_layout = QVBoxLayout()
        
        self.status_label = QLabel("Driver Status: --")
        self.status_label.setFont(QFont("Consolas", 10))
        self.temp_label = QLabel("Temperature: --")
        self.temp_label.setFont(QFont("Consolas", 10))
        self.stallguard_label = QLabel("StallGuard (Load): --")
        self.stallguard_label.setFont(QFont("Consolas", 10))
        
        status_layout.addWidget(self.status_label)
        status_layout.addWidget(self.temp_label)
        status_layout.addWidget(self.stallguard_label)
        
        refresh_btn = QPushButton("🔄 Refresh Now")
        refresh_btn.setMinimumHeight(35)
        refresh_btn.clicked.connect(self.refresh_status)
        status_layout.addWidget(refresh_btn)
        
        status_group.setLayout(status_layout)
        layout.addWidget(status_group)
        
        # Diagnostics log
        diag_group = QGroupBox("📝 Diagnostic Log")
        diag_layout = QVBoxLayout()
        
        self.diag_text = QTextEdit()
        self.diag_text.setReadOnly(True)
        self.diag_text.setMaximumHeight(200)
        self.diag_text.setFont(QFont("Consolas", 9))
        diag_layout.addWidget(self.diag_text)
        
        clear_btn = QPushButton("Clear Log")
        clear_btn.clicked.connect(lambda: self.diag_text.clear())
        diag_layout.addWidget(clear_btn)
        
        diag_group.setLayout(diag_layout)
        layout.addWidget(diag_group)
        
        layout.addStretch()
        self.setLayout(layout)
        
        self.log("TMC2209 Diagnostics Panel initialized")
        self.log("Enable CoolStep for 50-75% power savings!")
    
    def log(self, message):
        """Add message to diagnostic log"""
        self.diag_text.append(message)
    
    def enable_coolstep(self):
        response = self.protocol.send_command("ENABLE_COOLSTEP")
        self.command_sent.emit(f"Enable CoolStep: {response}")
        if "OK" in response:
            self.log("✅ CoolStep ENABLED - Motor will use 50-75% less power when idle")
            self.log("   Monitor current draw to verify power savings")
        else:
            self.log(f"❌ Failed to enable CoolStep: {response}")
    
    def disable_coolstep(self):
        response = self.protocol.send_command("DISABLE_COOLSTEP")
        self.command_sent.emit(f"Disable CoolStep: {response}")
        if "OK" in response:
            self.log("❌ CoolStep DISABLED - Motor using full current")
        else:
            self.log(f"❌ Failed to disable CoolStep: {response}")
    
    def set_current(self):
        run_ma = self.run_current_spin.value()
        hold_ma = self.hold_current_spin.value()
        response = self.protocol.send_command(f"SET_TMC_CURRENT {run_ma} {hold_ma}")
        self.command_sent.emit(f"Set current: {response}")
        if "OK" in response:
            self.log(f"✅ Current set: {run_ma}mA run, {hold_ma}mA hold")
        else:
            self.log(f"❌ Failed to set current: {response}")
    
    def refresh_status(self):
        if not self.protocol.is_connected():
            self.status_label.setText("Driver Status: Not connected")
            self.temp_label.setText("Temperature: --")
            self.stallguard_label.setText("StallGuard (Load): --")
            return
        
        # Get TMC status
        status_resp = self.protocol.send_command("GET_TMC_STATUS")
        if status_resp.startswith("TMC_STATUS"):
            try:
                status_hex = status_resp.split()[1]
                self.status_label.setText(f"Driver Status: {status_hex}")
                
                # Parse status bits
                status = int(status_hex, 16)
                if status & 0x01:
                    self.log("⚠️  Overtemperature pre-warning detected!")
                    self.temp_label.setStyleSheet("color: #ff9800;")
                if status & 0x02:
                    self.log("🔥 OVERTEMPERATURE SHUTDOWN!")
                    self.temp_label.setStyleSheet("color: #f44336;")
                if status & (0x04 | 0x08):
                    self.log("⚠️  Short to ground detected!")
                if status & (0x10 | 0x20):
                    self.log("⚠️  Short to supply detected!")
                if status & (0x40 | 0x80):
                    self.log("⚠️  Open load detected - check motor connection!")
            except:
                pass
        
        # Get temperature
        temp_resp = self.protocol.send_command("GET_TMC_TEMP")
        if temp_resp.startswith("TMC_TEMP"):
            try:
                temp = int(temp_resp.split()[1])
                self.temp_label.setText(f"Temperature: ~{temp}°C")
                if temp < 100:
                    self.temp_label.setStyleSheet("color: #4caf50;")
                elif temp < 120:
                    self.temp_label.setStyleSheet("color: #ff9800;")
                else:
                    self.temp_label.setStyleSheet("color: #f44336;")
            except:
                pass
        
        # Get StallGuard
        sg_resp = self.protocol.send_command("GET_STALLGUARD")
        if sg_resp.startswith("STALLGUARD"):
            try:
                sg_value = int(sg_resp.split()[1])
                self.stallguard_label.setText(f"StallGuard (Load): {sg_value}")
                # Higher value = higher load
                if sg_value < 50:
                    load_text = "Very Low"
                    color = "#4caf50"
                elif sg_value < 150:
                    load_text = "Low"
                    color = "#8bc34a"
                elif sg_value < 300:
                    load_text = "Medium"
                    color = "#ffc107"
                elif sg_value < 500:
                    load_text = "High"
                    color = "#ff9800"
                else:
                    load_text = "Very High"
                    color = "#f44336"
                
                self.stallguard_label.setText(f"StallGuard (Load): {sg_value} ({load_text})")
                self.stallguard_label.setStyleSheet(f"color: {color};")
            except:
                pass
    
    def set_connected(self, connected):
        self.enable_coolstep_btn.setEnabled(connected)
        self.disable_coolstep_btn.setEnabled(connected)
        self.set_current_btn.setEnabled(connected)
        
        if connected:
            self.log("🔌 Connected to device")
        else:
            self.log("🔌 Disconnected from device")
