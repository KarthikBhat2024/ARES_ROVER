# ARES — System Integration, Wiring & Calibration Guide

## 1. System Overview & Dual-Microcontroller Architecture

```
                       +---------------------------------------------+
                       |   ARES UNIFIED TELEOPERATION DASHBOARD     |
                       |       (Hosted in ESP32 Flash Memory)        |
                       +----------------------+----------------------+
                                              |
                   Web Browser (Phone / Laptop) over Wi-Fi AP
                                              v
                              +-------------------------------+
                              |    ESP32 ROVER CONTROLLER     |
                              |   SSID: ARES_ROVER_WIFI       |
                              |   IP:   192.168.4.1           |
                              +---------------+---------------+
                                              |
                     +------------------------+------------------------+
                     | Logic Signals (18 GPIOs)|                         | HTTP Gateway
                     v                         | (Subnet 192.168.4.x)   v
              +--------------+                 |              +-------------------+
              | 3x L298N     |                 |              | ESP8266 ARM MCU   |
              | Dual H-Bridge|                 |              | IP: 192.168.4.20  |
              +-------+------+                 |              +---------+---------+
                      |                        |                        | I2C (SDA=D2, SCL=D1)
                      v                        |                        v
               6x Geared DC Motors             |              +-------------------+
             (M1 - M6 Skid Steer)              |              | PCA9685 16-Ch PWM |
                                               |              +---------+---------+
                                               |                        |
                                               |             +----------+----------+
                                               |             | Ch 0,1,2: 3x MG996R |
                                               |             | Ch 3,4,5: 3x SG90   |
                                               |             +---------------------+
                                               |
                                     COMMON GROUND (GND)
```

---

## 2. Complete Wiring & Electrical Configuration

### 2.1 Lower Rover — ESP32 to 3x L298N Motor Drivers
> **IMPORTANT:** All 18 pin assignments match your existing working hardware configuration.

| Motor | Location | IN1 Pin | IN2 Pin | ENA/ENB (PWM) | Driver Board |
|---|---|---|---|---|---|
| **M1** | Front Left | GPIO 13 | GPIO 14 | GPIO 25 | Driver 1 (Front Axle) |
| **M2** | Front Right | GPIO 26 | GPIO 27 | GPIO 33 | Driver 1 (Front Axle) |
| **M3** | Middle Left | GPIO 16 | GPIO 17 | GPIO 32 | Driver 2 (Middle Axle) |
| **M4** | Middle Right | GPIO 18 | GPIO 19 | GPIO 23 | Driver 2 (Middle Axle) |
| **M5** | Rear Left | GPIO 21 | GPIO 22 | GPIO 4 | Driver 3 (Rear Axle) |
| **M6** | Rear Right | GPIO 5 | GPIO 15 | GPIO 2 | Driver 3 (Rear Axle) |

- **PWM Frequency:** 1,000 Hz (quiet, smooth torque).
- **Control Mode:** 3-Wire (Jumpers removed on ENA/ENB).
- **Safety Watchdog:** 1,500 ms automatic halt timeout.

---

### 2.2 Upper Robotic Arm — ESP8266 NodeMCU to PCA9685
| NodeMCU ESP8266 Pin | PCA9685 Pin | Description |
|---|---|---|
| **D2 (GPIO 4)** | **SDA** | I2C Data line |
| **D1 (GPIO 5)** | **SCL** | I2C Clock line |
| **3.3V** | **VCC** | Logic Power (powers PCA9685 logic core) |
| **GND** | **GND** | Logic Ground |
| *(Optional)* D5 | OE | Active-LOW Output Enable (tie to GND if unused) |

### 2.3 Servo Power Rails & 5V Buck Converter
> **CRITICAL ELECTRICAL RULE:** Servos must NEVER draw operating power from the ESP8266 3.3V or 5V regulator pin! High stall currents from MG996R servos (up to 2.5A each!) will instantly brown-out the microcontrollers or burn board traces.

1. **Servo Power (V+ Terminal on PCA9685):**
   - Connect the output `+5V` (or `+6V`) of your **dedicated 5V Buck Converter** to the heavy screw terminal marked `V+` on the PCA9685.
