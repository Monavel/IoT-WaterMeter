# Water Level and Water Consumption Monitor

**Firmware Version:** 0.6.9

An ESP32-based IoT system designed for real-time monitoring of water tank level and water consumption. The device uses differential pressure sensors to estimate water level and a flow sensor to measure water usage. Data can be monitored locally through an LCD display or remotely through the Blynk platform.

## Features

* Real-time water level measurement using differential pressure sensors.
* Water consumption monitoring using a pulse-based flow sensor.
* Local visualization through a 20x4 I2C LCD.
* Remote monitoring using Blynk IoT.
* Automatic daily reset of consumption statistics.
* Offline operation when internet connectivity is unavailable.
* Dual-core processing using the ESP32 FreeRTOS architecture.
* Non-volatile memory storage for calibration offsets.
* Remote configuration of system parameters through the Blynk application.

---

## Hardware

### Microcontroller

* ESP32

### Sensors

* Two analog pressure sensors:

  * Air reference pressure sensor
  * Water pressure sensor
* Hall-effect flow sensor

### User Interface

* 20x4 LCD display (I2C)
* Zero-level calibration push button
* Display mode selection switch

### Connectivity

* Wi-Fi
* Blynk IoT Platform

---

## Pin Assignment

| Function              | GPIO   |
| --------------------- | ------ |
| Zero Level Button     | GPIO25 |
| Display Mode Switch   | GPIO26 |
| Flow Sensor           | GPIO27 |
| Reset Input           | GPIO33 |
| Air Pressure Sensor   | GPIO34 |
| Water Pressure Sensor | GPIO35 |

---

## System Operation

### Water Level Measurement

The system continuously samples both pressure sensors and calculates the water column height using a differential measurement:

Level = k × (Vwater − Vair − Offset) / Vs

where:

* Vwater = water pressure sensor voltage
* Vair = atmospheric pressure sensor voltage
* Offset = stored zero reference
* Vs = sensor supply voltage
* k = calibration constant

The resulting level is displayed in meters and converted into a percentage of total tank capacity.

### Water Consumption Measurement

A pulse-based flow sensor is used to determine:

* Instantaneous flow rate (L/min)
* Total accumulated consumption (L)

Flow calculations are updated every second.

### Zero-Level Calibration

Pressing the calibration button stores the current differential pressure value as a reference offset. The offset is saved in the ESP32 non-volatile memory and remains available after power cycles.

### Daily Consumption Reset

Using the Blynk RTC service, the system automatically resets daily water consumption counters at midnight.

---

## Blynk Virtual Pins

| Virtual Pin | Function                         |
| ----------- | -------------------------------- |
| V0          | Water level (m)                  |
| V1          | Tank filling percentage (%)      |
| V3          | Tank height configuration (m)    |
| V4          | Sensor supply voltage (V)        |
| V5          | Instantaneous flow rate (L/min)  |
| V6          | Total water consumption (L)      |
| V8          | Flow sensor calibration constant |
| V9          | Previous day consumption         |
| V11         | Air pressure sensor voltage      |
| V12         | Water pressure sensor voltage    |

---

## Software Architecture

The firmware uses both ESP32 cores:

### Core 0

Handles:

* Blynk communication
* Wi-Fi management
* Timer execution
* Cloud synchronization

### Core 1

Handles:

* Sensor acquisition
* Signal processing
* Flow calculations
* LCD updates

This architecture allows measurement tasks to continue operating even when network communication is interrupted.

---

## Main Libraries

* BlynkEdgent
* LiquidCrystal_I2C
* TimeLib
* WidgetRTC
* Preferences

---

## Applications

* Residential water tank monitoring
* Water consumption tracking
* Smart home water management
* Remote water infrastructure monitoring
* Environmental instrumentation projects

---

## Author

Alejandro Monroy Avelino

M.Sc. Student in Advanced Computing and Electronics

Autonomous University of the State of Hidalgo (UAEH)
