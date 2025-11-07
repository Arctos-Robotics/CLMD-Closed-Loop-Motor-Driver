from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QGroupBox,
                             QPushButton, QLabel, QDoubleSpinBox, QSpinBox,
                             QGridLayout)
from PyQt5.QtCore import pyqtSignal

class ControlPanel(QWidget):
    command_sent = pyqtSignal(str)
    
    def __init__(self, protocol):
        super().__init__()
        self.protocol = protocol
        self.init_ui()
    
    def init_ui(self):
        layout = QVBoxLayout()
        
        # Motor enable/disable
        enable_group = QGroupBox("Motor Control")
        enable_layout = QHBoxLayout()
        
        self.enable_btn = QPushButton("⚡ ENABLE")
        self.enable_btn.setMinimumHeight(50)
        self.enable_btn.setStyleSheet("""
            QPushButton {
                background-color: #2e7d32;
                font-size: 12pt;
            }
            QPushButton:hover {
                background-color: #388e3c;
            }
            QPushButton:disabled {
                background-color: #3d3d3d;
            }
        """)
        self.enable_btn.clicked.connect(self.enable_motor)
        self.enable_btn.setEnabled(False)
        enable_layout.addWidget(self.enable_btn)
        
        self.disable_btn = QPushButton("⏸ DISABLE")
        self.disable_btn.setMinimumHeight(50)
        self.disable_btn.setStyleSheet("""
            QPushButton {
                background-color: #f57c00;
                font-size: 12pt;
            }
            QPushButton:hover {
                background-color: #fb8c00;
            }
            QPushButton:disabled {
                background-color: #3d3d3d;
            }
        """)
        self.disable_btn.clicked.connect(self.disable_motor)
        self.disable_btn.setEnabled(False)
        enable_layout.addWidget(self.disable_btn)
        
        self.stop_btn = QPushButton("⛔ STOP")
        self.stop_btn.setMinimumHeight(50)
        self.stop_btn.setStyleSheet("""
            QPushButton {
                background-color: #c62828;
                font-size: 12pt;
            }
            QPushButton:hover {
                background-color: #d32f2f;
            }
            QPushButton:disabled {
                background-color: #3d3d3d;
            }
        """)
        self.stop_btn.clicked.connect(self.emergency_stop)
        self.stop_btn.setEnabled(False)
        enable_layout.addWidget(self.stop_btn)
        
        enable_group.setLayout(enable_layout)
        layout.addWidget(enable_group)
        
        # Jog controls
        jog_group = QGroupBox("Quick Jog")
        jog_layout = QGridLayout()
        
        jog_buttons = [
            ("-45°", -45), ("-10°", -10), ("-1°", -1),
            ("+1°", 1), ("+10°", 10), ("+45°", 45)
        ]
        
        for i, (text, value) in enumerate(jog_buttons):
            btn = QPushButton(text)
            btn.setMinimumHeight(40)
            btn.clicked.connect(lambda checked, v=value: self.jog(v))
            btn.setEnabled(False)
            jog_layout.addWidget(btn, i // 3, i % 3)
            setattr(self, f"jog_btn_{i}", btn)
        
        jog_group.setLayout(jog_layout)
        layout.addWidget(jog_group)
        
        # Absolute move
        abs_group = QGroupBox("Absolute Move")
        abs_layout = QVBoxLayout()
        
        abs_inputs = QHBoxLayout()
        abs_inputs.addWidget(QLabel("Target:"))
        self.abs_pos_spin = QDoubleSpinBox()
        self.abs_pos_spin.setRange(-360, 360)
        self.abs_pos_spin.setValue(0)
        self.abs_pos_spin.setSuffix(" °")
        self.abs_pos_spin.setMinimumHeight(35)
        abs_inputs.addWidget(self.abs_pos_spin)
        
        abs_inputs.addWidget(QLabel("Speed:"))
        self.abs_speed_spin = QSpinBox()
        self.abs_speed_spin.setRange(1, 1000)
        self.abs_speed_spin.setValue(400)
        self.abs_speed_spin.setSuffix(" RPM")
        self.abs_speed_spin.setMinimumHeight(35)
        abs_inputs.addWidget(self.abs_speed_spin)
        
        abs_layout.addLayout(abs_inputs)
        
        self.abs_move_btn = QPushButton("→ Move to Position")
        self.abs_move_btn.setMinimumHeight(40)
        self.abs_move_btn.clicked.connect(self.move_absolute)
        self.abs_move_btn.setEnabled(False)
        abs_layout.addWidget(self.abs_move_btn)
        
        abs_group.setLayout(abs_layout)
        layout.addWidget(abs_group)
        
        # Relative move
        rel_group = QGroupBox("Relative Move")
        rel_layout = QVBoxLayout()
        
        rel_inputs = QHBoxLayout()
        rel_inputs.addWidget(QLabel("Distance:"))
        self.rel_pos_spin = QDoubleSpinBox()
        self.rel_pos_spin.setRange(-360, 360)
        self.rel_pos_spin.setValue(45)
        self.rel_pos_spin.setSuffix(" °")
        self.rel_pos_spin.setMinimumHeight(35)
        rel_inputs.addWidget(self.rel_pos_spin)
        
        rel_inputs.addWidget(QLabel("Speed:"))
        self.rel_speed_spin = QSpinBox()
        self.rel_speed_spin.setRange(1, 1000)
        self.rel_speed_spin.setValue(400)
        self.rel_speed_spin.setSuffix(" RPM")
        self.rel_speed_spin.setMinimumHeight(35)
        rel_inputs.addWidget(self.rel_speed_spin)
        
        rel_layout.addLayout(rel_inputs)
        
        self.rel_move_btn = QPushButton("↔ Move Relative")
        self.rel_move_btn.setMinimumHeight(40)
        self.rel_move_btn.clicked.connect(self.move_relative)
        self.rel_move_btn.setEnabled(False)
        rel_layout.addWidget(self.rel_move_btn)
        
        rel_group.setLayout(rel_layout)
        layout.addWidget(rel_group)
        
        # Zero button
        zero_layout = QHBoxLayout()
        self.zero_btn = QPushButton("⊙ Set Zero Position")
        self.zero_btn.setMinimumHeight(40)
        self.zero_btn.clicked.connect(self.set_zero)
        self.zero_btn.setEnabled(False)
        zero_layout.addWidget(self.zero_btn)
        layout.addLayout(zero_layout)
        
        layout.addStretch()
        self.setLayout(layout)
    
    def set_connected(self, connected):
        self.enable_btn.setEnabled(connected)
        self.disable_btn.setEnabled(connected)
        self.stop_btn.setEnabled(connected)
        self.abs_move_btn.setEnabled(connected)
        self.rel_move_btn.setEnabled(connected)
        self.zero_btn.setEnabled(connected)
        
        for i in range(6):
            btn = getattr(self, f"jog_btn_{i}")
            btn.setEnabled(connected)
    
    def update_status(self, status):
        # Could update button states based on motor status
        pass
    
    def enable_motor(self):
        response = self.protocol.enable_motor()
        self.command_sent.emit(f"Enable: {response}")
    
    def disable_motor(self):
        response = self.protocol.disable_motor()
        self.command_sent.emit(f"Disable: {response}")
    
    def emergency_stop(self):
        response = self.protocol.stop()
        self.command_sent.emit(f"EMERGENCY STOP: {response}")
    
    def jog(self, degrees):
        response = self.protocol.move_relative(degrees, 200)
        self.command_sent.emit(f"Jog {degrees:+.0f}°: {response}")
    
    def move_absolute(self):
        pos = self.abs_pos_spin.value()
        speed = self.abs_speed_spin.value()
        response = self.protocol.move_absolute(pos, speed)
        self.command_sent.emit(f"Move to {pos:.1f}° @ {speed} RPM: {response}")
    
    def move_relative(self):
        pos = self.rel_pos_spin.value()
        speed = self.rel_speed_spin.value()
        response = self.protocol.move_relative(pos, speed)
        self.command_sent.emit(f"Move {pos:+.1f}° @ {speed} RPM: {response}")
    
    def set_zero(self):
        response = self.protocol.set_zero()
        self.command_sent.emit(f"Set Zero: {response}")
