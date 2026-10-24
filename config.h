#pragma once
#include <Arduino.h>

// ==============================================================================
//  ARES — Autonomous Response & Exploration System
//  Configuration File: Pin Assignments, Motor Calibration & Safety Parameters
// ==============================================================================

// ------------------------------------------------------------------------------
// 1. MOTOR CONTROL MODE SELECTION
// ------------------------------------------------------------------------------
// MODE_3_WIRE: Uses IN1, IN2, and ENA/ENB (PWM on Enable). 
//              Requires 18 GPIOs. ENA/ENB jumpers MUST be REMOVED from L298N!
// MODE_2_WIRE: Uses IN1 (PWM) and IN2 (PWM) with ENA/ENB jumpers KEPT ON (tied to 5V).
//              Requires only 12 GPIOs. Eliminates boot twitching on M6.
//
// Default is set to MODE_3_WIRE to match your proposed configuration.
#define CONTROL_MODE_3_WIRE   1
#define CONTROL_MODE_2_WIRE   2
#define MOTOR_CONTROL_MODE    CONTROL_MODE_3_WIRE

// ------------------------------------------------------------------------------
// 2. GPIO PIN ASSIGNMENTS (Proposed Configuration)
// ------------------------------------------------------------------------------
// Driver 1: Front Axle
#define PIN_M1_IN1      13    // Motor 1 (Front Left) Direction 1
#define PIN_M1_IN2      14    // Motor 1 (Front Left) Direction 2
#define PIN_M1_EN       25    // Motor 1 Enable / PWM

#define PIN_M2_IN1      26    // Motor 2 (Front Right) Direction 1
#define PIN_M2_IN2      27    // Motor 2 (Front Right) Direction 2
#define PIN_M2_EN       33    // Motor 2 Enable / PWM

// Driver 2: Middle Axle
#define PIN_M3_IN1      16    // Motor 3 (Middle Left) Direction 1
#define PIN_M3_IN2      17    // Motor 3 (Middle Left) Direction 2
#define PIN_M3_EN       32    // Motor 3 Enable / PWM

#define PIN_M4_IN1      18    // Motor 4 (Middle Right) Direction 1
#define PIN_M4_IN2      19    // Motor 4 (Middle Right) Direction 2
#define PIN_M4_EN       23    // Motor 4 Enable / PWM

// Driver 3: Rear Axle
#define PIN_M5_IN1      21    // Motor 5 (Rear Left) Direction 1
#define PIN_M5_IN2      22    // Motor 5 (Rear Left) Direction 2
#define PIN_M5_EN       4     // Motor 5 Enable / PWM

// WARNING ON M6 PINS: GPIO 2, 5, 15 are strapping pins on ESP32!
// See docs for alternative pin recommendation if boot anomalies occur.
#define PIN_M6_IN1      5     // Motor 6 (Rear Right) Direction 1 (Strapping pin)
#define PIN_M6_IN2      15    // Motor 6 (Rear Right) Direction 2 (Boot debug TX pin)
#define PIN_M6_EN       2     // Motor 6 Enable / PWM (Strapping pin & Onboard LED)

// ------------------------------------------------------------------------------
// 3. MOTOR DIRECTION INVERSION (Software Polarity Calibration)
// ------------------------------------------------------------------------------
// Change to 'true' if an individual motor spins backward when Rover moves forward.
// This eliminates the need to physically unscrew and swap wires!
#define INVERT_M1       false   // Front Left
#define INVERT_M2       false   // Front Right
#define INVERT_M3       false   // Middle Left
#define INVERT_M4       false   // Middle Right
#define INVERT_M5       false   // Rear Left
#define INVERT_M6       false   // Rear Right

// ------------------------------------------------------------------------------
// 4. PWM & SPEED CONFIGURATION
// ------------------------------------------------------------------------------
#define PWM_FREQ_HZ         1000  // 1 kHz PWM frequency (quiet and efficient for DC motors)
#define PWM_RESOLUTION_BITS 8     // 8-bit resolution (0 - 255)

// Speed presets (0 to 255)
#define SPEED_DEFAULT       160   // ~62% duty cycle (balanced cruise speed)
#define SPEED_MIN           90    // Minimum duty cycle to overcome DC motor static friction
#define SPEED_TEST_SAFE     110   // ~43% duty cycle for safe benchtop testing
#define SPEED_MAX           255   // 100% full power

// ------------------------------------------------------------------------------
// 5. SAFETY WATCHDOG & FAILSAFE TIMEOUT
// ------------------------------------------------------------------------------
// If no valid motion command is received within this window, all motors will
// automatically halt. Prevents runaway rovers during Wi-Fi or controller dropouts.
#define WATCHDOG_TIMEOUT_MS 1500  // 1.5 seconds timeout
#define SERIAL_BAUD_RATE    115200

// ------------------------------------------------------------------------------
// 6. WI-FI ACCESS POINT & WEB TELEOPERATION SETTINGS
// ------------------------------------------------------------------------------
// Standalone hotspot broadcast by the ESP32 (No home Wi-Fi or internet needed!)
#define WIFI_AP_SSID            "ARES_ROVER_WIFI"
#define WIFI_AP_PASSWORD        "ares123456"      // WPA2 Passphrase (min 8 characters)
#define WIFI_AP_CHANNEL         1                 // 2.4 GHz Wi-Fi channel (1 - 13)
#define MDNS_HOSTNAME           "ares"            // Allows connecting via http://ares.local
#define ENABLE_CAPTIVE_PORTAL   true              // Redirects mobile devices straight to dashboard

// ------------------------------------------------------------------------------
// 7. AUXILIARY RESCUE SUBSYSTEMS (LIGHTS & BEACON)
// ------------------------------------------------------------------------------
// Set to a valid GPIO pin number if external LEDs/MOSFETs or buzzers are connected.
// Set to -1 to run in software simulated mode without error if hardware is not yet attached.
#define PIN_HEADLIGHTS          -1   // Optional Headlights/Searchlight GPIO (set -1 if unassigned)
#define PIN_RESCUE_BEACON       -1   // Optional Warning Beacon/Buzzer GPIO (set -1 if unassigned)


