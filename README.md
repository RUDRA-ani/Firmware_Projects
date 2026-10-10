# ESP32 Dual-Zone Temperature Monitoring & Timer System

An embedded firmware project built using **ESP32, C, and ESP-IDF** to monitor two temperature sensors, manage independent countdown timers, and provide audible and visual alerts through a keypad-controlled interface.

The project focuses on modular firmware design, peripheral interfacing, sensor data processing, and embedded system debugging. The hardware is prototyped and simulated using Wokwi.

## Features

- **Dual-channel temperature monitoring** using NTC thermistors and the ESP32 ADC.
- **Temperature conversion** from raw ADC readings to degrees Celsius using the thermistor Beta equation.
- **Keypad-based control** using a 4×4 matrix keypad.
- **Independent countdown timers** for two zones.
- **Audible and visual alerts** using a buzzer and LED when a timer expires.
- **Sensor-reading validation** to identify invalid ADC readings and implausible temperature values.
- **Modular firmware architecture** using separate ESP-IDF components for temperature sensing, keypad input, timer management, and alarm control.

## Hardware Components

| Component | Purpose |
|---|---|
| ESP32 DevKit | Main microcontroller |
| 2 × NTC thermistor modules | Temperature sensing |
| 4×4 matrix keypad | User input |
| Buzzer | Audible alert |
| LED | Visual alert |
| Wokwi simulator | Circuit prototyping and firmware testing |

## Pin Configuration

| Peripheral | ESP32 GPIO | Function |
|---|---:|---|
| Burner 1 temperature sensor | GPIO 34 | ADC input |
| Burner 2 temperature sensor | GPIO 35 | ADC input |
| Keypad row 1 | GPIO 23 | Digital I/O |
| Keypad row 2 | GPIO 22 | Digital I/O |
| Keypad row 3 | GPIO 21 | Digital I/O |
| Keypad row 4 | GPIO 19 | Digital I/O |
| Keypad column 1 | GPIO 18 | Digital I/O |
| Keypad column 2 | GPIO 5 | Digital I/O |
| Keypad column 3 | GPIO 17 | Digital I/O |
| Keypad column 4 | GPIO 16 | Digital I/O |
| Buzzer and LED | GPIO 27 | Alert output |

The temperature inputs use the ESP32's ADC1 channels. GPIO34 and GPIO35 are input-only pins, making them suitable for analog sensor readings.

## How It Works

1. The ESP32 initializes the ADC, keypad, timer logic, and alarm output.
2. The firmware reads both NTC thermistors and converts their ADC values into temperature estimates.
3. The user selects a zone using the keypad and enters a timer duration.
4. The selected countdown timer starts while the firmware continues handling temperature readings and user input.
5. When a countdown expires, the alarm output activates the buzzer and LED.
6. The user can acknowledge the alarm or cancel a timer through the keypad.
7. Invalid or implausible sensor readings are flagged by the firmware.

## Firmware Architecture

The application is organized into independent ESP-IDF components to improve code readability, maintainability, and separation of responsibilities.

```text
project-name/
├── CMakeLists.txt
├── sdkconfig
├── diagram.json
├── wokwi.toml
├── main/
│   ├── CMakeLists.txt
│   └── hello_world_main.c
└── components/
    ├── temperature/
    │   ├── CMakeLists.txt
    │   ├── temperature.c
    │   └── temperature.h
    ├── keypad/
    │   ├── CMakeLists.txt
    │   ├── keypad.c
    │   └── keypad.h
    ├── burner_timer/
    │   ├── CMakeLists.txt
    │   ├── burner_timer.c
    │   └── burner_timer.h
    └── alarm/
        ├── CMakeLists.txt
        ├── alarm.c
        └── alarm.h
```

### Component Responsibilities

- **Temperature:** ADC initialization, thermistor conversion, and sensor-reading validation.
- **Keypad:** Matrix scanning, key detection, and debouncing.
- **Burner Timer:** Independent timer state and remaining-time management.
- **Alarm:** GPIO control for audible and visual alerts.
- **Main Application:** Initialization and coordination of the application components.

## Temperature Calculation

The firmware estimates the NTC thermistor resistance from the ADC reading and uses the Beta equation to calculate temperature.

For an NTC thermistor:


The calculated temperature is converted to degrees Celsius for reporting.

## Development Environment

- **Microcontroller:** ESP32
- **Framework:** ESP-IDF
- **Programming language:** C
- **Build system:** CMake and Ninja
- **Simulation:** Wokwi
- **Hardware design:** KiCad (planned / in progress)

## Getting Started

### Prerequisites

- ESP-IDF installed and configured.
- ESP32 target support.
- Wokwi extension or compatible simulation setup, if using VS Code.
- Git.
 

Configure the ESP32 target and build the firmware:

```bash
idf.py set-target esp32
idf.py build
```

To flash the firmware to a connected ESP32:

```bash
idf.py -p PORT flash monitor
```

Replace `PORT` with the serial port for your device. If you are using Wokwi, follow the simulation setup in `wokwi.toml` and the Wokwi project configuration.

## Testing

The project is developed and tested through simulation and iterative firmware debugging.

Suggested validation cases:

- Verify temperature readings from both sensor channels.
- Check that each countdown timer operates independently.
- Confirm that keypad input starts, cancels, and acknowledges the appropriate operations.
- Confirm buzzer and LED behavior when a timer expires.
- Test invalid sensor readings and confirm they are reported.
- Verify startup and peripheral initialization behavior.

Document the test environment and observed results as you complete each case.

## Future Improvements

- Design a custom PCB using KiCad.
- Improve sensor diagnostics and fault handling.
- Add unit tests for temperature conversion and timer state logic.
- Add structured logging and more comprehensive test cases.
- Evaluate low-power operation and alternative microcontrollers.
- Explore optional wireless temperature logging and monitoring.

## Safety Disclaimer

This project is an educational temperature-monitoring and timer-alarm prototype. It provides audible and visual alerts but does **not** automatically shut off a stove or provide certified fire-prevention protection. It must not be relied upon as a safety-rated appliance controller.

 

 