2. **Common Ground (MANDATORY):**
   - Connect the negative output (`GND`) of the 5V Buck Converter to the PCA9685 `GND` terminal AND to the ESP8266 `GND` pin AND to the ESP32 `GND` pin!
   - *Without a common ground, I2C logic levels float, causing jitter or communication lockups.*

### 2.4 Servo Channel Connections (PCA9685)
| PCA9685 Channel | Actuator Model | Arm Joint Role | Safe Limit Range | Default Home Angle |
|---|---|---|---|---|
| **Channel 0** | **MG996R** | Servo 1: Base Turret Rotation | 10° to 170° | 90° |
| **Channel 1** | **MG996R** | Servo 2: Shoulder Pitch | 20° to 160° | 90° |
| **Channel 2** | **MG996R** | Servo 3: Elbow Pitch | 20° to 160° | 90° |
| **Channel 3** | **SG90** | Servo 4: Wrist Pitch | 15° to 165° | 90° |
| **Channel 4** | **SG90** | Servo 5: Wrist Roll / Rotation | 10° to 170° | 90° |
| **Channel 5** | **SG90** | Servo 6: Gripper Jaw (Claw) | 40° (closed) to 135° (open) | 90° |

---

## 3. Library Dependencies & Setup

### 3.1 ESP32 Rover Firmware (`esp32_rover/`)
- **Required Arduino Core:** `esp32` by Espressif Systems (v2.0.x or v3.0.x).
- **Built-in Libraries Used:**
  - `WiFi.h` (Built into ESP32 core)
  - `WebServer.h` (Built into ESP32 core)
  - `DNSServer.h` (Built into ESP32 core)
  - `ESPmDNS.h` (Built into ESP32 core)
  - `HTTPClient.h` (Built into ESP32 core)
- **Zero External Library Downloads Required!** Compiles straight out of the box.

### 3.2 ESP8266 Arm Firmware (`esp8266_arm/`)
- **Required Arduino Core:** `esp8266` by ESP8266 Community (v3.0.x or v3.1.x).
- **Built-in Libraries Used:**
  - `ESP8266WiFi.h` (Built into ESP8266 core)
  - `ESP8266WebServer.h` (Built into ESP8266 core)
  - `Wire.h` (Built into ESP8266 core)
- **PCA9685 Driver:** We engineered a lightweight, direct hardware I2C driver inside `pca9685_arm_controller.cpp`. It communicates directly over standard `Wire.h` with the PCA9685 registers (frequency prescaling, 12-bit PWM generation, auto-increment), eliminating external library version conflicts.

---

## 4. How to Compile & Upload Each Microcontroller

### Step A: Upload ESP32 Rover Firmware
1. Open the Arduino IDE.
2. Go to **Tools > Board > esp32 > ESP32 Dev Module** (or your ESP32 model).
3. Connect your **ESP32** board via USB.
4. Select the matching COM port under **Tools > Port**.
5. Open `esp32_rover/ares_rover.ino`.
6. Click **Upload** (or `Ctrl + U`).
7. Open Serial Monitor at **115200 baud** to verify the startup banner and SoftAP status.

### Step B: Upload ESP8266 Arm Firmware
1. In the Arduino IDE, go to **Tools > Board > ESP8266 Boards > NodeMCU 1.0 (ESP-12E Module)**.
2. Set CPU Frequency to **80 MHz** (or **160 MHz**).
3. Connect your **ESP8266 NodeMCU** board via USB.
4. Select the matching COM port under **Tools > Port**.
5. Open `esp8266_arm/ares_arm.ino`.
6. Click **Upload** (or `Ctrl + U`).
7. Open Serial Monitor at **115200 baud** to verify PCA9685 hardware detection and Wi-Fi connection.

---

## 5. How Website Commands Reach Motors & Servos

```
[ Operator clicks / drags on Website ]
                 |
                 v
   HTTP GET/POST to ESP32 (192.168.4.1)
                 |
        +--------+--------+
        |                 |
  Rover Drive        Arm Command
  (/api/drive)     (/api/arm/servo)
        |                 |
        v                 v
 ESP32 LEDC PWM     HTTPClient Bridge
  -> 3x L298N       -> ESP8266 (192.168.4.20)
  -> 6 DC Motors          |
                          v
                    ESP8266 Interpolation Engine
                    -> I2C (50 Hz PWM)
                    -> PCA9685 Generator
                    -> 6 Servos (MG996R & SG90)
```

