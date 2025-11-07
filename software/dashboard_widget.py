from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLabel, 
                             QPushButton, QComboBox, QGroupBox, QTextEdit,
                             QFrame)
from PyQt5.QtCore import pyqtSignal, QTimer, Qt
from PyQt5.QtGui import QPainter, QColor, QPen, QFont, QLinearGradient
import serial.tools.list_ports
import math

class BoardVisualizer(QWidget):
    def __init__(self):
        super().__init__()
        self.position = 0.0
        self.status = 0
        self.setMinimumHeight(400)
    
    def update_position(self, pos):
        self.position = pos
        self.update()
    
    def update_status(self, status):
        self.status = status
        self.update()
    
    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        
        w = self.width()
        h = self.height()
        
        # Draw board outline
        board_rect = self.rect().adjusted(20, 20, -20, -20)
        
        # Board background gradient
        gradient = QLinearGradient(0, 0, 0, h)
        gradient.setColorAt(0, QColor(30, 80, 150))
        gradient.setColorAt(1, QColor(20, 50, 100))
        painter.setBrush(gradient)
        painter.setPen(QPen(QColor(100, 150, 200), 3))
        painter.drawRoundedRect(board_rect, 20, 20)
        
        # Draw center circle (encoder visualization)
        center_x = w // 2
        center_y = h // 2
        radius = min(w, h) // 4
        
        # Encoder circle
        painter.setBrush(QColor(40, 40, 40))
        painter.setPen(QPen(QColor(200, 200, 200), 2))
        painter.drawEllipse(center_x - radius, center_y - radius, 
                          radius * 2, radius * 2)
        
        # Position indicator
        angle_rad = math.radians(self.position - 90)
        indicator_x = center_x + int(radius * 0.7 * math.cos(angle_rad))
        indicator_y = center_y + int(radius * 0.7 * math.sin(angle_rad))
        
        painter.setPen(QPen(QColor(255, 100, 100), 4))
        painter.drawLine(center_x, center_y, indicator_x, indicator_y)
        
        # Draw position text
        painter.setPen(QColor(255, 255, 255))
        font = QFont('Arial', 24, QFont.Bold)
        painter.setFont(font)
        pos_text = f"{self.position:.1f}°"
        text_rect = painter.fontMetrics().boundingRect(pos_text)
        painter.drawText(center_x - text_rect.width()//2, 
                        center_y + radius + 40, pos_text)
        
        # Status indicators (LEDs)
        led_y = 40
        led_spacing = 30
        
        # Moving LED
        self.draw_led(painter, 40, led_y, "MOVING", 
                     self.status & 0x01, QColor(74, 158, 255))
        
        # Error LED
        self.draw_led(painter, 40, led_y + led_spacing, "ERROR", 
                     self.status & 0x02, QColor(255, 80, 80))
        
        # Homed LED
        self.draw_led(painter, 40, led_y + led_spacing * 2, "HOMED", 
                     self.status & 0x04, QColor(100, 255, 100))
        
        # Enabled LED
        self.draw_led(painter, 40, led_y + led_spacing * 3, "ENABLED", 
                     self.status & 0x08, QColor(255, 200, 0))
    
    def draw_led(self, painter, x, y, label, active, color):
        # LED circle
        if active:
            painter.setBrush(color)
            painter.setPen(QPen(color.lighter(150), 2))
        else:
            painter.setBrush(QColor(40, 40, 40))
            painter.setPen(QPen(QColor(80, 80, 80), 1))
        
        painter.drawEllipse(x, y, 15, 15)
        
        # Label
        painter.setPen(QColor(220, 220, 220))
        font = QFont('Arial', 9, QFont.Bold)
        painter.setFont(font)
        painter.drawText(x + 25, y + 12, label)


class DashboardWidget(QWidget):
    connection_changed = pyqtSignal(bool)
    
    def __init__(self, protocol):
        super().__init__()
        self.protocol = protocol
        self.init_ui()
        
        # Auto-refresh ports
        self.refresh_timer = QTimer()
        self.refresh_timer.timeout.connect(self.refresh_ports)
        self.refresh_timer.start(2000)
        self.refresh_ports()
    
    def init_ui(self):
        layout = QVBoxLayout()
        layout.setContentsMargins(10, 10, 10, 10)
        
        # Connection section
        conn_group = QGroupBox("Connection")
        conn_layout = QVBoxLayout()
        
        port_layout = QHBoxLayout()
        port_layout.addWidget(QLabel("Port:"))
        self.port_combo = QComboBox()
        self.port_combo.setMinimumWidth(250)
        port_layout.addWidget(self.port_combo)
        
        self.connect_btn = QPushButton("Connect")
        self.connect_btn.setMinimumHeight(35)
        self.connect_btn.clicked.connect(self.toggle_connection)
        port_layout.addWidget(self.connect_btn)
        
        conn_layout.addLayout(port_layout)
        
        self.status_label = QLabel("● Not Connected")
        self.status_label.setStyleSheet("color: #ff5555; font-size: 11pt; font-weight: bold;")
        conn_layout.addWidget(self.status_label)
        
        conn_group.setLayout(conn_layout)
        layout.addWidget(conn_group)
        
        # Board visualizer
        visual_group = QGroupBox("Motor Position & Status")
        visual_layout = QVBoxLayout()
        
        self.board_viz = BoardVisualizer()
        visual_layout.addWidget(self.board_viz)
        
        visual_group.setLayout(visual_layout)
        layout.addWidget(visual_group)
        
        # Command log
        log_group = QGroupBox("Command Log")
        log_layout = QVBoxLayout()
        
        self.log_text = QTextEdit()
        self.log_text.setReadOnly(True)
        self.log_text.setMaximumHeight(150)
        log_layout.addWidget(self.log_text)
        
        log_group.setLayout(log_layout)
        layout.addWidget(log_group)
        
        self.setLayout(layout)
    
    def refresh_ports(self):
        current = self.port_combo.currentText()
        self.port_combo.clear()
        
        ports = serial.tools.list_ports.comports()
        for port in ports:
            self.port_combo.addItem(f"{port.device} - {port.description}")
        
        idx = self.port_combo.findText(current)
        if idx >= 0:
            self.port_combo.setCurrentIndex(idx)
    
    def toggle_connection(self):
        if self.protocol.is_connected():
            self.protocol.disconnect()
            self.connect_btn.setText("Connect")
            self.status_label.setText("● Not Connected")
            self.status_label.setStyleSheet("color: #ff5555; font-size: 11pt; font-weight: bold;")
            self.port_combo.setEnabled(True)
            self.connection_changed.emit(False)
        else:
            port_text = self.port_combo.currentText()
            if not port_text:
                return
            
            port = port_text.split(' - ')[0]
            if self.protocol.connect(port):
                self.connect_btn.setText("Disconnect")
                self.status_label.setText("● Connected")
                self.status_label.setStyleSheet("color: #55ff55; font-size: 11pt; font-weight: bold;")
                self.port_combo.setEnabled(False)
                self.connection_changed.emit(True)
                self.log_command("Connected to device")
                
                version = self.protocol.get_version()
                self.log_command(f"Firmware: {version}")
    
    def update_position(self, pos):
        self.board_viz.update_position(pos)
    
    def update_status(self, status):
        self.board_viz.update_status(status)
    
    def log_command(self, message):
        self.log_text.append(f"→ {message}")
        scrollbar = self.log_text.verticalScrollBar()
        scrollbar.setValue(scrollbar.maximum())
