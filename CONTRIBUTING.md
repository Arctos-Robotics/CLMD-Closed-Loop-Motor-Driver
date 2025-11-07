# Contributing to Closed-Loop Stepper Motor Driver

Thank you for your interest in contributing! This project welcomes contributions from everyone.

## How to Contribute

### Reporting Bugs

If you find a bug, please create an issue with:
- Clear description of the problem
- Steps to reproduce
- Expected vs actual behavior
- Hardware/software versions
- Relevant logs or screenshots

### Suggesting Features

Feature requests are welcome! Please:
- Check if the feature already exists or is planned
- Describe the use case and benefits
- Provide examples if possible

### Code Contributions

1. **Fork the repository**
   ```bash
   git clone https://github.com/yourusername/closed-loop-driver.git
   cd closed-loop-driver
   ```

2. **Create a feature branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

3. **Make your changes**
   - Follow existing code style
   - Add comments for complex logic
   - Update documentation if needed

4. **Test your changes**
   - Test firmware on actual hardware
   - Test configuration tool on Windows/Linux/Mac
   - Verify no regressions

5. **Commit your changes**
   ```bash
   git add .
   git commit -m "Add feature: description"
   ```

6. **Push to your fork**
   ```bash
   git push origin feature/your-feature-name
   ```

7. **Create a Pull Request**
   - Describe what your PR does
   - Reference any related issues
   - Include test results

## Development Guidelines

### Firmware (C/ESP-IDF)

- Follow ESP-IDF coding style
- Use meaningful variable names
- Add comments for hardware interactions
- Keep functions focused and small
- Test on actual hardware before submitting

### Configuration Tool (Python/PyQt5)

- Follow PEP 8 style guide
- Use type hints where appropriate
- Add docstrings to functions
- Test on multiple platforms
- Ensure UI remains responsive

### Hardware

- Document any design changes
- Provide rationale for component changes
- Update BOM and assembly notes
- Test prototypes before submitting

### Documentation

- Use clear, concise language
- Include examples and screenshots
- Update table of contents
- Check for broken links
- Proofread for typos

## Code Style

### Firmware (C)
```c
// Use descriptive names
float calculate_position_error(float target, float current);

// Comment hardware interactions
gpio_set_level(TMC_STEP_PIN, 1);  // Generate step pulse

// Use constants for magic numbers
#define ENCODER_RESOLUTION 32768
#define CONTROL_LOOP_HZ 1000
```

### Python
```python
# Use type hints
def move_absolute(self, position: float, speed: int) -> bool:
    """Move to absolute position.
    
    Args:
        position: Target position in degrees
        speed: Speed in RPM
        
    Returns:
        True if command sent successfully
    """
    pass

# Use descriptive names
current_position_deg = self.read_encoder()
```

## Testing

### Firmware Testing
- Build without warnings
- Test on ESP32-S3 hardware
- Verify all features work
- Check for memory leaks
- Test error handling

### Software Testing
- Test on Windows, Linux, and Mac
- Verify all UI elements work
- Test with real hardware
- Check for exceptions
- Test edge cases

### Hardware Testing
- Verify schematic correctness
- Check PCB layout (DRC)
- Test prototype boards
- Measure power consumption
- Verify signal integrity

## Areas for Contribution

### High Priority
- Bug fixes
- Documentation improvements
- Test coverage
- Performance optimization

### Medium Priority
- New features
- Hardware variants
- Example projects
- Tutorials

### Low Priority
- Code refactoring
- UI improvements
- Additional language support

## Questions?

- Open a discussion on GitHub
- Check existing issues and PRs
- Review documentation

## Code of Conduct

- Be respectful and inclusive
- Provide constructive feedback
- Help others learn
- Focus on the project goals

## License

By contributing, you agree that your contributions will be licensed under the MIT License (for software) and CERN-OHL-P v2 (for hardware).

---

Thank you for contributing to the open-source robotics community! 🤖
