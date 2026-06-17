# Legacy Versions

This directory contains historical versions of the ESP32-based water level monitoring system. Each version represents a development milestone, including new features, hardware changes, performance improvements, and bug fixes implemented throughout the project.

> **Note:** These versions are preserved for historical and reference purposes only. The latest stable release can be found in the main project directory.

| Version | Description                                                                                                                                                                                                            |
| ------- | ------------------------------------------------------------------------------------------ |
| v0.1.0  | Initial implementation. Measures water depth only using Josh's formula.                                                                                                                                                |
| v0.1.2  | Added a tank height parameter to calculate the water level percentage.                                                                                                                                                 |
| v0.1.9  | Added dynamic configuration variables through Blynk for tank height (V3) and supply voltage (V4). Also included `double` variables for storing the sensor's geographic coordinates.                                    |
| v0.2.0  | Modified input pin assignments. Added a button to switch between "Depth" and "Flow Rate" display modes. Introduced flow measurement functionality.                                                                     |
| v0.2.2  | Restored the original pin configuration. Added virtual pins for configuring supply voltage, tank height, location, tolerance, and calibration constants through Blynk.                                                 |
| v0.2.3  | Defined default calibration parameters for depth measurement: `slope_air`, `slope_h2o`, `tol`, `Vs`, and `total`.                                                                                                      |
| v0.2.4  | Reduced the sampling interval to 2 ms to improve internet connectivity performance.                                                                                                                                    |
| v0.2.5  | Added a reset counter displayed on the screen when the reset button is pressed. Introduced an RGB LED for internet connection status indication. Reassigned the reset button to GPIO33.                                |
| v0.2.6  | Updated the RGB LED status color scheme.                                                                                                                                                                               |
| v0.3.0  | Added offline operation capability. Included "Starting" and "Connection Successful" messages on the LCD.                                                                                                               |
| v0.3.1  | Changed RGB LED pins to D18, D19, and D23 and configured it as a common-anode device. Added additional configuration messages to the LCD.                                                                              |
| v0.3.2  | Added a second RGB LED with a red-to-green gradient indicating water level percentage. Added a buzzer alarm that activates intermittently when the water level drops below 10%.                                        |
| v0.3.3  | Replaced `delay()`-based timing with hardware timers for pressure measurement and display updates. Configured the water sensor pin with an internal `PULL_DOWN` resistor to prevent random readings when disconnected. |
| v0.4.0  | General code cleanup and refactoring. Removed unused code, added detailed comments, and introduced virtual pins V11 and V12 for transmitting measured sensor voltages.                                                 |
| v0.4.1  | Notification messages are now generated directly from the firmware using the `events()` function.                                                                                                                      |
| v0.4.2  | Minor bug fixes and code improvements.                                                                                                                                                                                 |
| v0.5.1  | Updated Blynk libraries. No changes were made to the main application code.                                                                                                                                            |
| v0.5.2  | Added calibration values for both pressure sensors.                                                                                                                                                                    |
| v0.5.6  | Implemented dual-core processing on the ESP32. One core handles sensor measurements while the other manages network connectivity, allowing continuous operation even when Wi-Fi is unavailable.                        |
