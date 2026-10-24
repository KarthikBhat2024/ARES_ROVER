#include "motor_controller.h"

// Global Rover controller instance
AresRoverController Rover;

// ==============================================================================
// AresMotor Implementation
// ==============================================================================
AresMotor::AresMotor() 
    : _pinIn1(0), _pinIn2(0), _pinEn(0), _inverted(false), 
      _name("M?"), _controlMode(CONTROL_MODE_3_WIRE), _currentSpeed(0) {}

void AresMotor::configure(uint8_t pinIn1, uint8_t pinIn2, uint8_t pinEn, 
                          bool inverted, const char* name, uint8_t controlMode) {
    _pinIn1 = pinIn1;
    _pinIn2 = pinIn2;
    _pinEn = pinEn;
    _inverted = inverted;
    _name = name;
    _controlMode = controlMode;
    _currentSpeed = 0;
}

void AresMotor::writePwm(uint8_t pin, uint8_t duty) {
    // Arduino ESP32 core provides analogWrite(pin, duty) on all output-capable GPIOs.
    analogWrite(pin, duty);
}

void AresMotor::begin() {
    // CRITICAL: Ensure pins are driven LOW BEFORE enabling output mode.
    // This guarantees no motor twitching or erratic jump during boot.
    digitalWrite(_pinIn1, LOW);
    digitalWrite(_pinIn2, LOW);
    pinMode(_pinIn1, OUTPUT);
    pinMode(_pinIn2, OUTPUT);

    if (_controlMode == CONTROL_MODE_3_WIRE) {
        digitalWrite(_pinEn, LOW);
        pinMode(_pinEn, OUTPUT);
        writePwm(_pinEn, 0);
    }

    _currentSpeed = 0;
}

void AresMotor::setSpeed(int16_t speed) {
    // Constrain to valid 8-bit signed range (-255 to +255)
    speed = constrain(speed, -255, 255);
    _currentSpeed = speed;

    // Apply software direction inversion if configured
    int16_t effectiveSpeed = _inverted ? -speed : speed;

    if (effectiveSpeed > 0) {
        // FORWARD ROTATION
        uint8_t duty = (uint8_t)effectiveSpeed;
        if (_controlMode == CONTROL_MODE_3_WIRE) {
            digitalWrite(_pinIn1, HIGH);
            digitalWrite(_pinIn2, LOW);
            writePwm(_pinEn, duty);
        } else { // CONTROL_MODE_2_WIRE (Jumpers on L298N kept ON)
            writePwm(_pinIn1, duty);
            digitalWrite(_pinIn2, LOW);
        }
    } else if (effectiveSpeed < 0) {
        // REVERSE ROTATION
        uint8_t duty = (uint8_t)(-effectiveSpeed);
        if (_controlMode == CONTROL_MODE_3_WIRE) {
            digitalWrite(_pinIn1, LOW);
            digitalWrite(_pinIn2, HIGH);
            writePwm(_pinEn, duty);
        } else { // CONTROL_MODE_2_WIRE
            digitalWrite(_pinIn1, LOW);
            writePwm(_pinIn2, duty);
        }
    } else {
        // STOP / COAST
        stop();
    }
}

void AresMotor::stop() {
    _currentSpeed = 0;
    if (_controlMode == CONTROL_MODE_3_WIRE) {
        writePwm(_pinEn, 0);
        digitalWrite(_pinIn1, LOW);
        digitalWrite(_pinIn2, LOW);
    } else {
        digitalWrite(_pinIn1, LOW);
        digitalWrite(_pinIn2, LOW);
    }
}

void AresMotor::brake() {
    _currentSpeed = 0;
    if (_controlMode == CONTROL_MODE_3_WIRE) {
        // Active braking on L298N: ENA=HIGH, IN1=HIGH, IN2=HIGH
        writePwm(_pinEn, 255);
        digitalWrite(_pinIn1, HIGH);
        digitalWrite(_pinIn2, HIGH);
    } else {
        digitalWrite(_pinIn1, HIGH);
        digitalWrite(_pinIn2, HIGH);
    }
}

// ==============================================================================
// AresRoverController Implementation
// ==============================================================================
AresRoverController::AresRoverController()
    : _currentState(STATE_STOPPED), _lastCommandTime(0),
      _watchdogTimeoutMs(WATCHDOG_TIMEOUT_MS), _watchdogEnabled(true), _watchdogTriggered(false) {}

