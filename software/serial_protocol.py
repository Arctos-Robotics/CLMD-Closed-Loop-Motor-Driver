import serial
import time
import logging

logger = logging.getLogger(__name__)

class MotorConfigProtocol:
    def __init__(self, port=None, baudrate=115200):
        self.ser = None
        self.port = port
        self.baudrate = baudrate
        
    def connect(self, port):
        """Connect to serial port"""
        try:
            if self.ser and self.ser.is_open:
                self.ser.close()
            
            self.ser = serial.Serial(port, self.baudrate, timeout=1)
            time.sleep(2)  # Wait for ESP32 to boot
            logger.info(f"Connected to {port}")
            return True
        except Exception as e:
            logger.error(f"Connection failed: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from serial port"""
        if self.ser and self.ser.is_open:
            self.ser.close()
            logger.info("Disconnected")
    
    def is_connected(self):
        """Check if connected"""
        return self.ser and self.ser.is_open
    
    def send_command(self, command):
        """Send command and wait for response"""
        if not self.is_connected():
            return "ERROR Not connected"
        
        try:
            self.ser.write(f"{command}\n".encode())
            response = self.ser.readline().decode().strip()
            logger.debug(f"TX: {command} | RX: {response}")
            return response
        except Exception as e:
            logger.error(f"Command failed: {e}")
            return f"ERROR {str(e)}"
    
    # Motor control commands
    def enable_motor(self):
        return self.send_command("ENABLE")
    
    def disable_motor(self):
        return self.send_command("DISABLE")
    
    def move_absolute(self, degrees, rpm):
        return self.send_command(f"MOVE_ABS {degrees} {rpm}")
    
    def move_relative(self, degrees, rpm):
        return self.send_command(f"MOVE_REL {degrees} {rpm}")
    
    def stop(self):
        return self.send_command("STOP")
    
    def set_zero(self):
        return self.send_command("SET_ZERO")
    
    # Query commands
    def get_position(self):
        response = self.send_command("GET_POS")
        if response.startswith("POS"):
            try:
                return float(response.split()[1])
            except:
                return None
        return None
    
    def get_status(self):
        response = self.send_command("GET_STATUS")
        if response.startswith("STATUS"):
            try:
                return int(response.split()[1], 16)
            except:
                return None
        return None
    
    def get_version(self):
        return self.send_command("VERSION")
    
    # Configuration commands
    def set_can_id(self, can_id):
        return self.send_command(f"SET_ID {can_id}")
    
    def save_config(self):
        return self.send_command("SAVE")
    
    def load_config(self):
        return self.send_command("LOAD")

    # Advanced commands
    def set_pid(self, kp, ki, kd):
        return self.send_command(f"SET_PID {kp} {ki} {kd}")
    
    def set_led(self, r, g, b):
        return self.send_command(f"LED {r} {g} {b}")
    
    def get_endstops(self):
        response = self.send_command("GET_ENDSTOPS")
        if response.startswith("ENDSTOPS"):
            try:
                parts = response.split()
                return (int(parts[1]), int(parts[2]))
            except:
                return None
        return None
    
    def get_encoder_raw(self):
        response = self.send_command("GET_ENCODER")
        if response.startswith("ENCODER"):
            try:
                return int(response.split()[1])
            except:
                return None
        return None
