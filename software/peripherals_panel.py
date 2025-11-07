from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QGroupBox,
                             QLabel, QSlider, QPushButton, QProgressBar,
                             QCheckBox, QTextEdit)
from PyQt5.QtCore import pyqtSignal, QTimer, Qt
from PyQt5.QtGui import QColor

class ColorPreview(QWidget):
    def __init__(self):
        super().__init__()
        self.color = QColor(0, 0, 0)
        self.setMinimumSize(100, 100)
    
    def set_color(self, r, g, b):
        self.color = QColor(r, g, b)
        self.update()
    
    def paintEvent(self, event):
        from PyQt5.QtGui import QPainter
        painter = QPainter(self)
        painter.fillRect(self.rect(), self.color)


class PeripheralsPanel(QWidget):
    command_sent = pyqtSignal(str)
    
    def __init__(self, protocol):
        super().__init__()
        self.protocol = protocol
        self.init_ui()
        
        # Update timer for endstops
        self.update_timer = QTimer()
        self.update_timer.timeout.connect(self.update_endstops)
        self.update_timer.start(200)  # 5 Hz
    
    def init_ui(self):
        layout = QVBoxLayout()
        
        # RGB LED Control
        led_group = QGroupBox("RGB LED Control")
        led_layout = QVBoxLayout()
        
        # Color preview
        preview_layout = QHBoxLayout()
        self.color_preview = ColorPreview()
        preview_layout.addWidget(self.color_preview)
        
        # Sliders
        sliders_layout = QVBoxLayout()
        
        # Red
        red_layout = QHBoxLayout()
        red_layout.addWidget(QLabel("R:"))
        self.red_slider = QSlider(Qt.Horizontal)
        self.red_slider.setRange(0, 255)
        self.red_slider.setValue(0)
        self.red_slider.valueChanged.connect(self.update_led_preview)
        red_layout.addWidget(self.red_slider)
        self.red_value_label = QLabel("0")
        self.red_value_label.setMinimumWidth(30)
        red_layout.addWidget(self.red_value_label)
        sliders_layout.addLayout(red_layout)
        
        # Green
        green_layout = QHBoxLayout()
        green_layout.addWidget(QLabel("G:"))
        self.green_slider = QSlider(Qt.Horizontal)
        self.green_slider.setRange(0, 255)
        self.green_slider.setValue(0)
        self.green_slider.valueChanged.connect(self.update_led_preview)
        green_layout.addWidget(self.green_slider)
        self.green_value_label = QLabel("0")
        self.green_value_label.setMinimumWidth(30)
        green_layout.addWidget(self.green_value_label)
        sliders_layout.addLayout(green_layout)
        
        # Blue
        blue_layout = QHBoxLayout()
        blue_layout.addWidget(QLabel("B:"))
        self.blue_slider = QSlider(Qt.Horizontal)
        self.blue_slider.setRange(0, 255)
        self.blue_slider.setValue(0)
        self.blue_slider.valueChanged.connect(self.update_led_preview)
        blue_layout.addWidget(self.blue_slider)
        self.blue_value_label = QLabel("0")
        self.blue_value_label.setMinimumWidth(30)
        blue_layout.addWidget(self.blue_value_label)
        sliders_layout.addLayout(blue_layout)
        
        preview_layout.addLayout(sliders_layout)
        led_layout.addLayout(preview_layout)
        
        # Preset colors
        preset_layout = QHBoxLayout()
        presets = [
            ("Off", 0, 0, 0),
            ("Red", 255, 0, 0),
            ("Green", 0, 255, 0),
            ("Blue", 0, 0, 255),
            ("Yellow", 255, 255, 0),
            ("Cyan", 0, 255, 255),
            ("Magenta", 255, 0, 255),
            ("White", 255, 255, 255),
        ]
        
        for name, r, g, b in presets:
            btn = QPushButton(name)
            btn.setMinimumHeight(30)
            btn.clicked.connect(lambda checked, r=r, g=g, b=b: self.set_preset_color(r, g, b))
            preset_layout.addWidget(btn)
        
        led_layout.addLayout(preset_layout)
        
        # Apply button
        self.apply_led_btn = QPushButton("Apply LED Color")
        self.apply_led_btn.setMinimumHeight(40)
        self.apply_led_btn.clicked.connect(self.apply_led_color)
        self.apply_led_btn.setEnabled(False)
        led_layout.addWidget(self.apply_led_btn)
        
        led_group.setLayout(led_layout)
        layout.addWidget(led_group)
        
        # Endstops
        endstop_group = QGroupBox("Limit Switches / Endstops")
        endstop_layout = QVBoxLayout()
        
        endstop_status = QHBoxLayout()
        
        # J_MIN
        jmin_layout = QVBoxLayout()
        jmin_layout.addWidget(QLabel("J_MIN"))
        self.jmin_indicator = QLabel("●")
        self.jmin_indicator.setStyleSheet("color: #808080; font-size: 36pt;")
        self.jmin_indicator.setAlignment(Qt.AlignCenter)
        jmin_layout.addWidget(self.jmin_indicator)
        self.jmin_label = QLabel("Not Triggered")
        self.jmin_label.setAlignment(Qt.AlignCenter)
        jmin_layout.addWidget(self.jmin_label)
        endstop_status.addLayout(jmin_layout)
        
        # J_MAX
        jmax_layout = QVBoxLayout()
        jmax_layout.addWidget(QLabel("J_MAX"))
        self.jmax_indicator = QLabel("●")
        self.jmax_indicator.setStyleSheet("color: #808080; font-size: 36pt;")
        self.jmax_indicator.setAlignment(Qt.AlignCenter)
        jmax_layout.addWidget(self.jmax_indicator)
        self.jmax_label = QLabel("Not Triggered")
        self.jmax_label.setAlignment(Qt.AlignCenter)
        jmax_layout.addWidget(self.jmax_label)
        endstop_status.addLayout(jmax_layout)
        
        endstop_layout.addLayout(endstop_status)
        
        endstop_group.setLayout(endstop_layout)
        layout.addWidget(endstop_group)
        
        # Encoder diagnostics
        encoder_group = QGroupBox("Encoder Diagnostics")
        encoder_layout = QVBoxLayout()
        
        self.encoder_raw_label = QLabel("Raw Value: --")
        encoder_layout.addWidget(self.encoder_raw_label)
        
        self.encoder_status_label = QLabel("Communication: --")
        encoder_layout.addWidget(self.encoder_status_label)
        
        self.refresh_encoder_btn = QPushButton("Refresh Encoder Info")
        self.refresh_encoder_btn.setMinimumHeight(35)
        self.refresh_encoder_btn.clicked.connect(self.refresh_encoder)
        self.refresh_encoder_btn.setEnabled(False)
        encoder_layout.addWidget(self.refresh_encoder_btn)
        
        encoder_group.setLayout(encoder_layout)
        layout.addWidget(encoder_group)
        
        layout.addStretch()
        self.setLayout(layout)
    
    def set_connected(self, connected):
        self.apply_led_btn.setEnabled(connected)
        self.refresh_encoder_btn.setEnabled(connected)
    
    def update_led_preview(self):
        r = self.red_slider.value()
        g = self.green_slider.value()
        b = self.blue_slider.value()
        
        self.red_value_label.setText(str(r))
        self.green_value_label.setText(str(g))
        self.blue_value_label.setText(str(b))
        
        self.color_preview.set_color(r, g, b)
    
    def set_preset_color(self, r, g, b):
        self.red_slider.setValue(r)
        self.green_slider.setValue(g)
        self.blue_slider.setValue(b)
    
    def apply_led_color(self):
        r = self.red_slider.value()
        g = self.green_slider.value()
        b = self.blue_slider.value()
        
        response = self.protocol.send_command(f"LED {r} {g} {b}")
        self.command_sent.emit(f"Set LED: R={r}, G={g}, B={b} - {response}")
    
    def update_endstops(self):
        if not self.protocol.is_connected():
            return
        
        response = self.protocol.send_command("GET_ENDSTOPS")
        if response.startswith("ENDSTOPS"):
            try:
                parts = response.split()
                jmin = int(parts[1])
                jmax = int(parts[2])
                
                # Update J_MIN
                if jmin:
                    self.jmin_indicator.setStyleSheet("color: #ff5555; font-size: 36pt;")
                    self.jmin_label.setText("TRIGGERED")
                else:
                    self.jmin_indicator.setStyleSheet("color: #55ff55; font-size: 36pt;")
                    self.jmin_label.setText("Not Triggered")
                
                # Update J_MAX
                if jmax:
                    self.jmax_indicator.setStyleSheet("color: #ff5555; font-size: 36pt;")
                    self.jmax_label.setText("TRIGGERED")
                else:
                    self.jmax_indicator.setStyleSheet("color: #55ff55; font-size: 36pt;")
                    self.jmax_label.setText("Not Triggered")
            except:
                pass
    
    def refresh_encoder(self):
        response = self.protocol.send_command("GET_ENCODER")
        if response.startswith("ENCODER"):
            try:
                parts = response.split()
                raw_value = int(parts[1])
                self.encoder_raw_label.setText(f"Raw Value: {raw_value} (0-32767)")
                self.encoder_status_label.setText("Communication: OK")
                self.encoder_status_label.setStyleSheet("color: #55ff55;")
            except:
                self.encoder_status_label.setText("Communication: ERROR")
                self.encoder_status_label.setStyleSheet("color: #ff5555;")
