from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QGroupBox,
                             QLabel, QDoubleSpinBox, QSpinBox, QPushButton, 
                             QMessageBox, QProgressBar, QTextEdit)
from PyQt5.QtCore import QTimer
import logging

logger = logging.getLogger(__name__)

class MotionTestTab(QWidget):
    def __init__(self, protocol):
        super().__init__()
        self.protocol = protocol
        self.init_ui()
        
        # Position update timer
        self.pos_timer = QTimer()
        self.pos_timer.timeout.connect(self.update_position)
        self.pos_timer.start(100)  # Update every 100ms
    
    def init_ui(self):
        layout = QVBoxLayout()
        
        # Motor control group
        control_group = QGroupBox("Motor Control")
        control_layout = QVBoxLayout()
        
        # Enable/Disable buttons
        enable_layout = QHBoxLayout()
        self.enable_btn = QPushButton("Enable Motor")
        self.enable_btn.setMinimumHeight(40)
        self.enable_btn.clicked.connect(self.enable_motor)
        enable_layout.addWidget(self.enable_btn)
        
        self.disable_btn = QPushButton("Disable Motor")
        self.disable_btn.setMinimumHeight(40)
        self.disable_btn.clicked.connect(self.disable_motor)
        enable_layout.addWidget(self.disable_btn)
        
        self.stop_btn = QPushButton("STOP")
        self.stop_btn.setMinimumHeight(40)
        self.stop_btn.setStyleSheet("background-color: #ff4444; color: white; font-weight: bold;")
        self.stop_btn.clicked.connect(self.emergency_stop)
        enable_layout.addWidget(self.stop_btn)
        
        control_layout.addLayout(enable_layout)
        control_group.setLayout(control_layout)
        layout.addWidget(control_group)
        
        # Position display
        pos_group = QGroupBox("Current Position")
        pos_layout = QVBoxLayout()
        
        self.position_label = QLabel("Position: --- °")
        self.position_label.setStyleSheet("font-size: 24px; font-weight: bold;")
        pos_layout.addWidget(self.position_label)
        
        # Zero button
        zero_layout = QHBoxLayout()
        self.zero_btn = QPushButton("Set Zero Position")
        self.zero_btn.clicked.connect(self.set_zero)
        zero_layout.addWidget(self.zero_btn)
        zero_layout.addStretch()
        pos_layout.addLayout(zero_layout)
        
        pos_group.setLayout(pos_layout)
        layout.addWidget(pos_group)
        
        # Jog controls
        jog_group = QGroupBox("Jog Control")
        jog_layout = QVBoxLayout()
        
        jog_btn_layout = QHBoxLayout()
        
        self.jog_minus_10 = QPushButton("-10°")
        self.jog_minus_10.clicked.connect(lambda: self.jog(-10))
        jog_btn_layout.addWidget(self.jog_minus_10)
        
        self.jog_minus_1 = QPushButton("-1°")
        self.jog_minus_1.clicked.connect(lambda: self.jog(-1))
        jog_btn_layout.addWidget(self.jog_minus_1)
        
        self.jog_plus_1 = QPushButton("+1°")
        self.jog_plus_1.clicked.connect(lambda: self.jog(1))
        jog_btn_layout.addWidget(self.jog_plus_1)
        
        self.jog_plus_10 = QPushButton("+10°")
        self.jog_plus_10.clicked.connect(lambda: self.jog(10))
        jog_btn_layout.addWidget(self.jog_plus_10)
        
        jog_layout.addLayout(jog_btn_layout)
        jog_group.setLayout(jog_layout)
        layout.addWidget(jog_group)
        
        # Absolute move
        abs_group = QGroupBox("Absolute Move")
        abs_layout = QHBoxLayout()
        
        abs_layout.addWidget(QLabel("Target:"))
        self.abs_pos_spin = QDoubleSpinBox()
        self.abs_pos_spin.setRange(-360, 360)
        self.abs_pos_spin.setValue(0)
        self.abs_pos_spin.setSuffix(" °")
        abs_layout.addWidget(self.abs_pos_spin)
        
        abs_layout.addWidget(QLabel("Speed:"))
        self.abs_speed_spin = QSpinBox()
        self.abs_speed_spin.setRange(1, 1000)
        self.abs_speed_spin.setValue(400)
        self.abs_speed_spin.setSuffix(" RPM")
        abs_layout.addWidget(self.abs_speed_spin)
        
        self.abs_move_btn = QPushButton("Move Absolute")
        self.abs_move_btn.clicked.connect(self.move_absolute)
        abs_layout.addWidget(self.abs_move_btn)
        
        abs_group.setLayout(abs_layout)
        layout.addWidget(abs_group)
        
        # Relative move
        rel_group = QGroupBox("Relative Move")
        rel_layout = QHBoxLayout()
        
        rel_layout.addWidget(QLabel("Distance:"))
        self.rel_pos_spin = QDoubleSpinBox()
        self.rel_pos_spin.setRange(-360, 360)
        self.rel_pos_spin.setValue(45)
        self.rel_pos_spin.setSuffix(" °")
        rel_layout.addWidget(self.rel_pos_spin)
        
        rel_layout.addWidget(QLabel("Speed:"))
        self.rel_speed_spin = QSpinBox()
        self.rel_speed_spin.setRange(1, 1000)
        self.rel_speed_spin.setValue(400)
        self.rel_speed_spin.setSuffix(" RPM")
        rel_layout.addWidget(self.rel_speed_spin)
        
        self.rel_move_btn = QPushButton("Move Relative")
        self.rel_move_btn.clicked.connect(self.move_relative)
        rel_layout.addWidget(self.rel_move_btn)
        
        rel_group.setLayout(rel_layout)
        layout.addWidget(rel_group)
        
        # Log
        log_group = QGroupBox("Command Log")
        log_layout = QVBoxLayout()
        
        self.log_text = QTextEdit()
        self.log_text.setReadOnly(True)
        self.log_text.setMaximumHeight(100)
        log_layout.addWidget(self.log_text)
        
        log_group.setLayout(log_layout)
        layout.addWidget(log_group)
        
        layout.addStretch()
        self.setLayout(layout)
    
    def log_message(self, message):
        self.log_text.append(message)
        # Auto-scroll to bottom
        scrollbar = self.log_text.verticalScrollBar()
        scrollbar.setValue(scrollbar.maximum())
    
    def update_position(self):
        if self.protocol.is_connected():
            pos = self.protocol.get_position()
            if pos is not None:
                self.position_label.setText(f"Position: {pos:.2f} °")
    
    def enable_motor(self):
        if not self.protocol.is_connected():
            QMessageBox.warning(self, "Not Connected", "Please connect first")
            return
        
        response = self.protocol.enable_motor()
        self.log_message(f"Enable: {response}")
    
    def disable_motor(self):
        if not self.protocol.is_connected():
            return
        
        response = self.protocol.disable_motor()
        self.log_message(f"Disable: {response}")
    
    def emergency_stop(self):
        if not self.protocol.is_connected():
            return
        
        response = self.protocol.stop()
        self.log_message(f"STOP: {response}")
    
    def set_zero(self):
        if not self.protocol.is_connected():
            return
        
        response = self.protocol.set_zero()
        self.log_message(f"Set Zero: {response}")
    
    def jog(self, degrees):
        if not self.protocol.is_connected():
            return
        
        response = self.protocol.move_relative(degrees, 200)
        self.log_message(f"Jog {degrees}°: {response}")
    
    def move_absolute(self):
        if not self.protocol.is_connected():
            QMessageBox.warning(self, "Not Connected", "Please connect first")
            return
        
        pos = self.abs_pos_spin.value()
        speed = self.abs_speed_spin.value()
        
        response = self.protocol.move_absolute(pos, speed)
        self.log_message(f"Move Abs {pos}° @ {speed} RPM: {response}")
    
    def move_relative(self):
        if not self.protocol.is_connected():
            QMessageBox.warning(self, "Not Connected", "Please connect first")
            return
        
        pos = self.rel_pos_spin.value()
        speed = self.rel_speed_spin.value()
        
        response = self.protocol.move_relative(pos, speed)
        self.log_message(f"Move Rel {pos}° @ {speed} RPM: {response}")
