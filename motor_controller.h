#pragma once
#include <Arduino.h>
#include "config.h"

// Rover motion states
enum RoverMotionState {
    STATE_STOPPED = 0,
    STATE_FORWARD,
    STATE_REVERSE,
    STATE_TURN_LEFT,
    STATE_TURN_RIGHT,
    STATE_ROTATE_LEFT,
    STATE_ROTATE_RIGHT,
    STATE_MANUAL_DRIVE,
    STATE_TEST_MODE
};

// ==============================================================================
// Single Motor Channel Controller
// ==============================================================================
class AresMotor {
public:
    AresMotor();
    
    // Configure pin assignments, inversion flag, and name
    void configure(uint8_t pinIn1, uint8_t pinIn2, uint8_t pinEn, 
                   bool inverted, const char* name, uint8_t controlMode = MOTOR_CONTROL_MODE);
    
    // Initialize GPIO pins and PWM hardware
    void begin();
    
    // Set motor power and direction:
    // speed: -255 (full reverse) to +255 (full forward), 0 = stop
    void setSpeed(int16_t speed);
    
    // Emergency stop / Coast to stop
    void stop();
    
    // Active electronic braking (shorts motor terminals)
    void brake();
    
    // Getters
    const char* getName() const { return _name; }
    int16_t getCurrentSpeed() const { return _currentSpeed; }

private:
    uint8_t _pinIn1;
    uint8_t _pinIn2;
    uint8_t _pinEn;
    bool _inverted;
    const char* _name;
    uint8_t _controlMode;
    int16_t _currentSpeed;
    
    void writePwm(uint8_t pin, uint8_t duty);
};

// ==============================================================================
// 6-Wheel Rover Differential Drive Controller
// ==============================================================================
class AresRoverController {
public:
    AresRoverController();

    // Initialize all 6 motors, PWM channels, and safety state
    void begin();
    
    // Non-blocking update called every loop() — handles failsafe watchdog
    void update();
    
    // Core motion commands
    void forward(uint8_t speed = SPEED_DEFAULT);
    void reverse(uint8_t speed = SPEED_DEFAULT);
    void turnLeft(uint8_t speed = SPEED_DEFAULT);    // Pivot turn (Left stopped, Right moves)
    void turnRight(uint8_t speed = SPEED_DEFAULT);   // Pivot turn (Right stopped, Left moves)
    void rotateLeft(uint8_t speed = SPEED_DEFAULT);  // Zero-radius spin (Left reverse, Right forward)
    void rotateRight(uint8_t speed = SPEED_DEFAULT); // Zero-radius spin (Left forward, Right reverse)
    void stop();
    void brake();
    
    // Direct differential drive (leftSpeed: -255 to +255, rightSpeed: -255 to +255)
    // Perfect for analog joysticks, curve steering, and web sliders
    void setDifferentialDrive(int16_t leftSpeed, int16_t rightSpeed);
    
    // Individual & Bank Motor Testing
    void testIndividualMotor(uint8_t motorIndex, int16_t speed, uint32_t durationMs);
    void testLeftBank(int16_t speed, uint32_t durationMs);
    void testRightBank(int16_t speed, uint32_t durationMs);
    void runSelfTestSequence();
    
    // Watchdog & Safety Management
    void feedWatchdog();
    void setWatchdogTimeout(uint32_t timeoutMs);
    void enableWatchdog(bool enabled);
    bool isWatchdogTriggered() const { return _watchdogTriggered; }
    RoverMotionState getMotionState() const { return _currentState; }
    const char* getMotionStateString() const;

    // Motor channel accessors
    AresMotor& getMotor(uint8_t index); // 1 = M1 ... 6 = M6

private:
    AresMotor _m1; // Front Left
    AresMotor _m2; // Front Right
    AresMotor _m3; // Middle Left
    AresMotor _m4; // Middle Right
    AresMotor _m5; // Rear Left
    AresMotor _m6; // Rear Right

    RoverMotionState _currentState;
    uint32_t _lastCommandTime;
    uint32_t _watchdogTimeoutMs;
    bool _watchdogEnabled;
    bool _watchdogTriggered;
    
    void setLeftBankSpeed(int16_t speed);
    void setRightBankSpeed(int16_t speed);
};

extern AresRoverController Rover;
