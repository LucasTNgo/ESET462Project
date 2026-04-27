# Embedded Environmental Control System (RP2350 / Raspberry Pi Pico 2 W)

An embedded C++ control system for regulating temperature and humidity inside a sealed enclosure using a Raspberry Pi Pico 2 W. The system implements closed-loop PI control with real-time scheduling via FreeRTOS and includes characterization tools for system identification.

---

## Overview

This project implements a dual-loop environmental control system:

- **Temperature regulation** via a resistive heater
- **Humidity regulation** via a PWM-controlled fan

The system is designed around real physical constraints (thermal time constants, relay switching limits, sensor dynamics) and structured as a layered embedded architecture.

---

## Key Features

- Dual **PI control loops** (temperature + humidity)
- **Tustin (bilinear) discretization** for digital control fidelity
- Real-time scheduling using **FreeRTOS (single-core RP2350)**
- Hardware abstraction layer for all peripherals
- Sensor fusion using internal/external humidity conversion
- Characterization tools for system identification (heater + fan)
- Debug output via USB serial (CSV + verbose modes)

---

## System Architecture

The codebase is organized into four layers:
- Hardware Layer 
- Application Layer 
- RTOS Layer
- Debug Layer


This separation enforces modularity between physical I/O, control logic, scheduling, and observability.

---

## Hardware Layer

**Files:** `Hardware.cpp / Hardware.hpp`

This layer abstracts all physical peripherals:

### Fan Control
- PWM-driven at **15 kHz** (PC fan standard)
- Duty cycle range: `[0.0, 1.0]`
- Enforced minimum effective duty (~30%) due to non-linear low-speed behavior

### Heater Control
- Solid-state **zero-cross AC relay**
- Switching constraint: ≤ 120 Hz (60 Hz mains)
- Operational PWM frequency: **0.6 Hz**
- Implemented via **timer interrupt** (RP2350 hardware PWM insufficient at this scale)

### Sensors
- Dual **SHT40 I2C temperature/humidity sensors**
  - Internal enclosure measurement
  - External ambient measurement
- External humidity is converted to **equivalent internal-relative humidity**, enabling meaningful control of humidity differential rather than absolute external conditions

---

## Application Layer (Control System)

**Files:** `Application.cpp / Application.hpp`

Implements the closed-loop control system.

### Control Strategy
Two discrete-time **PI controllers**:

- Implemented using **Tustin (bilinear) transform**
- Maintains frequency-domain equivalence with continuous-time design
- Executed at fixed sampling rate

### Temperature Control (Heater)
- Regulates internal temperature to setpoint
- Includes **gain scheduling**
  - Separate tuned gains depending on fan state
  - Accounts for altered thermal dynamics when airflow is active

### Humidity Control (Fan)
- Controls **humidity differential** (internal vs external equivalent)
- Controller output is inverted:
  - Negative error → increased fan duty
  - Increased airflow reduces internal humidity toward ambient equilibrium

### Configuration
All tuning parameters are centralized in:
`ApplicationConfig.hpp`

Includes:
- PI gains
- Setpoints
- Sampling period
- Output clamps

---

## RTOS Layer

**Files:** `RTOSTasks.cpp / RTOSTasks.hpp`

Runs on **FreeRTOS (single core RP2350)**.

### Task Structure

| Task | Priority | Frequency | Function |
|------|----------|----------|----------|
| Sensor Task | High | 10 Hz | Reads SHT40 sensors |
| Control Task | Medium | 10 Hz | Executes PI controllers |
| Actuator Task | Medium | ~0.6 Hz | Applies fan/heater outputs |
| Debug Task | Low | Variable | Serial logging |

### Communication Model

- Uses **single-slot FreeRTOS queues**
- Employs:
  - `xQueueOverwrite()` → latest-value semantics
  - `xQueuePeek()` → non-blocking reads

This ensures:
- No queue buildup
- Always using freshest sensor/control data
- Deterministic behavior under load

---

## Debug & Characterization Tools

**Files:** `UserDebug.cpp / UserDebug.hpp`

### Debug Output
- USB serial interface
- Output modes:
  - CSV logging (for plotting / analysis)
  - Verbose human-readable output

### System Identification Tools

Two standalone CMake targets are included:

#### Heater Characterization
- Applies fixed duty cycle
- Logs temperature response over time
- Used to estimate thermal transfer function

#### Fan Characterization
- Measures fan RPM vs PWM duty cycle
- Planned extension:
  - Measure airflow impact on thermal + humidity dynamics

---

## Build System

- **Language:** Embedded C++
- **Target:** Raspberry Pi Pico 2 W (RP2350)
- **Toolchain:** Raspberry Pi Pico SDK + GCC
- **Build System:** CMake

---

## Design Highlights

- Real-time embedded architecture with strict timing constraints
- Practical handling of non-ideal actuator behavior (fan dead zones, relay limits)
- Control system designed around measurable physical dynamics
- Clean separation of hardware, control logic, and scheduling
- Built-in system identification for model-based tuning

---

## Future Improvements

- Full system identification (fan + thermal coupling model)
- State-space controller replacement for PI loops
- Adaptive gain scheduling based on operating region
- On-device logging to external storage (SD / flash logging buffer)

---