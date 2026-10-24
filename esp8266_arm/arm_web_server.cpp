#include "arm_web_server.h"
#include <ESP8266WiFi.h>

AresArmWebServer ArmServer(ARM_HTTP_PORT);

AresArmWebServer::AresArmWebServer(uint16_t port)
    : _server(port)
{
}

void AresArmWebServer::begin() {
    registerEndpoints();
    _server.begin();
    Serial.printf("[ARM NET] Arm REST API Server active on port %d.\n", ARM_HTTP_PORT);
}

void AresArmWebServer::update() {
    _server.handleClient();
}

void AresArmWebServer::sendCorsHeaders() {
    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    _server.sendHeader("Access-Control-Allow-Headers", "Content-Type, X-Requested-With");
    _server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
}

void AresArmWebServer::handleOptions() {
    sendCorsHeaders();
    _server.send(204);
}

void AresArmWebServer::registerEndpoints() {
    // Info & Root
    _server.on("/", HTTP_GET, std::bind(&AresArmWebServer::handleRoot, this));
    _server.on("/api/status", HTTP_GET, std::bind(&AresArmWebServer::handleStatus, this));

    // Servo commands (accepts both GET and POST)
    _server.on("/api/servo", HTTP_GET, std::bind(&AresArmWebServer::handleServo, this));
    _server.on("/api/servo", HTTP_POST, std::bind(&AresArmWebServer::handleServo, this));

    // Batch all servos
    _server.on("/api/servos", HTTP_POST, std::bind(&AresArmWebServer::handleServosBatch, this));
    _server.on("/api/servos", HTTP_GET, std::bind(&AresArmWebServer::handleServosBatch, this));

    // Home & Stop
    _server.on("/api/home", HTTP_POST, std::bind(&AresArmWebServer::handleHome, this));
    _server.on("/api/home", HTTP_GET, std::bind(&AresArmWebServer::handleHome, this));
    _server.on("/api/stop", HTTP_POST, std::bind(&AresArmWebServer::handleStop, this));
    _server.on("/api/stop", HTTP_GET, std::bind(&AresArmWebServer::handleStop, this));

    // Runtime calibration & speed
    _server.on("/api/limits", HTTP_POST, std::bind(&AresArmWebServer::handleLimits, this));
    _server.on("/api/speed", HTTP_POST, std::bind(&AresArmWebServer::handleSpeed, this));
    _server.on("/api/power", HTTP_POST, std::bind(&AresArmWebServer::handlePower, this));

    // OPTIONS preflight for CORS
    _server.on("/api/status", HTTP_OPTIONS, std::bind(&AresArmWebServer::handleOptions, this));
    _server.on("/api/servo", HTTP_OPTIONS, std::bind(&AresArmWebServer::handleOptions, this));
    _server.on("/api/servos", HTTP_OPTIONS, std::bind(&AresArmWebServer::handleOptions, this));
    _server.on("/api/home", HTTP_OPTIONS, std::bind(&AresArmWebServer::handleOptions, this));
    _server.on("/api/stop", HTTP_OPTIONS, std::bind(&AresArmWebServer::handleOptions, this));
    _server.on("/api/limits", HTTP_OPTIONS, std::bind(&AresArmWebServer::handleOptions, this));

    _server.onNotFound(std::bind(&AresArmWebServer::handleNotFound, this));
}

void AresArmWebServer::handleRoot() {
    sendCorsHeaders();
    String html = F("<!DOCTYPE html><html><head><title>ARES Robotic Arm</title></head>"
                    "<body style='background:#070b14;color:#f8fafc;font-family:sans-serif;padding:24px;'>"
                    "<h2>ARES Robotic Arm ESP8266 Subsystem</h2>"
                    "<p>Status: <span style='color:#10b981;'>ONLINE</span></p>"
                    "<p>REST API: <a style='color:#00f0ff;' href='/api/status'>/api/status</a></p>"
                    "<p>Main Dashboard is hosted on ESP32 at: <a style='color:#00f0ff;' href='http://192.168.4.1'>http://192.168.4.1</a></p>"
                    "</body></html>");
    _server.send(200, "text/html", html);
}

void AresArmWebServer::handleStatus() {
    sendCorsHeaders();

    uint32_t uptime = millis() / 1000;
    int32_t rssi = WiFi.RSSI();

    // Construct detailed telemetry JSON
    String json = "{";
    json += "\"status\":\"ok\",";
    json += "\"device\":\"ARES_ARM_ESP8266\",";
    json += "\"uptime\":" + String(uptime) + ",";
    json += "\"moving\":" + String(ArmController.isMoving() ? "true" : "false") + ",";
    json += "\"frozen\":" + String(ArmController.isFrozen() ? "true" : "false") + ",";
    json += "\"pwm_enabled\":" + String(ArmController.isPwmEnabled() ? "true" : "false") + ",";
    json += "\"speed_dps\":" + String(ArmController.getGlobalSpeed(), 1) + ",";
    json += "\"rssi\":" + String(rssi) + ",";
    json += "\"feedback_type\":\"commanded_open_loop\",";
    json += "\"servos\":[";

    for (uint8_t i = 1; i <= TOTAL_ARM_SERVOS; i++) {
        json += "{";
        json += "\"id\":" + String(i) + ",";
        json += "\"name\":\"" + String(ArmController.getServoName(i)) + "\",";
        json += "\"model\":\"" + String(ArmController.getServoModel(i)) + "\",";
        json += "\"angle\":" + String(ArmController.getCommandedAngle(i), 1) + ",";
        json += "\"target\":" + String(ArmController.getTargetAngle(i), 1) + ",";
        json += "\"min\":" + String(ArmController.getMinAngle(i)) + ",";
        json += "\"max\":" + String(ArmController.getMaxAngle(i)) + ",";
        json += "\"home\":" + String(ArmController.getHomeAngle(i));
        json += "}";
        if (i < TOTAL_ARM_SERVOS) json += ",";
    }

    json += "]}";
    _server.send(200, "application/json", json);
}

