from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QGroupBox,
                             QLabel, QDoubleSpinBox, QPushButton, QSlider,
                             QCheckBox, QComboBox)
from PyQt5.QtCore import pyqtSignal, QTimer, Qt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure
import numpy as np

class PIDTuningPanel(QWidget):
    command_sent = pyqtSignal(str)
    
    def __init__(self, protocol):
        super().__init__()
        self.protocol = protocol
        self.position_history = []
        self.target_history = []
        self.error_history = []
        self.time_history = []
        self.start_time = 0
        self.recording = False
        
        self.init_ui()
        
        # Update timer for plotting
        self.plot_timer = QTimer()
        self.plot_timer.timeout.connect(self.update_plot)
    
    def init_ui(self):
        layout = QVBoxLayout()
        
        # PID Parameters
        pid_group = QGroupBox("PID Parameters")
        pid_layout = QVBoxLayout()
        
        # Kp
        kp_layout = QHBoxLayout()
        kp_layout.addWidget(QLabel("Kp (Proportional):"))
        self.kp_spin = QDoubleSpinBox()
        self.kp_spin.setRange(0, 100)
        self.kp_spin.setValue(2.0)
        self.kp_spin.setSingleStep(0.1)
        self.kp_spin.setDecimals(2)
        self.kp_spin.setMinimumHeight(35)
        kp_layout.addWidget(self.kp_spin)
        
        self.kp_slider = QSlider(Qt.Horizontal)
        self.kp_slider.setRange(0, 1000)
        self.kp_slider.setValue(200)
        self.kp_slider.valueChanged.connect(lambda v: self.kp_spin.setValue(v/100))
        self.kp_spin.valueChanged.connect(lambda v: self.kp_slider.setValue(int(v*100)))
        kp_layout.addWidget(self.kp_slider)
        pid_layout.addLayout(kp_layout)
        
        # Ki
        ki_layout = QHBoxLayout()
        ki_layout.addWidget(QLabel("Ki (Integral):"))
        self.ki_spin = QDoubleSpinBox()
        self.ki_spin.setRange(0, 10)
        self.ki_spin.setValue(0.1)
        self.ki_spin.setSingleStep(0.01)
        self.ki_spin.setDecimals(3)
        self.ki_spin.setMinimumHeight(35)
        ki_layout.addWidget(self.ki_spin)
        
        self.ki_slider = QSlider(Qt.Horizontal)
        self.ki_slider.setRange(0, 1000)
        self.ki_slider.setValue(100)
        self.ki_slider.valueChanged.connect(lambda v: self.ki_spin.setValue(v/1000))
        self.ki_spin.valueChanged.connect(lambda v: self.ki_slider.setValue(int(v*1000)))
        ki_layout.addWidget(self.ki_slider)
        pid_layout.addLayout(ki_layout)
        
        # Kd
        kd_layout = QHBoxLayout()
        kd_layout.addWidget(QLabel("Kd (Derivative):"))
        self.kd_spin = QDoubleSpinBox()
        self.kd_spin.setRange(0, 10)
        self.kd_spin.setValue(0.05)
        self.kd_spin.setSingleStep(0.01)
        self.kd_spin.setDecimals(3)
        self.kd_spin.setMinimumHeight(35)
        kd_layout.addWidget(self.kd_spin)
        
        self.kd_slider = QSlider(Qt.Horizontal)
        self.kd_slider.setRange(0, 1000)
        self.kd_slider.setValue(50)
        self.kd_slider.valueChanged.connect(lambda v: self.kd_spin.setValue(v/1000))
        self.kd_spin.valueChanged.connect(lambda v: self.kd_slider.setValue(int(v*1000)))
        kd_layout.addWidget(self.kd_slider)
        pid_layout.addLayout(kd_layout)
        
        # Apply button
        apply_layout = QHBoxLayout()
        self.apply_pid_btn = QPushButton("Apply PID Parameters")
        self.apply_pid_btn.setMinimumHeight(40)
        self.apply_pid_btn.clicked.connect(self.apply_pid)
        self.apply_pid_btn.setEnabled(False)
        apply_layout.addWidget(self.apply_pid_btn)
        
        self.reset_pid_btn = QPushButton("Reset to Defaults")
        self.reset_pid_btn.setMinimumHeight(40)
        self.reset_pid_btn.clicked.connect(self.reset_pid)
        apply_layout.addWidget(self.reset_pid_btn)
        pid_layout.addLayout(apply_layout)
        
        pid_group.setLayout(pid_layout)
        layout.addWidget(pid_group)
        
        # Test controls
        test_group = QGroupBox("Step Response Test")
        test_layout = QVBoxLayout()
        
        test_params = QHBoxLayout()
        test_params.addWidget(QLabel("Step Size:"))
        self.step_size_spin = QDoubleSpinBox()
        self.step_size_spin.setRange(1, 180)
        self.step_size_spin.setValue(45)
        self.step_size_spin.setSuffix(" °")
        self.step_size_spin.setMinimumHeight(35)
        test_params.addWidget(self.step_size_spin)
        
        test_params.addWidget(QLabel("Duration:"))
        self.duration_spin = QDoubleSpinBox()
        self.duration_spin.setRange(0.5, 10)
        self.duration_spin.setValue(3.0)
        self.duration_spin.setSuffix(" s")
        self.duration_spin.setMinimumHeight(35)
        test_params.addWidget(self.duration_spin)
        test_layout.addLayout(test_params)
        
        test_btn_layout = QHBoxLayout()
        self.start_test_btn = QPushButton("▶ Start Test")
        self.start_test_btn.setMinimumHeight(40)
        self.start_test_btn.clicked.connect(self.start_test)
        self.start_test_btn.setEnabled(False)
        test_btn_layout.addWidget(self.start_test_btn)
        
        self.stop_test_btn = QPushButton("⏹ Stop Test")
        self.stop_test_btn.setMinimumHeight(40)
        self.stop_test_btn.clicked.connect(self.stop_test)
        self.stop_test_btn.setEnabled(False)
        test_btn_layout.addWidget(self.stop_test_btn)
        
        self.clear_plot_btn = QPushButton("Clear Plot")
        self.clear_plot_btn.setMinimumHeight(40)
        self.clear_plot_btn.clicked.connect(self.clear_plot)
        test_btn_layout.addWidget(self.clear_plot_btn)
        test_layout.addLayout(test_btn_layout)
        
        test_group.setLayout(test_layout)
        layout.addWidget(test_group)
        
        # Plot
        plot_group = QGroupBox("Response Plot")
        plot_layout = QVBoxLayout()
        
        self.figure = Figure(figsize=(8, 4), facecolor='#2d2d2d')
        self.canvas = FigureCanvas(self.figure)
        self.ax = self.figure.add_subplot(111)
        self.ax.set_facecolor('#1e1e1e')
        self.ax.tick_params(colors='#e0e0e0')
        self.ax.spines['bottom'].set_color('#4d4d4d')
        self.ax.spines['top'].set_color('#4d4d4d')
        self.ax.spines['left'].set_color('#4d4d4d')
        self.ax.spines['right'].set_color('#4d4d4d')
        self.ax.set_xlabel('Time (s)', color='#e0e0e0')
        self.ax.set_ylabel('Position (°)', color='#e0e0e0')
        self.ax.set_title('Step Response', color='#4a9eff', fontweight='bold')
        self.ax.grid(True, alpha=0.2, color='#4d4d4d')
        
        plot_layout.addWidget(self.canvas)
        plot_group.setLayout(plot_layout)
        layout.addWidget(plot_group)
        
        # Metrics
        metrics_group = QGroupBox("Performance Metrics")
        metrics_layout = QHBoxLayout()
        
        self.overshoot_label = QLabel("Overshoot: --")
        self.settling_time_label = QLabel("Settling Time: --")
        self.steady_error_label = QLabel("Steady Error: --")
        
        metrics_layout.addWidget(self.overshoot_label)
        metrics_layout.addWidget(self.settling_time_label)
        metrics_layout.addWidget(self.steady_error_label)
        
        metrics_group.setLayout(metrics_layout)
        layout.addWidget(metrics_group)
        
        self.setLayout(layout)
    
    def set_connected(self, connected):
        self.apply_pid_btn.setEnabled(connected)
        self.start_test_btn.setEnabled(connected)
    
    def apply_pid(self):
        kp = self.kp_spin.value()
        ki = self.ki_spin.value()
        kd = self.kd_spin.value()
        
        response = self.protocol.send_command(f"SET_PID {kp} {ki} {kd}")
        self.command_sent.emit(f"Set PID: Kp={kp}, Ki={ki}, Kd={kd} - {response}")
    
    def reset_pid(self):
        self.kp_spin.setValue(2.0)
        self.ki_spin.setValue(0.1)
        self.kd_spin.setValue(0.05)
    
    def start_test(self):
        self.recording = True
        self.position_history = []
        self.target_history = []
        self.error_history = []
        self.time_history = []
        self.start_time = 0
        
        # Start recording
        self.plot_timer.start(50)  # 20 Hz
        
        # Send step command
        step = self.step_size_spin.value()
        response = self.protocol.move_relative(step, 400)
        self.command_sent.emit(f"Step test started: {step}° - {response}")
        
        self.start_test_btn.setEnabled(False)
        self.stop_test_btn.setEnabled(True)
        
        # Auto-stop after duration
        duration_ms = int(self.duration_spin.value() * 1000)
        QTimer.singleShot(duration_ms, self.stop_test)
    
    def stop_test(self):
        self.recording = False
        self.plot_timer.stop()
        self.start_test_btn.setEnabled(True)
        self.stop_test_btn.setEnabled(False)
        self.calculate_metrics()
    
    def update_plot(self):
        if not self.recording or not self.protocol.is_connected():
            return
        
        import time
        if self.start_time == 0:
            self.start_time = time.time()
        
        current_time = time.time() - self.start_time
        position = self.protocol.get_position()
        
        if position is not None:
            self.time_history.append(current_time)
            self.position_history.append(position)
            
            # Estimate target (last position + step)
            if len(self.position_history) > 1:
                target = self.position_history[0] + self.step_size_spin.value()
            else:
                target = position
            
            self.target_history.append(target)
            self.error_history.append(target - position)
            
            # Update plot
            self.ax.clear()
            self.ax.plot(self.time_history, self.position_history, 
                        'c-', linewidth=2, label='Actual')
            self.ax.plot(self.time_history, self.target_history, 
                        'r--', linewidth=1.5, label='Target')
            self.ax.set_xlabel('Time (s)', color='#e0e0e0')
            self.ax.set_ylabel('Position (°)', color='#e0e0e0')
            self.ax.set_title('Step Response', color='#4a9eff', fontweight='bold')
            self.ax.legend(facecolor='#2d2d2d', edgecolor='#4d4d4d', 
                          labelcolor='#e0e0e0')
            self.ax.grid(True, alpha=0.2, color='#4d4d4d')
            self.ax.set_facecolor('#1e1e1e')
            self.ax.tick_params(colors='#e0e0e0')
            self.canvas.draw()
    
    def clear_plot(self):
        self.position_history = []
        self.target_history = []
        self.error_history = []
        self.time_history = []
        self.ax.clear()
        self.ax.set_xlabel('Time (s)', color='#e0e0e0')
        self.ax.set_ylabel('Position (°)', color='#e0e0e0')
        self.ax.set_title('Step Response', color='#4a9eff', fontweight='bold')
        self.ax.grid(True, alpha=0.2, color='#4d4d4d')
        self.canvas.draw()
        
        self.overshoot_label.setText("Overshoot: --")
        self.settling_time_label.setText("Settling Time: --")
        self.steady_error_label.setText("Steady Error: --")
    
    def calculate_metrics(self):
        if len(self.position_history) < 10:
            return
        
        target = self.target_history[-1]
        positions = np.array(self.position_history)
        
        # Overshoot
        max_pos = np.max(positions)
        overshoot = ((max_pos - target) / abs(self.step_size_spin.value())) * 100
        self.overshoot_label.setText(f"Overshoot: {overshoot:.1f}%")
        
        # Settling time (2% criterion)
        settling_band = abs(target) * 0.02
        settled = False
        settling_time = 0
        for i in range(len(positions)-1, 0, -1):
            if abs(positions[i] - target) > settling_band:
                settling_time = self.time_history[i]
                break
        self.settling_time_label.setText(f"Settling Time: {settling_time:.2f}s")
        
        # Steady-state error
        steady_error = abs(positions[-1] - target)
        self.steady_error_label.setText(f"Steady Error: {steady_error:.2f}°")
