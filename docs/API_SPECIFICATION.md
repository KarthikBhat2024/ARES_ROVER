# ARES — REST API & Teleoperation Protocol Specification

## 1. System Overview

ARES utilizes a distributed dual-microcontroller architecture:
1. **ESP32 Rover Host (`192.168.4.1`):** Acts as the primary Wi-Fi SoftAP host, serves the unified Web Dashboard, handles 6-wheel differential motor kinematics with hardware failsafe watchdog, and proxies arm teleoperation commands to the ESP8266.
2. **ESP8266 Robotic Arm Controller (`192.168.4.20`):** Operates as a Wi-Fi station on the rover's local subnet, controls the PCA9685 16-channel 12-bit PWM generator, and executes non-blocking trajectory interpolation with conservative angle clamping.

All responses include CORS headers (`Access-Control-Allow-Origin: *`) and return JSON payloads.

---

## 2. ESP32 Rover Endpoints (`http://192.168.4.1`)

### 2.1 Web Dashboard Root
- **Route:** `GET /`
- **Description:** Returns the standalone mission control HTML/CSS/JS dashboard stored in ESP32 PROGMEM.
- **Captive Portal Handlers:** `/generate_204`, `/hotspot-detect.html`, `/ncsi.txt` automatically redirect to `/`.

---

### 2.2 Rover Telemetry & System Status
- **Route:** `GET /api/status`
- **Description:** Returns vehicle motion state, drive throttle, failsafe watchdog status, power telemetry, and arm gateway health.
- **Response Example (200 OK):**
```json
{
  "status": "ok",
  "device": "ARES_ROVER_ESP32",
  "state": "IDLE",
  "speed": 160,
  "watchdog": true,
  "uptime": 142,
  "batt": 11.8,
  "batt_sim": true,
  "light": false,
  "beacon": false,
  "arm_online": true,
  "arm_last_seen_ms": 120
}
```

---

### 2.3 Rover Drive Command
- **Route:** `POST /api/drive` or `GET /api/drive?cmd=<action>&spd=<duty>`
- **Query / Form Parameters:**
  - `cmd` (string, required): `w` (forward), `s` (reverse), `a` (spin left), `d` (spin right), `q` (pivot left), `e` (pivot right), `b` (brake), `x` (stop), `t` (self-test).
  - `spd` (integer, optional): PWM duty cycle (90 to 255).
- **Watchdog Behavior:** Every valid motion command resets the 1.5-second safety timer. If commands cease, motors halt automatically.
- **Response Example (200 OK):**
```json
{
  "status": "ok",
  "state": "FORWARD",
  "speed": 180
}
```

---

### 2.4 Rover Stop Command
- **Route:** `POST /api/stop` or `GET /api/stop`
- **Description:** Immediately de-energizes all 6 DC drive motors.
- **Response Example (200 OK):**
```json
{
  "status": "ok",
  "state": "STOPPED",
  "speed": 160
}
```

---

### 2.5 Master Emergency Stop (ALL-STOP)
- **Route:** `POST /api/emergency_stop` or `GET /api/emergency_stop`
- **Description:** Simultaneously halts rover motors AND forwards an urgent freeze command to the ESP8266 arm controller.
- **Response Example (200 OK):**
```json
{
  "status": "ok",
  "action": "EMERGENCY_STOP",
  "rover_state": "STOPPED",
  "arm_stop_sent": true,
  "arm_connected": true
}
```

---

### 2.6 Speed Throttle Update
- **Route:** `POST /api/speed?val=<duty>` or `GET /api/speed?val=<duty>`
- **Query Parameters:** `val` (integer between 90 and 255).
- **Response Example (200 OK):**
```json
{
  "status": "ok",
  "speed": 180
}
```

---

### 2.7 Auxiliary Subsystems
- **Route:** `POST /api/aux?item=<light|beacon>&state=<0|1>`
- **Response Example (200 OK):**
```json
{
  "status": "ok"
}
```

