/*
 * ==============================================================================
 * Project: ARES — Autonomous Response & Exploration System (Rescue Rover)
 * Component: Main Firmware (ESP32 Rover Host & Dual-Microcontroller Web Gateway)
 * Target: ESP32 Dev Board (NodeMCU-32S / DOIT ESP32 DevKit V1)
 * ==============================================================================
 * 
 * Capabilities:
 * - Full 6-motor differential drive control (M1..M6).
 * - 3x L298N dual H-bridge motor driver interface (Hardware GPIO assignments preserved).
 * - Multi-directional kinematics: Forward, Reverse, Pivot Left/Right, Skid Spin Left/Right.
 * - Hardware safety watchdog: Automatically halts all motors if commands timeout.
 * - Software polarity calibration (no rewiring needed if a motor spins backward).
 * - Standalone Wi-Fi Access Point hosting high-tech unified command dashboard.
 * - Inter-controller HTTP Gateway to upper robotic arm (ESP8266 Subsystem).
 * - Master Emergency Stop halting both Rover drive and Arm servos simultaneously.
 * - Interactive Serial Diagnostic CLI.
 * 
 * Baud Rate: 115200 baud
 * ==============================================================================
 */

#include "config.h"
#include "motor_controller.h"
#include "web_server_manager.h"
#include "arm_gateway.h"

// Current drive speed preset (0 - 255)
uint8_t currentSpeed = SPEED_DEFAULT;

void printBanner() {
    Serial.println(F("\n======================================================="));
    Serial.println(F("    ARES: AUTONOMOUS RESPONSE & EXPLORATION SYSTEM    "));
    Serial.println(F("        Smart 6-Wheel Rover & Arm Unified Gateway      "));
    Serial.println(F("======================================================="));
    Serial.print(F("Control Mode:       "));
    #if (MOTOR_CONTROL_MODE == CONTROL_MODE_3_WIRE)
    Serial.println(F("3-WIRE (ENA/ENB PWM, Jumpers Removed)"));
    #else
    Serial.println(F("2-WIRE (IN1/IN2 PWM, Jumpers Kept On)"));
    #endif

    Serial.printf("Watchdog Timeout:   %d ms\n", WATCHDOG_TIMEOUT_MS);
    Serial.printf("Default Speed:      %d / 255\n", currentSpeed);
    Serial.printf("Wi-Fi SSID:         %s\n", WIFI_AP_SSID);
    Serial.printf("Wi-Fi Password:     %s\n", WIFI_AP_PASSWORD);
    Serial.println(F("Web Dashboard URL:  http://192.168.4.1 or http://ares.local"));
    Serial.println(F("Arm Subsystem Link: http://192.168.4.20/api (via Gateway)"));
    Serial.println(F("Motor Mapping:"));
    Serial.println(F("  Left Bank:  M1 (FL: pin 13/14), M3 (ML: pin 16/17), M5 (RL: pin 21/22)"));
    Serial.println(F("  Right Bank: M2 (FR: pin 26/27), M4 (MR: pin 18/19), M6 (RR: pin 5/15)"));
    Serial.println(F("-------------------------------------------------------"));
    Serial.println(F("All motors initialized to STOPPED state."));
    Serial.println(F("Type '?' or 'h' for Interactive CLI commands.\n"));
}

void printHelpMenu() {
    Serial.println(F("\n--- ARES INTERACTIVE MOTOR & ARM CLI ---"));
    Serial.println(F("  Motion Controls:"));
    Serial.println(F("    w : Drive FORWARD"));
    Serial.println(F("    s : Drive REVERSE"));
    Serial.println(F("    q : Pivot TURN LEFT (Right bank pushes)"));
    Serial.println(F("    e : Pivot TURN RIGHT (Left bank pushes)"));
    Serial.println(F("    a : Zero-radius ROTATE LEFT (Left rev, Right fwd)"));
    Serial.println(F("    d : Zero-radius ROTATE RIGHT (Left fwd, Right rev)"));
    Serial.println(F("    x or [SPACE] : Hard STOP rover motors"));
    Serial.println(F("    b : Dynamic BRAKE (Active electrical brake)"));
    Serial.println(F("    estop : MASTER EMERGENCY STOP (Stops Rover AND Arm)"));
    Serial.println(F(""));
    Serial.println(F("  Speed Selection:"));
    Serial.println(F("    1 to 9 : Adjust speed (1 = ~35% safe crawl, 5 = 62% cruise, 9 = 100% max)"));
    Serial.println(F(""));
    Serial.println(F("  Robotic Arm Gateway Commands:"));
    Serial.println(F("    arm status : Query ESP8266 arm joint status & link"));
    Serial.println(F("    arm home   : Request arm to return to safe HOME"));
    Serial.println(F("    arm stop   : Request arm motion freeze"));
    Serial.println(F(""));
    Serial.println(F("  Diagnostics & Hardware Verification:"));
    Serial.println(F("    t : Run automated full-vehicle self-test sequence"));
    Serial.println(F("    l : Test LEFT bank only (M1 + M3 + M5) for 1.5s"));
    Serial.println(F("    r : Test RIGHT bank only (M2 + M4 + M6) for 1.5s"));
    Serial.println(F("    1-6 (with +/-): e.g. 'm1+' tests M1 fwd, 'm1-' tests M1 rev"));
    Serial.println(F("    h or ? : Show this menu"));
    Serial.println(F("-----------------------------------------\n"));
}