void AresRoverController::begin() {
    _currentState = STATE_STOPPED;
    _lastCommandTime = millis();
    _watchdogTimeoutMs = WATCHDOG_TIMEOUT_MS;
    _watchdogEnabled = true;
    _watchdogTriggered = false;

    // Configure all 6 motor channels
    // Front Axle (Driver 1)
    _m1.configure(PIN_M1_IN1, PIN_M1_IN2, PIN_M1_EN, INVERT_M1, "M1 (Front Left)");
    _m2.configure(PIN_M2_IN1, PIN_M2_IN2, PIN_M2_EN, INVERT_M2, "M2 (Front Right)");

    // Middle Axle (Driver 2)
    _m3.configure(PIN_M3_IN1, PIN_M3_IN2, PIN_M3_EN, INVERT_M3, "M3 (Mid Left)");
    _m4.configure(PIN_M4_IN1, PIN_M4_IN2, PIN_M4_EN, INVERT_M4, "M4 (Mid Right)");

    // Rear Axle (Driver 3)
    _m5.configure(PIN_M5_IN1, PIN_M5_IN2, PIN_M5_EN, INVERT_M5, "M5 (Rear Left)");
    _m6.configure(PIN_M6_IN1, PIN_M6_IN2, PIN_M6_EN, INVERT_M6, "M6 (Rear Right)");

    // Initialize pins & ensure all motors are at 0 duty cycle at startup
    _m1.begin();
    _m2.begin();
    _m3.begin();
    _m4.begin();
    _m5.begin();
    _m6.begin();

    stop();
}

void AresRoverController::setLeftBankSpeed(int16_t speed) {
    _m1.setSpeed(speed);
    _m3.setSpeed(speed);
    _m5.setSpeed(speed);
}

void AresRoverController::setRightBankSpeed(int16_t speed) {
    _m2.setSpeed(speed);
    _m4.setSpeed(speed);
    _m6.setSpeed(speed);
}

void AresRoverController::feedWatchdog() {
    _lastCommandTime = millis();
    _watchdogTriggered = false;
}

void AresRoverController::setWatchdogTimeout(uint32_t timeoutMs) {
    _watchdogTimeoutMs = timeoutMs;
}

void AresRoverController::enableWatchdog(bool enabled) {
    _watchdogEnabled = enabled;
}

void AresRoverController::update() {
    // Check safety watchdog timer
    if (_watchdogEnabled && (_currentState != STATE_STOPPED)) {
        if (millis() - _lastCommandTime > _watchdogTimeoutMs) {
            stop();
            _watchdogTriggered = true;
            Serial.println(F("\n[SAFETY WATCHDOG] Failsafe activated: Command timeout! All motors halted."));
        }
    }
}

void AresRoverController::forward(uint8_t speed) {
    feedWatchdog();
    _currentState = STATE_FORWARD;
    setLeftBankSpeed(speed);
    setRightBankSpeed(speed);
}

void AresRoverController::reverse(uint8_t speed) {
    feedWatchdog();
    _currentState = STATE_REVERSE;
    setLeftBankSpeed(-((int16_t)speed));
    setRightBankSpeed(-((int16_t)speed));
}

void AresRoverController::turnLeft(uint8_t speed) {
    // Pivot turn: left side stopped, right side moves forward
    feedWatchdog();
    _currentState = STATE_TURN_LEFT;
    setLeftBankSpeed(0);
    setRightBankSpeed(speed);
}

void AresRoverController::turnRight(uint8_t speed) {
    // Pivot turn: right side stopped, left side moves forward
    feedWatchdog();
    _currentState = STATE_TURN_RIGHT;
    setLeftBankSpeed(speed);
    setRightBankSpeed(0);
}

void AresRoverController::rotateLeft(uint8_t speed) {
    // Zero-radius skid-steer spin: left side reverse, right side forward
    feedWatchdog();
    _currentState = STATE_ROTATE_LEFT;
    setLeftBankSpeed(-((int16_t)speed));
    setRightBankSpeed(speed);
}

void AresRoverController::rotateRight(uint8_t speed) {
    // Zero-radius skid-steer spin: left side forward, right side reverse
    feedWatchdog();
    _currentState = STATE_ROTATE_RIGHT;
    setLeftBankSpeed(speed);
    setRightBankSpeed(-((int16_t)speed));
}

void AresRoverController::setDifferentialDrive(int16_t leftSpeed, int16_t rightSpeed) {
    feedWatchdog();
    _currentState = STATE_MANUAL_DRIVE;
    setLeftBankSpeed(leftSpeed);
    setRightBankSpeed(rightSpeed);
}

void AresRoverController::stop() {
    _currentState = STATE_STOPPED;
    _m1.stop();
    _m2.stop();
    _m3.stop();
    _m4.stop();
    _m5.stop();
    _m6.stop();
}

void AresRoverController::brake() {
    _currentState = STATE_STOPPED;
    _m1.brake();
    _m2.brake();
    _m3.brake();
    _m4.brake();
    _m5.brake();
    _m6.brake();
}

AresMotor& AresRoverController::getMotor(uint8_t index) {
    switch(index) {
        case 1: return _m1;
        case 2: return _m2;
        case 3: return _m3;
        case 4: return _m4;
        case 5: return _m5;
        case 6: return _m6;
        default: return _m1;
    }
}

