from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QGroupBox,
                             QLabel, QSpinBox, QPushButton, QMessageBox,
                             QFormLayout, QCheckBox)
from PyQt5.QtCore import pyqtSignal

class SettingsPanel(QWidget):
    def __init__(self, protocol):
        super().__init__()
        self.protocol = protocol
        self.init_ui()
    
    def init_ui(self):
        layout = QVBoxLayout()
        
        # Identity
        identity_group = QGroupBox("Device Identity")
        identity_layout = QFormLayout()
        
        self.can_id_spin = QSpinBox()
        self.can_id_spin.setRange(1, 6)
        self.can_id_spin.setValue(1)
        self.can_id_spin.setMinimumHeight(35)
        identity_layout.addRow("CAN ID:", self.can_id_spin)
        
        identity_group.setLayout(identity_layout)
        layout.addWidget(identity_group)
        
        # Motor settings
        motor_group = QGroupBox("Motor Configuration")
        motor_layout = QFormLayout()
        
        self.current_spin = QSpinBox()
        self.current_spin.setRange(0, 2000)
        self.current_spin.setValue(1400)
        self.current_spin.setSuffix(" mA")
        self.current_spin.setMinimumHeight(35)
        motor_layout.addRow("Current:", self.current_spin)
        
        self.microsteps_spin = QSpinBox()
        self.microsteps_spin.setRange(1, 256)
        self.microsteps_spin.setValue(16)
        self.microsteps_spin.setMinimumHeight(35)
        motor_layout.addRow("Microsteps:", self.microsteps_spin)
        
        motor_group.setLayout(motor_layout)
        layout.addWidget(motor_group)
        
        # Buttons
        btn_layout = QVBoxLayout()
        
        self.apply_btn = QPushButton("Apply CAN ID")
        self.apply_btn.setMinimumHeight(40)
        self.apply_btn.clicked.connect(self.apply_settings)
        self.apply_btn.setEnabled(False)
        btn_layout.addWidget(self.apply_btn)
        
        self.save_btn = QPushButton("💾 Save to Flash")
        self.save_btn.setMinimumHeight(40)
        self.save_btn.setStyleSheet("""
            QPushButton {
                background-color: #1565c0;
                font-size: 11pt;
            }
            QPushButton:hover {
                background-color: #1976d2;
            }
        """)
        self.save_btn.clicked.connect(self.save_config)
        self.save_btn.setEnabled(False)
        btn_layout.addWidget(self.save_btn)
        
        layout.addLayout(btn_layout)
        layout.addStretch()
        self.setLayout(layout)
    
    def set_connected(self, connected):
        self.apply_btn.setEnabled(connected)
        self.save_btn.setEnabled(connected)
    
    def apply_settings(self):
        if not self.protocol.is_connected():
            QMessageBox.warning(self, "Not Connected", 
                              "Please connect to device first")
            return
        
        can_id = self.can_id_spin.value()
        response = self.protocol.set_can_id(can_id)
        
        if "OK" in response:
            QMessageBox.information(self, "Success", 
                                  f"CAN ID set to {can_id}")
        else:
            QMessageBox.warning(self, "Error", 
                              f"Failed: {response}")
    
    def save_config(self):
        if not self.protocol.is_connected():
            QMessageBox.warning(self, "Not Connected", 
                              "Please connect to device first")
            return
        
        reply = QMessageBox.question(self, 'Save Configuration',
                                    'Save current configuration to flash memory?',
                                    QMessageBox.Yes | QMessageBox.No,
                                    QMessageBox.No)
        
        if reply == QMessageBox.Yes:
            response = self.protocol.save_config()
            if "OK" in response:
                QMessageBox.information(self, "Success", 
                                      "Configuration saved successfully!")
            else:
                QMessageBox.warning(self, "Error", 
                                  f"Failed to save: {response}")