1. **Direct Rover Kinematics:** When you tap Forward or press `W`, the browser dispatches `/api/drive?cmd=w&spd=180` to the ESP32. The ESP32 LEDC PWM generator immediately commands the L298N motor drivers and refreshes the failsafe watchdog.
2. **Proxied Arm Teleoperation:** When you drag a servo slider or trigger a pose preset, the browser dispatches `/api/arm/servo?id=1&angle=95` to the ESP32. The ESP32's `ArmGateway` validates the payload and issues an HTTP POST to `http://192.168.4.20/api/servo`.
3. **Smooth Servo Trajectory:** The ESP8266 receives the target angle, clamps it to conservative mechanical limits, and smoothly interpolates the servo position at the configured rate (e.g., 45°/sec) so that the arm moves with aerospace precision rather than sudden mechanical shock.
4. **Master Emergency Stop:** When E-STOP is pressed, the ESP32 stops all DC drive motors instantly and issues an urgent freeze command to the arm controller, halting trajectory movement while keeping active holding torque so elevated payloads do not collapse.

---

## 6. Safe Staged Testing Checklist

Follow this strict 10-stage test order:

- [ ] **Stage 1: Build Verification**
  - Verify that `esp32_rover.ino` compiles without errors.
  - Verify that `ares_arm.ino` compiles without errors.
- [ ] **Stage 2: Bench Stand Suspension**
  - Place the rover chassis on a block or stand so all 6 wheels are elevated and spin freely in mid-air.
- [ ] **Stage 3: Upper Arm Unloaded Inspection**
  - Disconnect any physical payload or gripper weight. Verify linkages have free clearance.
- [ ] **Stage 4: Arm Power & PCA9685 Detection**
  - Power the ESP8266 and the 5V buck converter.
  - Open the ESP8266 Serial Monitor (115200 baud). Confirm: `[ARM HW] PCA9685 successfully configured at 50 Hz`.
- [ ] **Stage 5: Individual Servo Nudge Test**
  - Using the serial CLI or local web API, send small nudge angles (`s1 85`, `s1 95`, `s6 90`). Verify correct joint movement without binding.
- [ ] **Stage 6: ESP32 Access Point Connection**
  - Power the ESP32. On your phone/laptop, connect to Wi-Fi `ARES_ROVER_WIFI` (password: `ares123456`).
- [ ] **Stage 7: ESP8266 to ESP32 Wi-Fi Link**
  - Verify on ESP8266 serial monitor that it associates with `ARES_ROVER_WIFI` and assumes static IP `192.168.4.20`.
- [ ] **Stage 8: Dashboard Teleoperation & Rover Drive Verification**
  - Open `http://192.168.4.1` on your browser.
  - Test Hold-to-Move: Press & hold `FORWARD` -> wheels spin forward; release -> wheels stop.
  - Test Click-to-Toggle and speed slider adjustment.
- [ ] **Stage 9: Arm Teleoperation & Preset Verification**
  - Move each slider on the Robotic Arm panel. Verify commanded angle displays match slider positions.
  - Test Presets (`Reach Forward`, `Release Position`, `ARM HOME`).
- [ ] **Stage 10: Failsafe Watchdog & E-STOP Test**
  - Trigger `FORWARD` and abruptly disconnect browser Wi-Fi: verify all wheels stop within 1.5 seconds.
  - While driving or moving arm, press red **E-STOP**: verify vehicle halts instantly.

---

## 7. Hardware Limitations & Roadmap

1. **Commanded vs. Measured Angle:**
   - Standard MG996R and SG90 servos are **open-loop PWM actuators**. The angles shown on the dashboard are the *commanded setpoint angles*. Physical angle feedback requires adding magnetic encoders (e.g. AS5600 over I2C) or potentiometers to each joint.
2. **Current Telemetry & Battery Voltage:**
   - The battery reading `11.8 V (SIM)` is labeled as simulated until an external resistor voltage divider circuit is attached to an ESP32 ADC pin (e.g. GPIO 34 / 35).
3. **Emergency Power Cutoff:**
   - While software E-STOP halts motor PWM and freezes servo interpolation, a mechanical emergency-stop kill switch or relay on the main LiPo battery rail is always recommended for ultimate physical safety.