---

## 3. Robotic Arm Gateway Endpoints (Proxied via ESP32)

The browser sends arm teleoperation requests directly to the ESP32 host, which validates and forwards them to the ESP8266 at `http://192.168.4.20/api/` with a 450 ms timeout.

### 3.1 Arm Telemetry & Status
- **Route:** `GET /api/arm/status`
- **Description:** Returns commanded joint angles, active limits, motion state, and holding torque status.
- **Response Example (200 OK):**
```json
{
  "status": "ok",
  "device": "ARES_ARM_ESP8266",
  "uptime": 128,
  "moving": false,
  "frozen": false,
  "pwm_enabled": true,
  "speed_dps": 45.0,
  "rssi": -52,
  "feedback_type": "commanded_open_loop",
  "servos": [
    { "id": 1, "name": "Base Turret", "model": "MG996R", "angle": 90.0, "target": 90.0, "min": 10, "max": 170, "home": 90 },
    { "id": 2, "name": "Shoulder Pitch", "model": "MG996R", "angle": 90.0, "target": 90.0, "min": 20, "max": 160, "home": 90 },
    { "id": 3, "name": "Elbow Pitch", "model": "MG996R", "angle": 90.0, "target": 90.0, "min": 20, "max": 160, "home": 90 },
    { "id": 4, "name": "Wrist Pitch", "model": "SG90", "angle": 90.0, "target": 90.0, "min": 15, "max": 165, "home": 90 },
    { "id": 5, "name": "Wrist Roll", "model": "SG90", "angle": 90.0, "target": 90.0, "min": 10, "max": 170, "home": 90 },
    { "id": 6, "name": "Gripper Jaw", "model": "SG90", "angle": 90.0, "target": 90.0, "min": 40, "max": 135, "home": 90 }
  ]
}
```

---

### 3.2 Single Servo Angle Command
- **Route:** `POST /api/arm/servo?id=<1..6>&angle=<deg>&spd=<dps>`
- **Parameters:**
  - `id` (integer, 1 to 6): Joint index.
  - `angle` (float, degrees): Target angle (automatically clamped to servo min/max).
  - `spd` (float, optional): Movement rate in degrees per second (default 45°/s).
- **Response Example (200 OK):**
```json
{
  "status": "ok",
  "servo": 1,
  "target": 95.0
}
```

---

### 3.3 Batch Joint Angles (Pose Preset)
- **Route:** `POST /api/arm/servos?s1=<deg>&s2=<deg>&s3=<deg>&s4=<deg>&s5=<deg>&s6=<deg>&spd=<dps>`
- **Description:** Simultaneously updates target angles for all 6 joints for smooth synchronized trajectory interpolation.
- **Response Example (200 OK):**
```json
{
  "status": "ok",
  "message": "Batch sent to arm"
}
```

---

### 3.4 Arm Home Command
- **Route:** `POST /api/arm/home`
- **Description:** Interpolates all 6 joints smoothly to their configured safe home angles.
- **Response Example (200 OK):**
```json
{
  "status": "ok",
  "message": "Arm home command dispatched"
}
```

---

### 3.5 Arm Freeze / Stop Command
- **Route:** `POST /api/arm/stop`
- **Description:** Halts trajectory interpolation immediately. Joints maintain holding torque at their current positions so elevated payloads do not collapse under gravity.
- **Response Example (200 OK):**
```json
{
  "status": "ok",
  "message": "Arm stop command dispatched"
}
```

---

### 3.6 Runtime Limit Calibration
- **Route:** `POST /api/arm/limits?id=<1..6>&min=<deg>&max=<deg>`
- **Description:** Updates the software safety limits on the ESP8266 controller to prevent mechanical binding.
- **Response Example (200 OK):**
```json
{
  "status": "ok",
  "message": "Arm limits updated"
}
```
