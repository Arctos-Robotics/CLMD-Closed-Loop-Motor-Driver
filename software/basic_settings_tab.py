from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QGroupBox,
                             QLabel, QSpinBox, QPushButton, QMessageBox,
                             QFormLayout)
from PyQt5.QtCore import QTimer
import logging

logger = logging.getLogger(__name__)

class BasicSettingsTab(QWidget):
    def __init__(self, protocol):
        super().__init__()
        self.protocol = protocol
        self.init_ui()
    
    def init_ui(self):
        layout = QVBoxLayout()
        
        # Identity group
        identity_group = QGroupBox("Identity")
        identity_layout = QFormLayout()
        
        self.can_id_spin = QSpinBox()
        self.can_id_spin.setRange(1, 6)
        self.can_id_spin.setValue(1)
        identity_layout.addRow("CAN ID:", self.can_id_spin)
        
        identity_group.setLayout(identity_layout)
        layout.addWidget(identity_group)
        
        # Motor settings group
        motor_group = QGroupBox("Motor Settings")
        motor_layout = QFormLayout()
        
        self.current_spin = QSpinBox()
        self.current_spin.setRange(0, 2000)
        self.current_spin.setValue(1400)
        self.current_spin.setSuffix(" mA")
        motor_layout.addRow("Motor Current:", self.current_spin)
        
        self.microsteps_spin = QSpinBox()
        self.microsteps_spin.setRange(1, 256)
        self.microsteps_spin.setValue(16)
        motor_layout.addRow("Microstepping:", self.microsteps_spin)
        
        motor_group.setLayout(motor_layout)
        layout.addWidget(motor_group)
        
        # Motion limits group
        limits_group = QGroupBox("Motion Limits")
        limits_layout = QFormLayout()
        
        self.max_rpm_spin = QSpinBox()
        self.max_rpm_spin.setRange(1, 1000)
        self.max_rpm_spin.setValue(600)
        self.max_rpm_spin.setSuffix(" RPM")
        limits_layout.addRow("Max Speed:", self.max_rpm_spin)
        
        self.max_accel_spin = QSpinBox()
        self.max_accel_spin.setRange(10, 2000)
        self.max_accel_spin.setValue(500)
        self.max_accel_spin.setSuffix(" deg/s²")
        limits_layout.addRow("Max Acceleration:", self.max_accel_spin)
        
        limits_group.setLayout(limits_layout)
        layout.addWidget(limits_group)
        
        # Buttons
        btn_layout = QHBoxLayout()
        
        self.apply_btn = QPushButton("Apply CAN ID")
        self.apply_btn.clicked.connect(self.apply_settings)
        btn_layout.addWidget(self.apply_btn)
        
        self.save_btn = QPushButton("Save to Flash")
        self.save_btn.clicked.connect(self.save_config)
        btn_layout.addWidget(self.save_btn)
        
        btn_layout.addStretch()
        layout.addLayout(btn_layout)
        
        layout.addStretch()
        self.setLayout(layout)
    
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
                              f"Failed to set CAN ID: {response}")
    
    def save_config(self):
        if not self.protocol.is_connected():
            QMessageBox.warning(self, "Not Connected", 
                              "Please connect to device first")
            return
        
        reply = QMessageBox.question(self, 'Save Configuration',
                                    'Save current configuration to flash?',
                                    QMessageBox.Yes | QMessageBox.No,
                                    QMessageBox.No)
        
        if reply == QMessageBox.Yes:
            response = self.protocol.save_config()
            if "OK" in response:
                QMessageBox.information(self, "Success", 
                                      "Configuration saved to flash")
            else:
                QMessageBox.warning(self, "Error", 
                                  f"Failed to save: {response}")
