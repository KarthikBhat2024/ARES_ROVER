/*
 * ==============================================================================
 * Project: ARES — Autonomous Response & Exploration System (Robotic Arm Subsystem)
 * Target: ESP8266 NodeMCU (ESP-12E / ESP-12F)
 * Hardware:
 *   - NodeMCU ESP8266 development board
 *   - PCA9685 16-channel 12-bit PWM servo controller
 *   - 3x MG996R high-torque servos on PCA9685 channels 0, 1, 2
 *   - 3x SG90 micro servos on PCA9685 channels 3, 4, 5
 *   - 5V DC Buck Converter supplying dedicated servo rail power (V+)
 *   - Common Ground (GND) shared between NodeMCU, PCA9685, and Buck Converter
 * ==============================================================================
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "arm_config.h"
#include "pca9685_arm_controller.h"
#include "arm_web_server.h"

void printBanner() {
    Serial.println(F("\n======================================================="));
    Serial.println(F("    ARES: AUTONOMOUS RESPONSE & EXPLORATION SYSTEM    "));
    Serial.println(F("         Upper Robotic Arm Subsystem (ESP8266)         "));
    Serial.println(F("======================================================="));
    Serial.println(F("Hardware Mapping (PCA9685):"));
    Serial.println(F("  Ch 0: Servo 1 - Base Turret    (MG996R) [10..170 deg]"));
    Serial.println(F("  Ch 1: Servo 2 - Shoulder Pitch (MG996R) [20..160 deg]"));
    Serial.println(F("  Ch 2: Servo 3 - Elbow Pitch    (MG996R) [20..160 deg]"));
    Serial.println(F("  Ch 3: Servo 4 - Wrist Pitch    (SG90)   [15..165 deg]"));
    Serial.println(F("  Ch 4: Servo 5 - Wrist Roll     (SG90)   [10..170 deg]"));
    Serial.println(F("  Ch 5: Servo 6 - Gripper Jaw    (SG90)   [40..135 deg]"));
    Serial.printf("I2C Pins:           SDA = GPIO %d (D2), SCL = GPIO %d (D1)\n", PIN_I2C_SDA, PIN_I2C_SCL);
    Serial.printf("Target Rover AP:    %s\n", ARM_WIFI_SSID);
    Serial.println(F("-------------------------------------------------------"));
}

void connectToWiFi() {
    Serial.print(F("[ARM NET] Connecting to Rover Wi-Fi AP: "));
    Serial.println(ARM_WIFI_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);

    #if ARM_USE_STATIC_IP
    IPAddress staticIP(ARM_STATIC_IP);
    IPAddress gateway(ARM_GATEWAY_IP);
    IPAddress subnet(ARM_SUBNET_MASK);
    IPAddress dns(ARM_DNS_IP);
    WiFi.config(staticIP, gateway, subnet, dns);
    Serial.printf("[ARM NET] Static IP configured: %s\n", staticIP.toString().c_str());
    #endif

    WiFi.begin(ARM_WIFI_SSID, ARM_WIFI_PASSWORD);

    uint8_t attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(300);
        Serial.print(F("."));
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(F("\n[ARM NET] Wi-Fi Link Established!"));
        Serial.printf("[ARM NET] IP Address:  %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("[ARM NET] Signal RSSI: %d dBm\n", WiFi.RSSI());
        Serial.printf("[ARM NET] REST API:    http://%s/api/status\n\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println(F("\n[ARM WARNING] Could not immediately connect to Rover AP."));
        Serial.println(F("[ARM WARNING] Ensure the ESP32 Rover is powered on. Retrying in background..."));
    }
}

void handleSerialCli(String cmd) {
    cmd.trim();
    if (cmd.length() == 0) return;

    if (cmd == "help" || cmd == "?") {
        Serial.println(F("\n--- ARES ARM SERIAL CLI ---"));
        Serial.println(F("  s<id> <deg> : Move servo 1-6 (e.g. 's1 90', 's6 45')"));
        Serial.println(F("  home        : Move all 6 joints to safe HOME position"));
        Serial.println(F("  stop        : Freeze arm motion instantly"));
        Serial.println(F("  status      : Print current joint angles and limits"));
        Serial.println(F("  speed <dps> : Set global motion speed in deg/sec"));
        Serial.println(F("---------------------------\n"));
        return;
    }

    if (cmd.equalsIgnoreCase("home")) {
        ArmController.homeAll();
        return;
    }

    if (cmd.equalsIgnoreCase("stop") || cmd.equalsIgnoreCase("freeze")) {
        ArmController.freeze();
        return;
    }

    if (cmd.equalsIgnoreCase("status")) {
        Serial.println(F("\n--- CURRENT ARM JOINT TELEMETRY ---"));
        for (uint8_t i = 1; i <= TOTAL_ARM_SERVOS; i++) {
            Serial.printf("  Servo %d (%s, %s): Commanded=%.1f deg, Target=%.1f deg, Limits=[%d..%d deg]\n",
                          i, ArmController.getServoName(i), ArmController.getServoModel(i),
                          ArmController.getCommandedAngle(i), ArmController.getTargetAngle(i),
                          ArmController.getMinAngle(i), ArmController.getMaxAngle(i));
        }
        Serial.printf("  Motion State: %s, Holding Torque: %s\n\n",
                      ArmController.isMoving() ? "MOVING" : "IDLE",
                      ArmController.isPwmEnabled() ? "ACTIVE" : "DISABLED");
        return;
    }

    // Parse servo command "s1 90" or "1 90"
    if ((cmd.charAt(0) == 's' || cmd.charAt(0) == 'S') && isDigit(cmd.charAt(1))) {
        uint8_t id = cmd.charAt(1) - '0';
        int spaceIdx = cmd.indexOf(' ');
        if (spaceIdx > 0) {
            float angle = cmd.substring(spaceIdx + 1).toFloat();
            ArmController.setTargetAngle(id, angle);
            return;
        }
    }

    Serial.println(F("[ARM CLI] Unknown command. Type '?' or 'help' for instructions."));
}

void setup() {
    Serial.begin(ARM_SERIAL_BAUD_RATE);
    delay(500);

    printBanner();

    // 1. Initialize PCA9685 I2C Servo Controller
    if (!ArmController.begin(PIN_I2C_SDA, PIN_I2C_SCL, PCA9685_I2C_ADDRESS)) {
        Serial.println(F("[FATAL] Hardware check failed: PCA9685 unresponsive!"));
    }

    // 2. Connect to ESP32 Wi-Fi Hotspot
    connectToWiFi();

    // 3. Initialize REST Web Server
    ArmServer.begin();

    Serial.println(F("[ARM READY] Subsystem initialized. Awaiting teleoperation commands.\n"));
}

void loop() {
    // Service smooth servo trajectory interpolation (non-blocking)
    ArmController.update();

    // Service HTTP teleoperation requests
    ArmServer.update();

    // Interactive Serial Diagnostics
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        handleSerialCli(input);
    }
}
