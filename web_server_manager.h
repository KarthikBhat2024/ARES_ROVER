#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include "config.h"
#include "motor_controller.h"

// ==============================================================================
// AresWebManager: High-Performance Local Wi-Fi & Web Teleoperation Engine
// ==============================================================================

class AresWebManager {
public:
    AresWebManager();

    // Initialize Access Point, DNS Captive Portal, and HTTP REST Endpoints
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
    void handleRoot();
    void handleCommand();
    void handleSpeed();
    void handleStatus();
    void handleAux();
    void handleNotFound();
};

extern AresWebManager WebManager;
