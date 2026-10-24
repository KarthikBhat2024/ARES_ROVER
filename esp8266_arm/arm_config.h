#pragma once
#include <Arduino.h>

// ==============================================================================
//  ARES — Autonomous Response & Exploration System
//  Upper Robotic Arm Subsystem — ESP8266 Firmware Configuration
// ==============================================================================

// ------------------------------------------------------------------------------
// 1. WI-FI STATION CONFIGURATION (Connects to ESP32 Rover Access Point)
// ------------------------------------------------------------------------------
#define ARM_WIFI_SSID           "ARES_ROVER_WIFI"
#define ARM_WIFI_PASSWORD       "ares123456"

// Static IP Configuration (Guarantees zero-lookup latency from ESP32 gateway)
#define ARM_USE_STATIC_IP       true
#define ARM_STATIC_IP           192, 168, 4, 20
#define ARM_GATEWAY_IP          192, 168, 4, 1
#define ARM_SUBNET_MASK         255, 255, 255, 0
#define ARM_DNS_IP              192, 168, 4, 1

// Web Server Port
#define ARM_HTTP_PORT           80

// ------------------------------------------------------------------------------
// 2. I2C & PCA9685 HARDWARE PINOUT (NodeMCU ESP8266)
// ------------------------------------------------------------------------------
// Default NodeMCU I2C Pins:
// D2 = GPIO 4 (SDA)
// D1 = GPIO 5 (SCL)
#define PIN_I2C_SDA             4   // NodeMCU D2
#define PIN_I2C_SCL             5   // NodeMCU D1
#define PCA9685_I2C_ADDRESS     0x40
#define PCA9685_PWM_FREQ_HZ     50  // 50 Hz PWM frequency for standard analog/digital servos

// Optional Hardware Output Enable (OE) Pin (Active LOW)
// If connected to NodeMCU D5 (GPIO 14), set to 14. If hardwired to GND, set to -1.
#define PIN_PCA9685_OE          -1

// ------------------------------------------------------------------------------
// 3. SERVO SPECIFICATIONS & CALIBRATED PULSE WIDTHS
// ------------------------------------------------------------------------------
// Standard 50Hz PWM cycle = 20,000 microseconds (20ms)
// PCA9685 has 12-bit resolution: 4096 ticks per 20ms cycle (~4.88 microseconds per tick).
// MG996R & SG90 typical pulse range: 500us to 2500us (corresponding to ~102 to ~512 ticks).
#define SERVO_PULSE_MIN_US      544   // ~111 ticks (0 degrees nominal)
#define SERVO_PULSE_MAX_US      2400  // ~491 ticks (180 degrees nominal)

// ------------------------------------------------------------------------------
// 4. SERVO CHANNEL MAPPING & CONSERVATIVE ANGLE LIMITS
// ------------------------------------------------------------------------------
// CAUTION: Do NOT exceed mechanical limits of the physical linkages!
// Start with conservative limits and adjust after testing without load.
struct ServoChannelConfig {
    uint8_t channel;        // PCA9685 channel (0 to 15)
    const char* name;       // Joint identifier
    const char* model;      // Motor model (MG996R or SG90)
    int16_t minAngle;       // Safe minimum limit (degrees)
    int16_t maxAngle;       // Safe maximum limit (degrees)
    int16_t homeAngle;      // Default safe startup / home angle (degrees)
    bool inverted;          // Invert software direction if linkage is reversed
};

// 6-DOF Robotic Arm Channel Setup
#define TOTAL_ARM_SERVOS 6

const ServoChannelConfig DEFAULT_SERVO_CONFIGS[TOTAL_ARM_SERVOS] = {
    // Ch 0: Servo 1 - Base Turret Rotation (MG996R, high torque)
    { 0, "Base Turret",    "MG996R", 10,  170,  90, false },

    // Ch 1: Servo 2 - Shoulder Joint Pitch (MG996R, high torque)
    { 1, "Shoulder Pitch", "MG996R", 20,  160,  90, false },

    // Ch 2: Servo 3 - Elbow Joint Pitch (MG996R, high torque)
    { 2, "Elbow Pitch",    "MG996R", 20,  160,  90, false },

    // Ch 3: Servo 4 - Wrist Pitch (SG90, precision micro servo)
    { 3, "Wrist Pitch",    "SG90",   15,  165,  90, false },

    // Ch 4: Servo 5 - Wrist Roll / Rotation (SG90, precision micro servo)
    { 4, "Wrist Roll",     "SG90",   10,  170,  90, false },

    // Ch 5: Servo 6 - End Effector Gripper (SG90, gripper claw)
    // 40° = closed grip, 130° = fully opened jaw (conservative starting limits)
    { 5, "Gripper Jaw",    "SG90",   40,  135,  90, false }
};

// ------------------------------------------------------------------------------
// 5. MOTION INTERPOLATION & SMOOTH SPEED DYNAMICS
// ------------------------------------------------------------------------------
// Prevents abrupt high-current surges and violent jerky arm movement.
#define MOTION_UPDATE_INTERVAL_MS   20    // Update servo position every 20ms
#define DEFAULT_SPEED_DPS           45    // Degrees per second (smooth, controlled motion)
#define SPEED_SLOW_DPS              25    // Slow precision placement
#define SPEED_FAST_DPS              90    // Fast slew rate

// ------------------------------------------------------------------------------
// 6. SAFETY & SERIAL PARAMETERS
// ------------------------------------------------------------------------------
#define ARM_SERIAL_BAUD_RATE        115200
#define AUTO_MOVE_ON_BOOT           false // SAFETY: NEVER move servos on boot until commanded!
