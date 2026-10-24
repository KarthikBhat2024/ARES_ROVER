#pragma once
#include <Arduino.h>
#include <Wire.h>
#include "arm_config.h"

// ==============================================================================
// AresArmController: 6-DOF Robotic Arm Controller via PCA9685 16-Channel PWM
// ==============================================================================

class AresArmController {
public:
    AresArmController();

    // Initialize I2C bus and configure PCA9685 12-bit PWM generator
    bool begin(uint8_t sdaPin = PIN_I2C_SDA, uint8_t sclPin = PIN_I2C_SCL, uint8_t i2cAddr = PCA9685_I2C_ADDRESS);

    // Periodic motion update loop (call every loop() iteration, non-blocking)
    void update();

    // Command a single servo to target angle (0-180 deg, clamped by configured safe limits)
    // servoIndex: 1 to 6 (or 0 to 5)
    bool setTargetAngle(uint8_t servoIndex, float targetDeg, float speedDps = DEFAULT_SPEED_DPS);

    // Command all 6 servos simultaneously to target angles
    bool setAllTargetAngles(const float targets[TOTAL_ARM_SERVOS], float speedDps = DEFAULT_SPEED_DPS);

    // Smoothly return all 6 joints to their default safe HOME position
    void homeAll(float speedDps = DEFAULT_SPEED_DPS);

    // Immediate Motion Freeze: Instantly halts all trajectory interpolation
    // Keeps active holding torque so arm doesn't drop under gravity
    void freeze();

    // Disable PWM signal to servos (CAUTION: Only use if arm is supported or resting safely!)
    void disableAllOutputs();

    // Enable PWM outputs
    void enableAllOutputs();

    // Setters & Runtime Calibration
    bool setServoLimits(uint8_t servoIndex, int16_t minLimit, int16_t maxLimit);
    bool setServoHome(uint8_t servoIndex, int16_t homeAngle);
    void setGlobalSpeed(float speedDps);

    // Getters
    float getCommandedAngle(uint8_t servoIndex) const;
    float getTargetAngle(uint8_t servoIndex) const;
    int16_t getMinAngle(uint8_t servoIndex) const;
    int16_t getMaxAngle(uint8_t servoIndex) const;
    int16_t getHomeAngle(uint8_t servoIndex) const;
    const char* getServoName(uint8_t servoIndex) const;
    const char* getServoModel(uint8_t servoIndex) const;
    bool isMoving() const { return _isMoving; }
    bool isFrozen() const { return _isFrozen; }
    bool isPwmEnabled() const { return _pwmEnabled; }
    float getGlobalSpeed() const { return _globalSpeedDps; }
    uint32_t getLastCommandTime() const { return _lastCommandTime; }

private:
    uint8_t _i2cAddr;
    uint8_t _sdaPin;
    uint8_t _sclPin;
    bool _initialized;
    bool _isMoving;
    bool _isFrozen;
    bool _pwmEnabled;
    float _globalSpeedDps;
    uint32_t _lastUpdateMs;
    uint32_t _lastCommandTime;

    // Servo State Arrays (0-indexed internally: 0 = Servo 1 ... 5 = Servo 6)
    float _currentAngles[TOTAL_ARM_SERVOS];
    float _targetAngles[TOTAL_ARM_SERVOS];
    float _servoSpeeds[TOTAL_ARM_SERVOS];
    int16_t _minAngles[TOTAL_ARM_SERVOS];
    int16_t _maxAngles[TOTAL_ARM_SERVOS];
    int16_t _homeAngles[TOTAL_ARM_SERVOS];
    const char* _servoNames[TOTAL_ARM_SERVOS];
    const char* _servoModels[TOTAL_ARM_SERVOS];

    // Low-Level PCA9685 I2C Register Protocol
    void writePwm(uint8_t channel, uint16_t onTick, uint16_t offTick);
    void writeRegister(uint8_t reg, uint8_t val);
    uint8_t readRegister(uint8_t reg);
    void setPwmFrequency(float freqHz);
    uint16_t angleToPwmTicks(float angleDeg, uint8_t channel);
};

extern AresArmController ArmController;
