#pragma once
#include <Arduino.h>
#include <ESP8266WebServer.h>
#include "arm_config.h"
#include "pca9685_arm_controller.h"

// ==============================================================================
// AresArmWebServer: Standalone REST API for Upper Robotic Arm
// ==============================================================================

class AresArmWebServer {
public:
    AresArmWebServer(uint16_t port = ARM_HTTP_PORT);

    // Initialize HTTP endpoints
    void begin();

    // Call inside main loop() to service requests
    void update();

private:
    ESP8266WebServer _server;

    void registerEndpoints();
    void sendCorsHeaders();

    // Route Handlers
    void handleRoot();
    void handleStatus();
    void handleServo();
    void handleServosBatch();
    void handleHome();
    void handleStop();
    void handleLimits();
    void handleSpeed();
    void handlePower();
    void handleOptions();
    void handleNotFound();
};

extern AresArmWebServer ArmServer;
