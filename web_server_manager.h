#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include "config.h"
#include "motor_controller.h"
#include "arm_gateway.h"

// ==============================================================================
// AresWebManager: High-Performance Integrated Teleoperation Engine
// Hosts unified dashboard, rover drive API, and ESP8266 arm proxy bridge
// ==============================================================================

class AresWebManager {
public:
    AresWebManager();

    // Initialize Access Point, DNS Captive Portal, and HTTP Endpoints
    void begin();

    // Call inside main loop() to service HTTP and DNS requests
    void update();

    // Auxiliary status setters/getters
    bool isLightOn() const { return _lightOn; }
    bool isBuzzerOn() const { return _buzzerOn; }
    uint8_t getWebSpeed() const { return _webSpeed; }

private:
    WebServer _server;
    DNSServer _dnsServer;
    uint8_t _webSpeed;
    bool _lightOn;
    bool _buzzerOn;
    uint32_t _bootTimeSeconds;

    void registerHandlers();
    void sendCorsHeaders();

    // Web Root & Captive Portal
    void handleRoot();
    void handleNotFound();
    void handleOptions();

    // Unified REST APIs
    void handleApiStatus();
    void handleApiDrive();
    void handleApiStop();
    void handleApiEmergencyStop();
    void handleApiSpeed();
    void handleApiAux();

    // Arm Proxy APIs (Forwards to ESP8266)
    void handleApiArmStatus();
    void handleApiArmServo();
    void handleApiArmServosBatch();
    void handleApiArmHome();
    void handleApiArmStop();
    void handleApiArmLimits();

    // Legacy Route Adapters
    void handleLegacyCmd();
    void handleLegacySpeed();
    void handleLegacyStatus();
    void handleLegacyAux();

    // Motion execution helper
    void executeDriveCommand(const String& cmd, int speedOverride = -1);
};

extern AresWebManager WebManager;