void AresArmWebServer::handleServo() {
    sendCorsHeaders();

    if (!_server.hasArg("id") || !_server.hasArg("angle")) {
        _server.send(400, "application/json", "{\"error\":\"Missing 'id' (1-6) or 'angle' parameter\"}");
        return;
    }

    uint8_t id = (uint8_t)_server.arg("id").toInt();
    float angle = _server.arg("angle").toFloat();
    float spd = _server.hasArg("spd") ? _server.arg("spd").toFloat() : ArmController.getGlobalSpeed();

    if (id < 1 || id > TOTAL_ARM_SERVOS) {
        _server.send(400, "application/json", "{\"error\":\"Servo ID out of range (1 to 6)\"}");
        return;
    }

    bool success = ArmController.setTargetAngle(id, angle, spd);

    if (success) {
        char resp[128];
        snprintf(resp, sizeof(resp), 
                 "{\"status\":\"ok\",\"servo\":%d,\"target\":%.1f,\"speed\":%.1f}",
                 id, ArmController.getTargetAngle(id), spd);
        _server.send(200, "application/json", resp);
    } else {
        _server.send(400, "application/json", "{\"error\":\"Failed to command servo\"}");
    }
}

void AresArmWebServer::handleServosBatch() {
    sendCorsHeaders();

    float targets[TOTAL_ARM_SERVOS];
    for (uint8_t i = 0; i < TOTAL_ARM_SERVOS; i++) {
        String argName = "s" + String(i + 1);
        if (_server.hasArg(argName)) {
            targets[i] = _server.arg(argName).toFloat();
        } else {
            targets[i] = ArmController.getTargetAngle(i + 1);
        }
    }

    float spd = _server.hasArg("spd") ? _server.arg("spd").toFloat() : ArmController.getGlobalSpeed();
    ArmController.setAllTargetAngles(targets, spd);

    _server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Batch target angles applied\"}");
}

void AresArmWebServer::handleHome() {
    sendCorsHeaders();
    float spd = _server.hasArg("spd") ? _server.arg("spd").toFloat() : ArmController.getGlobalSpeed();
    ArmController.homeAll(spd);
    _server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Arm returning to HOME\"}");
}

void AresArmWebServer::handleStop() {
    sendCorsHeaders();
    ArmController.freeze();
    _server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Arm motion frozen\"}");
}

void AresArmWebServer::handleLimits() {
    sendCorsHeaders();

    if (!_server.hasArg("id") || !_server.hasArg("min") || !_server.hasArg("max")) {
        _server.send(400, "application/json", "{\"error\":\"Missing 'id', 'min', or 'max' parameter\"}");
        return;
    }

    uint8_t id = (uint8_t)_server.arg("id").toInt();
    int16_t minVal = (int16_t)_server.arg("min").toInt();
    int16_t maxVal = (int16_t)_server.arg("max").toInt();

    bool success = ArmController.setServoLimits(id, minVal, maxVal);

    if (success) {
        _server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Limits updated\"}");
    } else {
        _server.send(400, "application/json", "{\"error\":\"Invalid limit values\"}");
    }
}

void AresArmWebServer::handleSpeed() {
    sendCorsHeaders();

    if (!_server.hasArg("val")) {
        _server.send(400, "application/json", "{\"error\":\"Missing 'val' speed parameter\"}");
        return;
    }

    float spd = _server.arg("val").toFloat();
    ArmController.setGlobalSpeed(spd);

    char resp[64];
    snprintf(resp, sizeof(resp), "{\"status\":\"ok\",\"speed_dps\":%.1f}", ArmController.getGlobalSpeed());
    _server.send(200, "application/json", resp);
}

void AresArmWebServer::handlePower() {
    sendCorsHeaders();

    if (!_server.hasArg("state")) {
        _server.send(400, "application/json", "{\"error\":\"Missing 'state' parameter (0 or 1)\"}");
        return;
    }

    bool state = (_server.arg("state") == "1");
    if (state) {
        ArmController.enableAllOutputs();
    } else {
        ArmController.disableAllOutputs();
    }

    _server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void AresArmWebServer::handleNotFound() {
    sendCorsHeaders();
    _server.send(404, "application/json", "{\"error\":\"Endpoint not found\"}");
}
