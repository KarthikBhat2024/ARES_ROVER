#pragma once
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClient.h>

// ==============================================================================
// AresArmGateway: Inter-Controller Bridge from ESP32 Rover to ESP8266 Arm
// ==============================================================================

class AresArmGateway {
public:
    AresArmGateway();

    void begin(const char* armBaseUrl = "http://192.168.4.20/api");

    // Telemetry & Status
    String getArmStatus();
    bool isArmConnected() const;
    uint32_t getLastSeenMs() const { return _lastSuccessfulContactMs; }

    // Arm Controls
    bool sendServoCommand(uint8_t servoId, float angleDeg, float speedDps = -1.0f);
    bool sendBatchServoCommand(const String& queryParams);
    bool sendHomeCommand();
    bool sendStopCommand();
    bool sendLimitsCommand(uint8_t servoId, int16_t minLimit, int16_t maxLimit);

    // Dynamic Arm IP configuration
    void setArmBaseUrl(const char* armBaseUrl);

private:
    String _baseUrl;
    uint32_t _lastSuccessfulContactMs;
    uint16_t _timeoutMs;
    bool _armOnline;

    String forwardGet(const String& endpoint);
    String forwardPost(const String& endpoint, const String& postData = "");
};

extern AresArmGateway ArmGateway;
