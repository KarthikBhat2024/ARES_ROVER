# ARES — Autonomous Response & Exploration System
## Smart 6-Wheel Rescue Rover & 6-DOF Robotic Arm

ARES is a complete robotics teleoperation platform featuring an **ESP32 microcontroller** for lower 6-wheel differential skid-steer locomotion and an **ESP8266 NodeMCU microcontroller** controlling an articulated 6-DOF robotic arm via a **PCA9685 16-channel 12-bit PWM generator**.

---

## Workspace Folder Structure

```
ARES RESCUE ROVER/
├── esp32_rover/                  # Complete ESP32 Lower Rover Firmware
│   ├── ares_rover.ino            # Main ESP32 sketch & serial diagnostic CLI
│   ├── config.h                  # Motor pin assignments (13,14,25,26,27,33,16,17,32,18,19,23,21,22,4,5,15,2)
│   ├── motor_controller.h        # 6-motor differential drive class declarations
│   ├── motor_controller.cpp      # LEDC PWM engine, skid-steer kinematics & safety watchdog
│   ├── arm_gateway.h             # Inter-controller HTTP bridge declarations
│   ├── arm_gateway.cpp           # Fast non-blocking HTTP proxy to ESP8266 arm
│   ├── web_server_manager.h      # REST teleoperation server declarations
│   ├── web_server_manager.cpp    # Full REST API, DNS Captive Portal & mDNS (ares.local)
│   ├── dashboard_html.h          # Cybernetic mission control dashboard in PROGMEM
│   └── index.html                # Source dashboard HTML/CSS/JS
│
├── esp8266_arm/                  # Complete ESP8266 Upper Robotic Arm Firmware
│   ├── ares_arm.ino              # Main ESP8266 sketch & serial diagnostic CLI
│   ├── arm_config.h              # Servo angle limits, static IP (192.168.4.20) & I2C pins
│   ├── pca9685_arm_controller.h  # 6-DOF trajectory interpolator class declarations
│   ├── pca9685_arm_controller.cpp# Direct I2C driver for PCA9685, 50 Hz PWM & angle clamping
│   ├── arm_web_server.h          # Standalone REST API server declarations
│   └── arm_web_server.cpp        # REST endpoints (/api/status, /api/servo, /api/home, /api/stop)
│
├── web_dashboard/                # Standalone Web Dashboard Assets
│   └── index.html                # High-tech Cybernetic Command Center Dashboard
│
└── docs/                         # Technical Documentation & Wiring
    ├── API_SPECIFICATION.md      # Complete REST API reference for both microcontrollers
    ├── SYSTEM_INTEGRATION_GUIDE.md # Wiring schematics, upload guide & 10-stage test checklist
    └── WIRING_AND_HARDWARE_GUIDE.md# Rover electrical analysis & strapping pin guidelines
```

---

## Quickstart Guide

### 1. Wi-Fi Connection
1. Power on the Rover and Robotic Arm.
2. Connect your laptop or mobile phone to the rover Wi-Fi network:
   - **SSID:** `ARES_ROVER_WIFI`
   - **Password:** `ares123456`
3. Open any modern browser (Chrome, Safari, Edge, Firefox) and navigate to:
   - **`http://192.168.4.1`** (or **`http://ares.local`**)

### 2. Dashboard Capabilities
- **Dual Connection Capsules:** Real-time link indicators for ESP32 Rover and ESP8266 Arm separately.
- **Rover Teleoperation:** Hold-to-move and Click-to-toggle modes, 6-wheel live chassis visualizer, and keyboard driving (`W`, `A`, `S`, `D`, `Q`, `E`, `Space`).
- **6-DOF Robotic Arm:** Dedicated sliders and ±1°/±5° nudge buttons for all 6 servos with configurable safe angle limits, master Home, and holding-torque Freeze.
- **Pose Presets:** Editable poses saved in browser storage (Home, Reach Forward, Raise Overhead, Ground Scan, Grab, Release).
- **Master Emergency Stop:** Giant red E-STOP button halting both rover motors and arm servos simultaneously.

For complete wiring diagrams and safe staged testing instructions, consult [`docs/SYSTEM_INTEGRATION_GUIDE.md`](docs/SYSTEM_INTEGRATION_GUIDE.md).
