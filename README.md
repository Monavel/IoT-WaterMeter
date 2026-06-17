# IoT - Water Level and Consumption Monitoring System

An ESP32-based IoT monitoring system designed for real-time measurement of water tank level and water consumption. The system uses differential pressure sensors to estimate water depth and a pulse-based flow sensor to track water usage. Measurements can be monitored locally through an LCD display and remotely using the Blynk IoT platform.

---

## Overview

This project was developed as a low-cost solution for continuous water storage monitoring in residential and small-scale infrastructure applications.

The system provides:

* Water level measurement in meters.
* Tank filling percentage calculation.
* Real-time flow rate monitoring.
* Accumulated water consumption tracking.
* Remote monitoring through Blynk IoT.
* Local visualization using a 20×4 LCD.
* Automatic daily consumption reset.
* Offline operation when internet connectivity is unavailable.

---

## Main Features

### Water Level Measurement

Water level is estimated using two pressure sensors:

* Atmospheric pressure reference sensor.
* Water pressure sensor.

The differential pressure is converted into water column height using calibration parameters and supply voltage compensation.

### Water Consumption Monitoring

A pulse-based flow sensor measures:

* Instantaneous flow rate (L/min).
* Total accumulated water consumption (L).

### Remote Monitoring

The ESP32 communicates with the Blynk IoT platform, allowing:

* Real-time visualization.
* Remote configuration.
* Data logging.
* Mobile access.

### Local Display

A 20×4 I2C LCD provides local information including:

* Water level.
* Tank percentage.
* Flow rate.
* Total water consumption.

### Persistent Calibration

Calibration offsets are stored in the ESP32 non-volatile memory and remain available after power cycles.

### Dual-Core Processing

The firmware uses both ESP32 cores:

* Core 0: Wi-Fi and Blynk communication.
* Core 1: Sensor acquisition and local display.

This architecture improves system responsiveness and reliability.

---

## Hardware Requirements

### Microcontroller

* ESP32 Development Board

### Sensors

* 2 × Analog pressure sensors
* 1 × Hall-effect flow sensor

### Display

* 20×4 LCD with I2C interface

### Additional Components

* Push button for zero-level calibration
* Display mode selection switch
* Power supply (5 V)

---

## Pin Configuration

| Function              | GPIO   |
| --------------------- | ------ |
| Calibration Button    | GPIO25 |
| Display Mode Switch   | GPIO26 |
| Flow Sensor           | GPIO27 |
| Reset Input           | GPIO33 |
| Air Pressure Sensor   | GPIO34 |
| Water Pressure Sensor | GPIO35 |

---

## Software Dependencies

Required Arduino libraries:

* Blynk
* BlynkEdgent
* LiquidCrystal_I2C
* TimeLib
* WidgetRTC
* Preferences

Install the libraries through Arduino IDE Library Manager before compiling.

---

## Blynk Configuration

Create your own Blynk Template and replace the placeholder values in the source code:

```cpp
#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_TEMPLATE_NAME"
```

Configure the following virtual pins:

| Virtual Pin | Description                 |
| ----------- | --------------------------- |
| V0          | Water level (m)             |
| V1          | Tank filling percentage (%) |
| V3          | Tank height                 |
| V4          | Supply voltage              |
| V5          | Flow rate (L/min)           |
| V6          | Total consumption (L)       |
| V8          | Flow calibration constant   |
| V9          | Daily consumption           |
| V11         | Air sensor voltage          |
| V12         | Water sensor voltage        |

---

## Repository Structure

```text
.
├── current_version/
│   └── firmware_v0.6.9
├── legacy_versions/
│   ├── v0.1.0
│   ├── v0.1.2
│   ├── ...
│   └── v0.5.6
├── hardware/
├── images/
├── documentation/
└── README.md
```

---

## Version History

The complete development history is available in the `legacy_versions` directory.

Current stable version:

**Firmware v0.6.9**

Main characteristics:

* Differential pressure level measurement.
* Flow rate monitoring.
* Dual-core ESP32 processing.
* Offline operation support.
* Automatic daily consumption reset.
* Remote monitoring using Blynk IoT.
* LCD local interface.

---

## Applications

This system can be adapted for:

* Residential water tanks.
* Water storage monitoring.
* Smart home applications.
* Educational IoT projects.
* Environmental monitoring systems.
* Scientific instrumentation prototypes.

---

## Future Work

Planned improvements include:

* CAN Bus communication.
* Data logging on SD card.
* Battery backup support.
* Web dashboard integration.
* Multi-tank monitoring.
* Remote firmware updates (OTA).

---

## Author

**Alejandro Monroy Avelino**

M.Sc. Student in Advanced Computing and Electronics

Autonomous University of the State of Hidalgo (UAEH)

---

## License

This project is released under the MIT License.
