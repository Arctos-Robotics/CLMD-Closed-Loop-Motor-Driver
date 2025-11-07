from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QGroupBox,
                             QComboBox, QPushButton, QLabel, QTextEdit)
from PyQt5.QtCore import pyqtSignal, QTimer
import serial.tools.list_ports
import logging

logger = logging.getLogger(__name__)

class ConnectionTab(QWidget):
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
        
        # Connection group
        conn_group = QGroupBox("Serial Connection")
        conn_layout = QVBoxLayout()
        
        # Port selection
        port_layout = QHBoxLayout()
        port_layout.addWidget(QLabel("COM Port:"))
        self.port_combo = QComboBox()
        self.port_combo.setMinimumWidth(200)
        port_layout.addWidget(self.port_combo)
        
        self.refresh_btn = QPushButton("Refresh")
        self.refresh_btn.clicked.connect(self.refresh_ports)
        port_layout.addWidget(self.refresh_btn)
        port_layout.addStretch()
        
        conn_layout.addLayout(port_layout)
        
        # Connect button
        btn_layout = QHBoxLayout()
        self.connect_btn = QPushButton("Connect")
        self.connect_btn.setMinimumHeight(40)
        self.connect_btn.clicked.connect(self.toggle_connection)
        btn_layout.addWidget(self.connect_btn)
        
        self.disconnect_btn = QPushButton("Disconnect")
        self.disconnect_btn.setMinimumHeight(40)
        self.disconnect_btn.clicked.connect(self.disconnect)
        self.disconnect_btn.setEnabled(False)
        btn_layout.addWidget(self.disconnect_btn)
        
        conn_layout.addLayout(btn_layout)
        
        # Status
        self.status_label = QLabel("Status: Not connected")
        conn_layout.addWidget(self.status_label)
        
        conn_group.setLayout(conn_layout)
        layout.addWidget(conn_group)
        
        # Device info group
        info_group = QGroupBox("Device Information")
        info_layout = QVBoxLayout()
        
        self.info_text = QTextEdit()
        self.info_text.setReadOnly(True)
        self.info_text.setMaximumHeight(150)
        info_layout.addWidget(self.info_text)
        
        self.get_info_btn = QPushButton("Get Device Info")
        self.get_info_btn.clicked.connect(self.get_device_info)
        self.get_info_btn.setEnabled(False)
        info_layout.addWidget(self.get_info_btn)
        
        info_group.setLayout(info_layout)
        layout.addWidget(info_group)
        
        layout.addStretch()
        self.setLayout(layout)
    
    def refresh_ports(self):
        current_port = self.port_combo.currentText()
        self.port_combo.clear()
        
        ports = serial.tools.list_ports.comports()
        for port in ports:
            self.port_combo.addItem(f"{port.device} - {port.description}")
        
        # Restore selection if still available
        index = self.port_combo.findText(current_port)
        if index >= 0:
            self.port_combo.setCurrentIndex(index)
    
    def toggle_connection(self):
        port_text = self.port_combo.currentText()
        if not port_text:
            self.status_label.setText("Status: No port selected")
            return
        
        port = port_text.split(' - ')[0]
        
        if self.protocol.connect(port):
            self.status_label.setText(f"Status: Connected to {port}")
            self.connect_btn.setEnabled(False)
            self.disconnect_btn.setEnabled(True)
            self.get_info_btn.setEnabled(True)
            self.port_combo.setEnabled(False)
            self.connection_changed.emit(True)
            self.get_device_info()
        else:
            self.status_label.setText("Status: Connection failed")
    
    def disconnect(self):
        self.protocol.disconnect()
        self.status_label.setText("Status: Disconnected")
        self.connect_btn.setEnabled(True)
        self.disconnect_btn.setEnabled(False)
        self.get_info_btn.setEnabled(False)
        self.port_combo.setEnabled(True)
        self.info_text.clear()
        self.connection_changed.emit(False)
    
    def get_device_info(self):
        if not self.protocol.is_connected():
            return
        
        info = []
        
        version = self.protocol.get_version()
        info.append(f"Firmware: {version}")
        
        status = self.protocol.get_status()
        if status is not None:
            info.append(f"\nStatus Flags: 0x{status:02X}")
            info.append(f"  Moving: {'Yes' if status & 0x01 else 'No'}")
            info.append(f"  Error: {'Yes' if status & 0x02 else 'No'}")
            info.append(f"  Homed: {'Yes' if status & 0x04 else 'No'}")
            info.append(f"  Enabled: {'Yes' if status & 0x08 else 'No'}")
        
        position = self.protocol.get_position()
        if position is not None:
            info.append(f"\nCurrent Position: {position:.2f}°")
        
        self.info_text.setText('\n'.join(info))
