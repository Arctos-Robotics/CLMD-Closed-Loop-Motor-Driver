#!/usr/bin/env python3
import sys
import logging
from PyQt5.QtWidgets import QApplication
from main_window import MainWindow

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)

def main():
    app = QApplication(sys.argv)
    app.setApplicationName("Closed Loop Motor Configuration")
    app.setOrganizationName("MotorDriver")
    
    window = MainWindow()
    window.show()
    
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
