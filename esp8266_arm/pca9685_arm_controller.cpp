#include "pca9685_arm_controller.h"

// PCA9685 I2C Registers
#define PCA9685_REG_MODE1       0x00
#define PCA9685_REG_MODE2       0x01
#define PCA9685_REG_SUBADR1     0x02
#define PCA9685_REG_SUBADR2     0x03
#define PCA9685_REG_SUBADR3     0x04
#define PCA9685_REG_ALLCALLADR  0x05
#define PCA9685_REG_LED0_ON_L   0x06
#define PCA9685_REG_LED0_ON_H   0x07
#define PCA9685_REG_LED0_OFF_L  0x08
#define PCA9685_REG_LED0_OFF_H  0x09
#define PCA9685_REG_ALL_LED_ON_L  0xFA
#define PCA9685_REG_ALL_LED_ON_H  0xFB
#define PCA9685_REG_ALL_LED_OFF_L 0xFC
#define PCA9685_REG_ALL_LED_OFF_H 0xFD
#define PCA9685_REG_PRESCALE    0xFE

// Bits
#define MODE1_RESTART           0x80
#define MODE1_EXTCLK            0x40
#define MODE1_AUTO_INC          0x20
#define MODE1_SLEEP             0x10
#define MODE1_ALLCALL           0x01

#define MODE2_OUTDRV            0x04

AresArmController ArmController;

AresArmController::AresArmController()
    : _i2cAddr(PCA9685_I2C_ADDRESS),
      _sdaPin(PIN_I2C_SDA),
      _sclPin(PIN_I2C_SCL),
      _initialized(false),
      _isMoving(false),
      _isFrozen(false),
      _pwmEnabled(true),
      _globalSpeedDps(DEFAULT_SPEED_DPS),
      _lastUpdateMs(0),
      _lastCommandTime(0)
{
    // Initialize default safe configurations
    for (uint8_t i = 0; i < TOTAL_ARM_SERVOS; i++) {
        _minAngles[i]    = DEFAULT_SERVO_CONFIGS[i].minAngle;
        _maxAngles[i]    = DEFAULT_SERVO_CONFIGS[i].maxAngle;
        _homeAngles[i]   = DEFAULT_SERVO_CONFIGS[i].homeAngle;
        _servoNames[i]   = DEFAULT_SERVO_CONFIGS[i].name;
        _servoModels[i]  = DEFAULT_SERVO_CONFIGS[i].model;
        _currentAngles[i] = _homeAngles[i];
        _targetAngles[i]  = _homeAngles[i];
        _servoSpeeds[i]  = DEFAULT_SPEED_DPS;
    }
}

bool AresArmController::begin(uint8_t sdaPin, uint8_t sclPin, uint8_t i2cAddr) {
    _sdaPin = sdaPin;
    _sclPin = sclPin;
    _i2cAddr = i2cAddr;

    Serial.printf("[ARM HW] Initializing PCA9685 on SDA=%d, SCL=%d, Addr=0x%02X...\n", 
                  _sdaPin, _sclPin, _i2cAddr);

    Wire.begin(_sdaPin, _sclPin);
    Wire.setClock(100000); // 100 kHz I2C clock (reliable with standard wire lengths)

    // Probe PCA9685 address
    Wire.beginTransmission(_i2cAddr);
    if (Wire.endTransmission() != 0) {
        Serial.printf("[ARM ERROR] PCA9685 not detected at I2C address 0x%02X! Check 3.3V/5V and I2C wiring.\n", _i2cAddr);
        _initialized = false;
        return false;
    }

    // Reset PCA9685
    writeRegister(PCA9685_REG_MODE1, MODE1_RESTART);
    delay(10);

    // Set 50 Hz PWM frequency for servos
    setPwmFrequency(PCA9685_PWM_FREQ_HZ);

    // Configure MODE2: Totem-pole output driver (standard for PCA9685 servo boards)
    writeRegister(PCA9685_REG_MODE2, MODE2_OUTDRV);

    // Configure MODE1: Enable Auto-Increment for fast register writes
    writeRegister(PCA9685_REG_MODE1, MODE1_AUTO_INC);

    // Optional OE hardware pin
    if (PIN_PCA9685_OE != -1) {
        pinMode(PIN_PCA9685_OE, OUTPUT);
        digitalWrite(PIN_PCA9685_OE, LOW); // Active LOW enables outputs
    }

    _initialized = true;
    _lastUpdateMs = millis();
    _lastCommandTime = millis();
    Serial.println(F("[ARM HW] PCA9685 successfully configured at 50 Hz."));

    #if AUTO_MOVE_ON_BOOT
    homeAll();
    #else
    Serial.println(F("[ARM SAFETY] Arm holding idle. Awaiting explicit operator command."));
    #endif

    return true;
}

