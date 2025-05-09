# ESP32 POV Fan Display with Blynk Integration

This project creates a **Persistence of Vision (POV)** display using an **ESP32** and a **NeoPixel LED strip**, mounted on a rotating system. It dynamically displays **time**, **weather**, or **custom text** based on user control via the **Blynk IoT app**.

---

## Features

- Real-time **clock display**
- **Weather info** (text + temperature)
- Custom **user-defined text**
- **Blynk control**: brightness, color, mode toggle
- Fully **non-blocking display loop**
- **Offline mode fallback** for time
- Compact **5x7 bitmap font** for uppercase letters and digits
- Optional **text centering**

---

## Hardware Components

| Component              | Purpose                         |
|------------------------|---------------------------------|
| ESP32 Dev Board        | Main controller and WiFi module |
| Hall effect sensor     | Detects rotation zero-point     |
| NeoPixel LED strip     | POV visual display              |
| 5V power supply        | Powers the ESP32 + LEDs         |
| Fan motor (DC or BLDC) | Drives rotation for POV         |

---

## Pin Configuration

| ESP32 Pin | Function         |
|-----------|------------------|
| GPIO 13   | Data line to NeoPixel LED strip |
| GPIO 14   | Hall effect sensor input        |

---

## Blynk Integration

| Blynk Virtual Pin | Function                  |
|-------------------|---------------------------|
| `V1`              | Brightness control (0–255)|
| `V2`              | Color picker (RGB)        |
| `V3`              | Display mode selector     |
| `V4`              | Clock string display      |
| `V5`              | Weather description input |
| `V6`              | Temperature input         |
| `V7`              | Custom text input         |

Make sure to create a template on [Blynk Console](https://blynk.cloud/dashboard) using your own **template ID**, **device name**, and **auth token**.

---

## How It Works

### 1. **Hall Effect Sensor Interrupt**
- Detects a magnet to mark the **start of a rotation**.
- Triggers an interrupt that calculates **rotation duration** (`rotationTime`) and resets the column display counter.

### 2. **Non-blocking LED Updates**
- The LED column updates based on time spacing using `micros()`, not `delay()`.
- `TOTAL_COLUMNS` evenly divides the full 360° rotation into discrete frame slices.
- `columnIndex` is incremented after each interval, and one vertical LED column is displayed.

### 3. **Font and Text Rendering**
- A fixed **5x7 bitmap font** is used for rendering characters.
- Each character is 5 columns wide, with 1 column of spacing.
- For each columnIndex, only the corresponding bit column of the text is rendered.

### 4. **Display Modes**
- Controlled via `mode` variable (`V3` in Blynk):
  - `0` = Clock mode (auto-updated from Blynk RTC)
  - `1` = Weather mode (concatenates weatherDesc and temperature)
  - `2` = Custom Text mode (fully user-defined string)

### 5. **Brightness and Color Control**
- Brightness: scaled via `strip.setBrightness()` (0–255)
- Color: Set using Blynk’s RGB Color Picker widget

---

## 🛠 Functions Overview

| Function                  | Purpose |
|---------------------------|---------|
| `setup()`                 | Initializes hardware, WiFi, Blynk, interrupts |
| `loop()`                  | Handles display timing, rotation logic, and text rendering |
| `hallSensorTrigger()`     | Interrupt handler to calculate rotation speed |
| `displayColumn(text, i)`  | Displays a single column of the current message on the strip |
| `updateTime()`            | Updates time from Blynk RTC if connected |
| `BLYNK_WRITE(...)`        | Handles updates from Blynk virtual pins |

---

## Notes

- The system requires accurate and **consistent rotation** for legible output.
- Font supports only uppercase letters and digits (no punctuation or lowercase).
- The **text centering feature** is implemented but commented out by default.
- For best results, ensure the Hall sensor is aligned **below the motor** (270°), such that the display renders at the top visually.

---

## Future Improvements

- Add support for more characters (punctuation, lowercase).
- Add animation/effects between mode switches.
- Add Bluetooth fallback for offline control.

---

## Maintainers

Group 3 - CpE 3202 (2024-2025)
ESP32 POV Display Final Project

---