void handleSerialCommand(String cmd) {
    cmd.trim();
    if (cmd.length() == 0) return;

    if (cmd.equalsIgnoreCase("estop")) {
        Serial.println(F("[SAFETY] MASTER EMERGENCY STOP INITIATED"));
        Rover.stop();
        ArmGateway.sendStopCommand();
        return;
    }

    if (cmd.startsWith("arm") || cmd.startsWith("ARM")) {
        String sub = cmd.substring(3);
        sub.trim();
        if (sub.equalsIgnoreCase("status")) {
            Serial.println(F("[GATEWAY] Querying ESP8266 Arm Status..."));
            Serial.println(ArmGateway.getArmStatus());
            return;
        } else if (sub.equalsIgnoreCase("home")) {
            Serial.println(F("[GATEWAY] Dispatching Arm HOME command..."));
            ArmGateway.sendHomeCommand();
            return;
        } else if (sub.equalsIgnoreCase("stop") || sub.equalsIgnoreCase("freeze")) {
            Serial.println(F("[GATEWAY] Dispatching Arm FREEZE command..."));
            ArmGateway.sendStopCommand();
            return;
        }
    }

    char key = tolower(cmd.charAt(0));

    // Handle individual motor testing syntax: "m1+", "m2-", "m3+", etc.
    if (cmd.length() >= 3 && (key == 'm') && isDigit(cmd.charAt(1))) {
        uint8_t motorIdx = cmd.charAt(1) - '0';
        char dirChar = cmd.charAt(2);
        int16_t testSpd = (dirChar == '-') ? -SPEED_TEST_SAFE : SPEED_TEST_SAFE;
        Rover.testIndividualMotor(motorIdx, testSpd, 1200);
        return;
    }

    switch (key) {
        case 'w':
            Serial.printf("[MOTION] Driving FORWARD (Speed: %d)\n", currentSpeed);
            Rover.forward(currentSpeed);
            break;

        case 's':
            Serial.printf("[MOTION] Driving REVERSE (Speed: %d)\n", currentSpeed);
            Rover.reverse(currentSpeed);
            break;

        case 'q':
            Serial.printf("[MOTION] Pivot Turn LEFT (Speed: %d)\n", currentSpeed);
            Rover.turnLeft(currentSpeed);
            break;

        case 'e':
            Serial.printf("[MOTION] Pivot Turn RIGHT (Speed: %d)\n", currentSpeed);
            Rover.turnRight(currentSpeed);
            break;

        case 'a':
            Serial.printf("[MOTION] Zero-radius Skid ROTATE LEFT (Speed: %d)\n", currentSpeed);
            Rover.rotateLeft(currentSpeed);
            break;

        case 'd':
            Serial.printf("[MOTION] Zero-radius Skid ROTATE RIGHT (Speed: %d)\n", currentSpeed);
            Rover.rotateRight(currentSpeed);
            break;

        case 'x':
        case ' ':
            Serial.println(F("[MOTION] STOPPED"));
            Rover.stop();
            break;

        case 'b':
            Serial.println(F("[MOTION] BRAKED (Active)"));
            Rover.brake();
            break;

        case 't':
            Rover.runSelfTestSequence();
            break;

        case 'l':
            Rover.testLeftBank(SPEED_TEST_SAFE, 1500);
            break;

        case 'r':
            Rover.testRightBank(SPEED_TEST_SAFE, 1500);
            break;

        case 'h':
        case '?':
            printHelpMenu();
            break;

        default:
            // Check if speed preset '1' - '9'
            if (key >= '1' && key <= '9') {
                uint8_t level = key - '0';
                currentSpeed = map(level, 1, 9, SPEED_MIN, SPEED_MAX);
                Serial.printf("[CONFIG] Speed set to level %d (%d/255 = %d%%)\n", 
                              level, currentSpeed, (currentSpeed * 100) / 255);
                if (Rover.getMotionState() == STATE_FORWARD) Rover.forward(currentSpeed);
                else if (Rover.getMotionState() == STATE_REVERSE) Rover.reverse(currentSpeed);
                else if (Rover.getMotionState() == STATE_TURN_LEFT) Rover.turnLeft(currentSpeed);
                else if (Rover.getMotionState() == STATE_TURN_RIGHT) Rover.turnRight(currentSpeed);
                else if (Rover.getMotionState() == STATE_ROTATE_LEFT) Rover.rotateLeft(currentSpeed);
                else if (Rover.getMotionState() == STATE_ROTATE_RIGHT) Rover.rotateRight(currentSpeed);
            } else {
                Serial.printf("[CLI] Unknown command: '%c'. Type '?' for help.\n", key);
            }
            break;
    }
}

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(500);

    // 1. Initialize all 6 motor channels and hardware PWM
    Rover.begin();

    // 2. Initialize Wi-Fi Access Point, Web Teleoperation Dashboard, and Arm Gateway
    WebManager.begin();

    // 3. Display diagnostic splash screen & network coordinates
    printBanner();
}

void loop() {
    // 1. Maintain motor failsafe watchdog (halts vehicle if Wi-Fi/CLI commands cease)
    Rover.update();

    // 2. Service incoming HTTP teleoperation and DNS captive portal requests
    WebManager.update();

    // 3. Interactive Serial CLI
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        handleSerialCommand(cmd);
    }
}