void AresArmController::writeRegister(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(_i2cAddr);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

uint8_t AresArmController::readRegister(uint8_t reg) {
    Wire.beginTransmission(_i2cAddr);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(_i2cAddr, (uint8_t)1);
    if (Wire.available()) {
        return Wire.read();
    }
    return 0;
}

void AresArmController::setPwmFrequency(float freqHz) {
    // Prescale calculation based on internal 25 MHz oscillator:
    // prescale = round(25000000 / (4096 * freqHz)) - 1
    float prescaleVal = 25000000.0f / (4096.0f * freqHz) - 1.0f;
    uint8_t prescale = (uint8_t)floor(prescaleVal + 0.5f);

    uint8_t oldMode = readRegister(PCA9685_REG_MODE1);
    uint8_t sleepMode = (oldMode & 0x7F) | MODE1_SLEEP; // Put oscillator to sleep to write prescale

    writeRegister(PCA9685_REG_MODE1, sleepMode);
    writeRegister(PCA9685_REG_PRESCALE, prescale);
    writeRegister(PCA9685_REG_MODE1, oldMode);
    delay(5);
    writeRegister(PCA9685_REG_MODE1, oldMode | MODE1_RESTART | MODE1_AUTO_INC);
}

void AresArmController::writePwm(uint8_t channel, uint16_t onTick, uint16_t offTick) {
    if (!_initialized || !_pwmEnabled || channel >= 16) return;

    uint8_t reg = PCA9685_REG_LED0_ON_L + 4 * channel;
    Wire.beginTransmission(_i2cAddr);
    Wire.write(reg);
    Wire.write((uint8_t)(onTick & 0xFF));
    Wire.write((uint8_t)(onTick >> 8));
    Wire.write((uint8_t)(offTick & 0xFF));
    Wire.write((uint8_t)(offTick >> 8));
    Wire.endTransmission();
}

uint16_t AresArmController::angleToPwmTicks(float angleDeg, uint8_t channel) {
    // Clamp to configured physical safety limits
    int16_t minLimit = _minAngles[channel];
    int16_t maxLimit = _maxAngles[channel];
    float clamped = constrain(angleDeg, (float)minLimit, (float)maxLimit);

    // Calculate pulse in microseconds (544us to 2400us default)
    float pulseUs = SERVO_PULSE_MIN_US + (clamped / 180.0f) * (SERVO_PULSE_MAX_US - SERVO_PULSE_MIN_US);

    // Convert microseconds to 12-bit PCA9685 ticks (20,000us per period at 50Hz)
    // ticks = (pulseUs / 20000) * 4096
    float ticks = (pulseUs * 4096.0f) / 20000.0f;
    return (uint16_t)round(ticks);
}

bool AresArmController::setTargetAngle(uint8_t servoIndex, float targetDeg, float speedDps) {
    // Accept either 1-based (1..6) or 0-based (0..5) index
    uint8_t idx = (servoIndex >= 1 && servoIndex <= TOTAL_ARM_SERVOS) ? (servoIndex - 1) : servoIndex;
    if (idx >= TOTAL_ARM_SERVOS) return false;

    // Validate and clamp against conservative limits
    float clamped = constrain(targetDeg, (float)_minAngles[idx], (float)_maxAngles[idx]);
    _targetAngles[idx] = clamped;
    _servoSpeeds[idx] = (speedDps > 0) ? speedDps : _globalSpeedDps;
    _isFrozen = false;
    _lastCommandTime = millis();

    Serial.printf("[ARM CMD] Servo %d (%s) target: %.1f deg (Safe range: %d..%d deg, Spd: %.1f dps)\n",
                  idx + 1, _servoNames[idx], clamped, _minAngles[idx], _maxAngles[idx], _servoSpeeds[idx]);
    return true;
}

bool AresArmController::setAllTargetAngles(const float targets[TOTAL_ARM_SERVOS], float speedDps) {
    _isFrozen = false;
    _lastCommandTime = millis();
    for (uint8_t i = 0; i < TOTAL_ARM_SERVOS; i++) {
        _targetAngles[i] = constrain(targets[i], (float)_minAngles[i], (float)_maxAngles[i]);
        _servoSpeeds[i] = (speedDps > 0) ? speedDps : _globalSpeedDps;
    }
    Serial.println(F("[ARM CMD] All 6 joint targets updated."));
    return true;
}

void AresArmController::homeAll(float speedDps) {
    _isFrozen = false;
    _lastCommandTime = millis();
    for (uint8_t i = 0; i < TOTAL_ARM_SERVOS; i++) {
        _targetAngles[i] = (float)_homeAngles[i];
        _servoSpeeds[i] = (speedDps > 0) ? speedDps : _globalSpeedDps;
    }
    Serial.println(F("[ARM CMD] Returning all 6 joints to configured HOME position."));
}

void AresArmController::freeze() {
    _isFrozen = true;
    for (uint8_t i = 0; i < TOTAL_ARM_SERVOS; i++) {
        _targetAngles[i] = _currentAngles[i]; // Lock target to current location
    }
    _isMoving = false;
    Serial.println(F("[ARM SAFETY] Motion FROZEN. Current positions held under torque."));
}

void AresArmController::disableAllOutputs() {
    _pwmEnabled = false;
    if (PIN_PCA9685_OE != -1) {
        digitalWrite(PIN_PCA9685_OE, HIGH); // Disable outputs
    } else {
        // Send 0 ticks to all channels
        for (uint8_t ch = 0; ch < TOTAL_ARM_SERVOS; ch++) {
            writePwm(ch, 0, 0);
        }
    }
    Serial.println(F("[ARM WARNING] All servo PWM outputs DISABLED. Holding torque released."));
}

void AresArmController::enableAllOutputs() {
    _pwmEnabled = true;
    if (PIN_PCA9685_OE != -1) {
        digitalWrite(PIN_PCA9685_OE, LOW);
    }
    // Re-apply current angles
    for (uint8_t ch = 0; ch < TOTAL_ARM_SERVOS; ch++) {
        writePwm(ch, 0, angleToPwmTicks(_currentAngles[ch], ch));
    }
    Serial.println(F("[ARM HW] Servo PWM outputs ENABLED."));
}

bool AresArmController::setServoLimits(uint8_t servoIndex, int16_t minLimit, int16_t maxLimit) {
    uint8_t idx = (servoIndex >= 1 && servoIndex <= TOTAL_ARM_SERVOS) ? (servoIndex - 1) : servoIndex;
    if (idx >= TOTAL_ARM_SERVOS || minLimit >= maxLimit || minLimit < 0 || maxLimit > 180) {
        return false;
    }
    _minAngles[idx] = minLimit;
    _maxAngles[idx] = maxLimit;
    // Ensure current and target remain within new limits
    _targetAngles[idx] = constrain(_targetAngles[idx], (float)minLimit, (float)maxLimit);
    _currentAngles[idx] = constrain(_currentAngles[idx], (float)minLimit, (float)maxLimit);
    Serial.printf("[ARM CALIB] Servo %d limits updated: [%d, %d]\n", idx + 1, minLimit, maxLimit);
    return true;
}

bool AresArmController::setServoHome(uint8_t servoIndex, int16_t homeAngle) {
    uint8_t idx = (servoIndex >= 1 && servoIndex <= TOTAL_ARM_SERVOS) ? (servoIndex - 1) : servoIndex;
    if (idx >= TOTAL_ARM_SERVOS || homeAngle < _minAngles[idx] || homeAngle > _maxAngles[idx]) {
        return false;
    }
    _homeAngles[idx] = homeAngle;
    Serial.printf("[ARM CALIB] Servo %d home updated: %d deg\n", idx + 1, homeAngle);
    return true;
}

void AresArmController::setGlobalSpeed(float speedDps) {
    if (speedDps > 5.0f && speedDps <= 180.0f) {
        _globalSpeedDps = speedDps;
        for (uint8_t i = 0; i < TOTAL_ARM_SERVOS; i++) {
            _servoSpeeds[i] = speedDps;
        }
    }
}

float AresArmController::getCommandedAngle(uint8_t servoIndex) const {
    uint8_t idx = (servoIndex >= 1 && servoIndex <= TOTAL_ARM_SERVOS) ? (servoIndex - 1) : servoIndex;
    return (idx < TOTAL_ARM_SERVOS) ? _currentAngles[idx] : 0.0f;
}

float AresArmController::getTargetAngle(uint8_t servoIndex) const {
    uint8_t idx = (servoIndex >= 1 && servoIndex <= TOTAL_ARM_SERVOS) ? (servoIndex - 1) : servoIndex;
    return (idx < TOTAL_ARM_SERVOS) ? _targetAngles[idx] : 0.0f;
}

int16_t AresArmController::getMinAngle(uint8_t servoIndex) const {
    uint8_t idx = (servoIndex >= 1 && servoIndex <= TOTAL_ARM_SERVOS) ? (servoIndex - 1) : servoIndex;
    return (idx < TOTAL_ARM_SERVOS) ? _minAngles[idx] : 0;
}

int16_t AresArmController::getMaxAngle(uint8_t servoIndex) const {
    uint8_t idx = (servoIndex >= 1 && servoIndex <= TOTAL_ARM_SERVOS) ? (servoIndex - 1) : servoIndex;
    return (idx < TOTAL_ARM_SERVOS) ? _maxAngles[idx] : 180;
}

int16_t AresArmController::getHomeAngle(uint8_t servoIndex) const {
    uint8_t idx = (servoIndex >= 1 && servoIndex <= TOTAL_ARM_SERVOS) ? (servoIndex - 1) : servoIndex;
    return (idx < TOTAL_ARM_SERVOS) ? _homeAngles[idx] : 90;
}

const char* AresArmController::getServoName(uint8_t servoIndex) const {
    uint8_t idx = (servoIndex >= 1 && servoIndex <= TOTAL_ARM_SERVOS) ? (servoIndex - 1) : servoIndex;
    return (idx < TOTAL_ARM_SERVOS) ? _servoNames[idx] : "Unknown";
}

const char* AresArmController::getServoModel(uint8_t servoIndex) const {
    uint8_t idx = (servoIndex >= 1 && servoIndex <= TOTAL_ARM_SERVOS) ? (servoIndex - 1) : servoIndex;
    return (idx < TOTAL_ARM_SERVOS) ? _servoModels[idx] : "Unknown";
}

void AresArmController::update() {
    if (!_initialized || !_pwmEnabled) return;

    uint32_t now = millis();
    if (now - _lastUpdateMs < MOTION_UPDATE_INTERVAL_MS) {
        return; // Wait for next tick interval
    }

    float dtSec = (now - _lastUpdateMs) / 1000.0f;
    _lastUpdateMs = now;

    if (_isFrozen) {
        _isMoving = false;
        return;
    }

    bool anyJointMoving = false;

    // Smoothly step all 6 servos toward target angles
    for (uint8_t i = 0; i < TOTAL_ARM_SERVOS; i++) {
        float diff = _targetAngles[i] - _currentAngles[i];
        if (fabs(diff) > 0.05f) {
            anyJointMoving = true;
            float maxStep = _servoSpeeds[i] * dtSec;

            if (fabs(diff) <= maxStep) {
                _currentAngles[i] = _targetAngles[i];
            } else {
                _currentAngles[i] += (diff > 0 ? maxStep : -maxStep);
            }

            // Output updated PWM tick to PCA9685
            uint16_t ticks = angleToPwmTicks(_currentAngles[i], i);
            writePwm(i, 0, ticks);
        }
    }

    _isMoving = anyJointMoving;
}
