from PyQt5.QtWidgets import (QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
                             QMessageBox, QSplitter, QTabWidget)
from PyQt5.QtCore import QTimer, Qt
from PyQt5.QtGui import QIcon
from serial_protocol import MotorConfigProtocol
from dashboard_widget import DashboardWidget
from control_panel import ControlPanel
from settings_panel import SettingsPanel
from pid_tuning_panel import PIDTuningPanel
from peripherals_panel import PeripheralsPanel
from tmc_diagnostics_panel import TMCDiagnosticsPanel
from firmware_upload_panel import FirmwareUploadPanel
import logging

logger = logging.getLogger(__name__)

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.protocol = MotorConfigProtocol()
        self.init_ui()
        self.apply_stylesheet()
        
        # Status update timer
        self.status_timer = QTimer()
        self.status_timer.timeout.connect(self.update_status)
        self.status_timer.start(100)  # Update every 100ms
    
    def init_ui(self):
        self.setWindowTitle("Closed Loop Stepper Driver - Configuration Tool")
        self.setGeometry(100, 100, 1400, 900)
        
        # Main container
        main_widget = QWidget()
        main_layout = QHBoxLayout()
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)
        
        # Left panel - Dashboard
        self.dashboard = DashboardWidget(self.protocol)
        
        # Right panel - Tabbed interface for different features
        right_tabs = QTabWidget()
        right_tabs.setStyleSheet("""
            QTabWidget::pane {
                border: 2px solid #3d3d3d;
                border-radius: 8px;
                background-color: #2d2d2d;
            }
            QTabBar::tab {
                background-color: #3d3d3d;
                color: #e0e0e0;
                padding: 10px 20px;
                margin-right: 2px;
                border-top-left-radius: 6px;
                border-top-right-radius: 6px;
            }
            QTabBar::tab:selected {
                background-color: #4a9eff;
                color: white;
                font-weight: bold;
            }
            QTabBar::tab:hover {
                background-color: #4d4d4d;
            }
        """)
        
        # Create panels
        self.control_panel = ControlPanel(self.protocol)
        self.settings_panel = SettingsPanel(self.protocol)
        self.pid_tuning_panel = PIDTuningPanel(self.protocol)
        self.peripherals_panel = PeripheralsPanel(self.protocol)
        self.tmc_panel = TMCDiagnosticsPanel(self.protocol)
        self.firmware_panel = FirmwareUploadPanel(self.protocol)
        
        # Add tabs
        right_tabs.addTab(self.control_panel, "🎮 Control")
        right_tabs.addTab(self.pid_tuning_panel, "📊 PID Tuning")
        right_tabs.addTab(self.tmc_panel, "⚡ TMC Diagnostics")
        right_tabs.addTab(self.peripherals_panel, "🔧 Peripherals")
        right_tabs.addTab(self.settings_panel, "⚙️ Settings")
        right_tabs.addTab(self.firmware_panel, "📤 Firmware Upload")
        
        # Splitter for resizable panels
        splitter = QSplitter(Qt.Horizontal)
        splitter.addWidget(self.dashboard)
        splitter.addWidget(right_tabs)
        splitter.setStretchFactor(0, 2)
        splitter.setStretchFactor(1, 3)
        
        main_layout.addWidget(splitter)
        main_widget.setLayout(main_layout)
        self.setCentralWidget(main_widget)
        
        # Connect signals
        self.dashboard.connection_changed.connect(self.on_connection_changed)
        self.control_panel.command_sent.connect(self.dashboard.log_command)
        self.pid_tuning_panel.command_sent.connect(self.dashboard.log_command)
        self.tmc_panel.command_sent.connect(self.dashboard.log_command)
        self.peripherals_panel.command_sent.connect(self.dashboard.log_command)
        self.firmware_panel.command_sent.connect(self.dashboard.log_command)
    
    def on_connection_changed(self, connected):
        if connected:
            logger.info("Device connected")
            self.control_panel.set_connected(True)
            self.settings_panel.set_connected(True)
            self.pid_tuning_panel.set_connected(True)
            self.tmc_panel.set_connected(True)
            self.peripherals_panel.set_connected(True)
            self.firmware_panel.set_connected(True)
        else:
            logger.info("Device disconnected")
            self.control_panel.set_connected(False)
            self.settings_panel.set_connected(False)
            self.pid_tuning_panel.set_connected(False)
            self.tmc_panel.set_connected(False)
            self.peripherals_panel.set_connected(False)
            self.firmware_panel.set_connected(False)
    
    def update_status(self):
        if self.protocol.is_connected():
            # Update dashboard
            position = self.protocol.get_position()
            status = self.protocol.get_status()
            
            if position is not None:
                self.dashboard.update_position(position)
            
            if status is not None:
                self.dashboard.update_status(status)
                self.control_panel.update_status(status)
    
    def apply_stylesheet(self):
        self.setStyleSheet("""
            QMainWindow {
                background-color: #1e1e1e;
            }
            QWidget {
                color: #e0e0e0;
                font-family: 'Segoe UI', Arial, sans-serif;
                font-size: 10pt;
            }
            QGroupBox {
                border: 2px solid #3d3d3d;
                border-radius: 8px;
                margin-top: 12px;
                padding-top: 10px;
                background-color: #2d2d2d;
                font-weight: bold;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 15px;
                padding: 0 5px;
                color: #4a9eff;
            }
            QPushButton {
                background-color: #0d47a1;
                color: white;
                border: none;
                border-radius: 6px;
                padding: 10px 20px;
                font-weight: bold;
                font-size: 10pt;
            }
            QPushButton:hover {
                background-color: #1565c0;
            }
            QPushButton:pressed {
                background-color: #0a3d91;
            }
            QPushButton:disabled {
                background-color: #3d3d3d;
                color: #808080;
            }
            QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {
                background-color: #3d3d3d;
                border: 2px solid #4d4d4d;
                border-radius: 5px;
                padding: 8px;
                color: #e0e0e0;
            }
            QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus {
                border: 2px solid #4a9eff;
            }
            QLabel {
                color: #e0e0e0;
            }
            QTextEdit {
                background-color: #252525;
                border: 2px solid #3d3d3d;
                border-radius: 5px;
                color: #e0e0e0;
                font-family: 'Consolas', 'Courier New', monospace;
            }
            QProgressBar {
                border: 2px solid #3d3d3d;
                border-radius: 5px;
                text-align: center;
                background-color: #2d2d2d;
            }
            QProgressBar::chunk {
                background-color: #4a9eff;
                border-radius: 3px;
            }
        """)
    
    def closeEvent(self, event):
        if self.protocol.is_connected():
            self.protocol.disconnect()
        event.accept()