const char* AresRoverController::getMotionStateString() const {
    switch(_currentState) {
        case STATE_STOPPED:       return "STOPPED";
        case STATE_FORWARD:       return "FORWARD";
        case STATE_REVERSE:       return "REVERSE";
        case STATE_TURN_LEFT:     return "TURN_LEFT";
        case STATE_TURN_RIGHT:    return "TURN_RIGHT";
        case STATE_ROTATE_LEFT:   return "ROTATE_LEFT";
        case STATE_ROTATE_RIGHT:  return "ROTATE_RIGHT";
        case STATE_MANUAL_DRIVE:  return "MANUAL_DRIVE";
        case STATE_TEST_MODE:     return "TEST_MODE";
        default:                  return "UNKNOWN";
    }
}

// ------------------------------------------------------------------------------
// Testing & Diagnostics Routines
// ------------------------------------------------------------------------------
void AresRoverController::testIndividualMotor(uint8_t motorIndex, int16_t speed, uint32_t durationMs) {
    _currentState = STATE_TEST_MODE;
    stop(); // safety first

    AresMotor* target = nullptr;
    switch(motorIndex) {
        case 1: target = &_m1; break;
        case 2: target = &_m2; break;
        case 3: target = &_m3; break;
        case 4: target = &_m4; break;
        case 5: target = &_m5; break;
        case 6: target = &_m6; break;
        default:
            Serial.println(F("[ERROR] Invalid motor index (Must be 1 to 6)."));
            return;
    }

    Serial.printf("[TEST] Testing %s at speed %d for %u ms...\n", target->getName(), speed, durationMs);
    target->setSpeed(speed);
    delay(durationMs);
    target->stop();
    Serial.printf("[TEST] %s test completed.\n", target->getName());
    _currentState = STATE_STOPPED;
}

void AresRoverController::testLeftBank(int16_t speed, uint32_t durationMs) {
    _currentState = STATE_TEST_MODE;
    stop();
    Serial.printf("[TEST] Testing LEFT BANK (M1 + M3 + M5) at speed %d for %u ms...\n", speed, durationMs);
    setLeftBankSpeed(speed);
    delay(durationMs);
    stop();
    Serial.println(F("[TEST] Left bank test completed."));
    _currentState = STATE_STOPPED;
}

void AresRoverController::testRightBank(int16_t speed, uint32_t durationMs) {
    _currentState = STATE_TEST_MODE;
    stop();
    Serial.printf("[TEST] Testing RIGHT BANK (M2 + M4 + M6) at speed %d for %u ms...\n", speed, durationMs);
    setRightBankSpeed(speed);
    delay(durationMs);
    stop();
    Serial.println(F("[TEST] Right bank test completed."));
    _currentState = STATE_STOPPED;
}

void AresRoverController::runSelfTestSequence() {
    _currentState = STATE_TEST_MODE;
    stop();
    Serial.println(F("\n=================================================="));
    Serial.println(F(">>> ARES ROVER AUTOMATED HARDWARE SELF-TEST <<<"));
    Serial.println(F("Place rover on a bench stand (wheels free to spin)!"));
    Serial.println(F("=================================================="));

    int16_t testSpeed = SPEED_TEST_SAFE;
    uint32_t singleMotorDuration = 1000; // 1 second each
    uint32_t bankDuration = 1500;        // 1.5 seconds

    // 1. Sequential single motor tests
    for (uint8_t i = 1; i <= 6; i++) {
        testIndividualMotor(i, testSpeed, singleMotorDuration);
        delay(400); // brief pause between motors to prevent power surges
    }

    // 2. Bank tests
    Serial.println(F("\nTesting Left Bank (M1, M3, M5)..."));
    testLeftBank(testSpeed, bankDuration);
    delay(500);

    Serial.println(F("\nTesting Right Bank (M2, M4, M6)..."));
    testRightBank(testSpeed, bankDuration);
    delay(500);

    // 3. Coordinated Motion Test (Brief Low-Speed Kicks)
    Serial.println(F("\nTesting Rover Forward (Low Speed)..."));
    forward(testSpeed);
    delay(1000);
    stop();
    delay(500);

    Serial.println(F("Testing Rover Reverse (Low Speed)..."));
    reverse(testSpeed);
    delay(1000);
    stop();
    delay(500);

    Serial.println(F("Testing Rover Rotate Left..."));
    rotateLeft(testSpeed);
    delay(1000);
    stop();
    delay(500);

    Serial.println(F("Testing Rover Rotate Right..."));
    rotateRight(testSpeed);
    delay(1000);
    stop();

    Serial.println(F("\n>>> Self-test completed successfully. All motors verified. <<<\n"));
    _currentState = STATE_STOPPED;
}
