#include "web_server_manager.h"
#include "dashboard_html.h"

AresWebManager WebManager;

AresWebManager::AresWebManager()
    : _server(80),
      _webSpeed(SPEED_DEFAULT),
      _lightOn(false),
      _buzzerOn(false),
      _bootTimeSeconds(0)
{
}

void AresWebManager::begin() {
    Serial.println(F("\n[NET] Starting ARES Wi-Fi Access Point & Teleoperation Server..."));

    // 1. Initialize Wi-Fi Access Point
    WiFi.mode(WIFI_AP_STA); // AP for dashboard, STA for local subnet bridges
    bool apStarted = WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, WIFI_AP_CHANNEL);
    if (apStarted) {
        Serial.printf("[NET] SoftAP Started. SSID: %s | Pass: %s\n", WIFI_AP_SSID, WIFI_AP_PASSWORD);
        Serial.printf("[NET] AP IP Address: %s\n", WiFi.softAPIP().toString().c_str());
    } else {
        Serial.println(F("[FATAL] Failed to start ESP32 SoftAP!"));
    }

    // 2. Start Captive Portal DNS Server (Redirects all DNS queries to Rover IP)
    #if ENABLE_CAPTIVE_PORTAL
    _dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    _dnsServer.start(53, "*", WiFi.softAPIP());
    Serial.println(F("[NET] Captive Portal DNS Server active on port 53."));
    #endif

    // 3. Start mDNS (Enables typing http://ares.local in browser)
    if (MDNS.begin(MDNS_HOSTNAME)) {
        Serial.printf("[NET] mDNS responder started: http://%s.local\n", MDNS_HOSTNAME);
        MDNS.addService("http", "tcp", 80);
    }

    // 4. Initialize Auxiliary Hardware Pins if assigned
    if (PIN_HEADLIGHTS != -1) {
        pinMode(PIN_HEADLIGHTS, OUTPUT);
        digitalWrite(PIN_HEADLIGHTS, LOW);
        Serial.printf("[AUX] Headlights hardware initialized on GPIO %d\n", PIN_HEADLIGHTS);
    } else {
        Serial.println(F("[AUX] Headlights hardware unassigned (Running in simulated mode)."));
    }

    if (PIN_RESCUE_BEACON != -1) {
        pinMode(PIN_RESCUE_BEACON, OUTPUT);
        digitalWrite(PIN_RESCUE_BEACON, LOW);
        Serial.printf("[AUX] Rescue Beacon hardware initialized on GPIO %d\n", PIN_RESCUE_BEACON);
    } else {
        Serial.println(F("[AUX] Rescue Beacon hardware unassigned (Running in simulated mode)."));
    }

    // 5. Initialize Arm Gateway bridge
    ArmGateway.begin("http://192.168.4.20/api");

    // 6. Register HTTP Endpoints & Start Server
    registerHandlers();
    _server.begin();
    Serial.println(F("[NET] Integrated Web Teleoperation Server started on port 80.\n"));
}

void AresWebManager::update() {
    #if ENABLE_CAPTIVE_PORTAL
    _dnsServer.processNextRequest();
    #endif
    _server.handleClient();
}

void AresWebManager::sendCorsHeaders() {
    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    _server.sendHeader("Access-Control-Allow-Headers", "Content-Type, X-Requested-With");
    _server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
}

void AresWebManager::handleOptions() {
    sendCorsHeaders();
    _server.send(204);
}

void AresWebManager::registerHandlers() {
    // -------------------------------------------------------------------------
    // 1. Web Dashboard Root & Captive Portal Probes
    // -------------------------------------------------------------------------
    _server.on("/", HTTP_GET, std::bind(&AresWebManager::handleRoot, this));
    _server.on("/generate_204", HTTP_GET, std::bind(&AresWebManager::handleRoot, this));
    _server.on("/hotspot-detect.html", HTTP_GET, std::bind(&AresWebManager::handleRoot, this));
    _server.on("/ncsi.txt", HTTP_GET, std::bind(&AresWebManager::handleRoot, this));

    // -------------------------------------------------------------------------
    // 2. Unified Rover REST API Routes (Specification Section 9)
    // -------------------------------------------------------------------------
    _server.on("/api/status", HTTP_GET, std::bind(&AresWebManager::handleApiStatus, this));
    _server.on("/api/drive", HTTP_POST, std::bind(&AresWebManager::handleApiDrive, this));
    _server.on("/api/drive", HTTP_GET, std::bind(&AresWebManager::handleApiDrive, this));
    _server.on("/api/stop", HTTP_POST, std::bind(&AresWebManager::handleApiStop, this));
    _server.on("/api/stop", HTTP_GET, std::bind(&AresWebManager::handleApiStop, this));
    _server.on("/api/emergency_stop", HTTP_POST, std::bind(&AresWebManager::handleApiEmergencyStop, this));
    _server.on("/api/emergency_stop", HTTP_GET, std::bind(&AresWebManager::handleApiEmergencyStop, this));
    _server.on("/api/speed", HTTP_POST, std::bind(&AresWebManager::handleApiSpeed, this));
    _server.on("/api/speed", HTTP_GET, std::bind(&AresWebManager::handleApiSpeed, this));
    _server.on("/api/aux", HTTP_POST, std::bind(&AresWebManager::handleApiAux, this));
    _server.on("/api/aux", HTTP_GET, std::bind(&AresWebManager::handleApiAux, this));

    // -------------------------------------------------------------------------
    // 3. Robotic Arm Proxy API Routes (Forwards to ESP8266 Subsystem)
    // -------------------------------------------------------------------------
    _server.on("/api/arm/status", HTTP_GET, std::bind(&AresWebManager::handleApiArmStatus, this));
    _server.on("/api/arm/servo", HTTP_POST, std::bind(&AresWebManager::handleApiArmServo, this));
    _server.on("/api/arm/servo", HTTP_GET, std::bind(&AresWebManager::handleApiArmServo, this));
    _server.on("/api/arm/servos", HTTP_POST, std::bind(&AresWebManager::handleApiArmServosBatch, this));
    _server.on("/api/arm/home", HTTP_POST, std::bind(&AresWebManager::handleApiArmHome, this));
    _server.on("/api/arm/home", HTTP_GET, std::bind(&AresWebManager::handleApiArmHome, this));
    _server.on("/api/arm/stop", HTTP_POST, std::bind(&AresWebManager::handleApiArmStop, this));
    _server.on("/api/arm/stop", HTTP_GET, std::bind(&AresWebManager::handleApiArmStop, this));
    _server.on("/api/arm/limits", HTTP_POST, std::bind(&AresWebManager::handleApiArmLimits, this));

    // OPTIONS preflight for browser CORS
    _server.on("/api/status", HTTP_OPTIONS, std::bind(&AresWebManager::handleOptions, this));
    _server.on("/api/drive", HTTP_OPTIONS, std::bind(&AresWebManager::handleOptions, this));
    _server.on("/api/stop", HTTP_OPTIONS, std::bind(&AresWebManager::handleOptions, this));
    _server.on("/api/emergency_stop", HTTP_OPTIONS, std::bind(&AresWebManager::handleOptions, this));
    _server.on("/api/arm/status", HTTP_OPTIONS, std::bind(&AresWebManager::handleOptions, this));
    _server.on("/api/arm/servo", HTTP_OPTIONS, std::bind(&AresWebManager::handleOptions, this));
    _server.on("/api/arm/home", HTTP_OPTIONS, std::bind(&AresWebManager::handleOptions, this));
    _server.on("/api/arm/stop", HTTP_OPTIONS, std::bind(&AresWebManager::handleOptions, this));

    // -------------------------------------------------------------------------
    // 4. Backward Compatible Legacy Routes
    // -------------------------------------------------------------------------
    _server.on("/cmd", HTTP_GET, std::bind(&AresWebManager::handleLegacyCmd, this));
    _server.on("/speed", HTTP_GET, std::bind(&AresWebManager::handleLegacySpeed, this));
    _server.on("/status", HTTP_GET, std::bind(&AresWebManager::handleLegacyStatus, this));
    _server.on("/aux", HTTP_GET, std::bind(&AresWebManager::handleLegacyAux, this));

    // 404 Fallback / Captive Portal redirect
    _server.onNotFound(std::bind(&AresWebManager::handleNotFound, this));
}

void AresWebManager::handleRoot() {
    _server.sendHeader("Cache-Control", "max-age=3600");
    _server.send_P(200, "text/html", INDEX_HTML);
}

void AresWebManager::executeDriveCommand(const String& cmd, int speedOverride) {
    uint8_t effectiveSpeed = (speedOverride >= SPEED_MIN && speedOverride <= SPEED_MAX) 
                             ? (uint8_t)speedOverride : _webSpeed;

    if (cmd == "w" || cmd == "fwd" || cmd == "forward") {
        Rover.forward(effectiveSpeed);
    } else if (cmd == "s" || cmd == "rev" || cmd == "reverse" || cmd == "back") {
        Rover.reverse(effectiveSpeed);
    } else if (cmd == "a" || cmd == "rot_left" || cmd == "rotate_left" || cmd == "spin_left") {
        Rover.rotateLeft(effectiveSpeed);
    } else if (cmd == "d" || cmd == "rot_right" || cmd == "rotate_right" || cmd == "spin_right") {
        Rover.rotateRight(effectiveSpeed);
    } else if (cmd == "q" || cmd == "left" || cmd == "turn_left" || cmd == "pivot_left") {
        Rover.turnLeft(effectiveSpeed);
    } else if (cmd == "e" || cmd == "right" || cmd == "turn_right" || cmd == "pivot_right") {
        Rover.turnRight(effectiveSpeed);
    } else if (cmd == "b" || cmd == "brake") {
        Rover.brake();
    } else if (cmd == "t" || cmd == "test") {
        Rover.runSelfTestSequence();
    } else if (cmd == "x" || cmd == "stop") {
        Rover.stop();
    } else {
        Rover.stop();
    }
}

void AresWebManager::handleApiDrive() {
    sendCorsHeaders();

    String cmd = "";
    if (_server.hasArg("cmd")) cmd = _server.arg("cmd");
    else if (_server.hasArg("c")) cmd = _server.arg("c");

    if (cmd.length() == 0) {
        _server.send(400, "application/json", "{\"error\":\"Missing 'cmd' parameter\"}");
        return;
    }

    cmd.toLowerCase();
    cmd.trim();

    int speedOverride = -1;
    if (_server.hasArg("spd")) speedOverride = _server.arg("spd").toInt();
    else if (_server.hasArg("speed")) speedOverride = _server.arg("speed").toInt();

    if (speedOverride >= SPEED_MIN && speedOverride <= SPEED_MAX) {
        _webSpeed = (uint8_t)speedOverride;
    }

    executeDriveCommand(cmd, speedOverride);

    char resp[128];
    snprintf(resp, sizeof(resp), "{\"status\":\"ok\",\"state\":\"%s\",\"speed\":%d}", 
             Rover.getMotionStateString(), _webSpeed);
    _server.send(200, "application/json", resp);
}

void AresWebManager::handleApiStop() {
    sendCorsHeaders();
    Rover.stop();
    char resp[128];
    snprintf(resp, sizeof(resp), "{\"status\":\"ok\",\"state\":\"%s\",\"speed\":%d}", 
             Rover.getMotionStateString(), _webSpeed);
    _server.send(200, "application/json", resp);
}

void AresWebManager::handleApiEmergencyStop() {
    sendCorsHeaders();
    // 1. Immediately halt Rover drive motors
    Rover.stop();

    // 2. Request Arm Controller to freeze and cancel trajectory
    bool armNotified = ArmGateway.sendStopCommand();

    Serial.println(F("[SAFETY] MASTER EMERGENCY STOP EXECUTED (Rover Motors + Arm)"));

    char resp[192];
    snprintf(resp, sizeof(resp), 
             "{\"status\":\"ok\",\"action\":\"EMERGENCY_STOP\",\"rover_state\":\"STOPPED\","
             "\"arm_stop_sent\":%s,\"arm_connected\":%s}",
             armNotified ? "true" : "false",
             ArmGateway.isArmConnected() ? "true" : "false");
    _server.send(200, "application/json", resp);
}

void AresWebManager::handleApiSpeed() {
    sendCorsHeaders();
    if (_server.hasArg("val") || _server.hasArg("speed")) {
        int val = _server.hasArg("val") ? _server.arg("val").toInt() : _server.arg("speed").toInt();
        _webSpeed = (uint8_t)constrain(val, SPEED_MIN, SPEED_MAX);

        RoverMotionState state = Rover.getMotionState();
        if (state == STATE_FORWARD) Rover.forward(_webSpeed);
        else if (state == STATE_REVERSE) Rover.reverse(_webSpeed);
        else if (state == STATE_TURN_LEFT) Rover.turnLeft(_webSpeed);
        else if (state == STATE_TURN_RIGHT) Rover.turnRight(_webSpeed);
        else if (state == STATE_ROTATE_LEFT) Rover.rotateLeft(_webSpeed);
        else if (state == STATE_ROTATE_RIGHT) Rover.rotateRight(_webSpeed);
    }

    char resp[64];
    snprintf(resp, sizeof(resp), "{\"status\":\"ok\",\"speed\":%d}", _webSpeed);
    _server.send(200, "application/json", resp);
}

void AresWebManager::handleApiStatus() {
    sendCorsHeaders();

    uint32_t uptimeSec = millis() / 1000;
    float batteryVoltage = 11.8f; // Placeholder until Stage 4 ADC sensor is attached
    bool armOnline = ArmGateway.isArmConnected();

    char json[384];
    snprintf(json, sizeof(json),
             "{\"status\":\"ok\",\"device\":\"ARES_ROVER_ESP32\","
             "\"state\":\"%s\",\"speed\":%d,\"watchdog\":%s,\"uptime\":%u,"
             "\"batt\":%.1f,\"batt_sim\":true,"
             "\"light\":%s,\"beacon\":%s,"
             "\"arm_online\":%s,\"arm_last_seen_ms\":%u}",
             Rover.getMotionStateString(),
             _webSpeed,
             Rover.isWatchdogTriggered() ? "false" : "true",
             uptimeSec,
             batteryVoltage,
             _lightOn ? "true" : "false",
             _buzzerOn ? "true" : "false",
             armOnline ? "true" : "false",
             armOnline ? (millis() - ArmGateway.getLastSeenMs()) : 99999);

    _server.send(200, "application/json", json);
}

void AresWebManager::handleApiAux() {
    sendCorsHeaders();
    if (_server.hasArg("item")) {
        String item = _server.arg("item");
        item.toLowerCase();
        bool state = (_server.arg("state") == "1" || _server.arg("state") == "true");

        if (item == "light" || item == "lights") {
            _lightOn = state;
            if (PIN_HEADLIGHTS != -1) digitalWrite(PIN_HEADLIGHTS, _lightOn ? HIGH : LOW);
        } else if (item == "beacon" || item == "siren" || item == "buzzer") {
            _buzzerOn = state;
            if (PIN_RESCUE_BEACON != -1) digitalWrite(PIN_RESCUE_BEACON, _buzzerOn ? HIGH : LOW);
        }
    }
    _server.send(200, "application/json", "{\"status\":\"ok\"}");
}

// -----------------------------------------------------------------------------
// Arm Proxy Implementations
// -----------------------------------------------------------------------------
void AresWebManager::handleApiArmStatus() {
    sendCorsHeaders();
    String armJson = ArmGateway.getArmStatus();
    _server.send(200, "application/json", armJson);
}

void AresWebManager::handleApiArmServo() {
    sendCorsHeaders();
    if (!_server.hasArg("id") || !_server.hasArg("angle")) {
        _server.send(400, "application/json", "{\"error\":\"Missing 'id' or 'angle' parameter\"}");
        return;
    }

    uint8_t id = (uint8_t)_server.arg("id").toInt();
    float angle = _server.arg("angle").toFloat();
    float spd = _server.hasArg("spd") ? _server.arg("spd").toFloat() : -1.0f;

    bool sent = ArmGateway.sendServoCommand(id, angle, spd);
    if (sent) {
        char resp[96];
        snprintf(resp, sizeof(resp), "{\"status\":\"ok\",\"servo\":%d,\"target\":%.1f}", id, angle);
        _server.send(200, "application/json", resp);
    } else {
        _server.send(503, "application/json", "{\"error\":\"ESP8266 Arm Controller offline or unreachable\"}");
    }
}

void AresWebManager::handleApiArmServosBatch() {
    sendCorsHeaders();
    // Build query string from all args
    String query = "";
    for (int i = 0; i < _server.args(); i++) {
        if (i > 0) query += "&";
        query += _server.argName(i) + "=" + _server.arg(i);
    }
    bool sent = ArmGateway.sendBatchServoCommand(query);
    if (sent) {
        _server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Batch sent to arm\"}");
    } else {
        _server.send(503, "application/json", "{\"error\":\"ESP8266 Arm Controller offline or unreachable\"}");
    }
}

void AresWebManager::handleApiArmHome() {
    sendCorsHeaders();
    bool sent = ArmGateway.sendHomeCommand();
    if (sent) {
        _server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Arm home command dispatched\"}");
    } else {
        _server.send(503, "application/json", "{\"error\":\"ESP8266 Arm Controller offline or unreachable\"}");
    }
}

void AresWebManager::handleApiArmStop() {
    sendCorsHeaders();
    bool sent = ArmGateway.sendStopCommand();
    if (sent) {
        _server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Arm stop command dispatched\"}");
    } else {
        _server.send(503, "application/json", "{\"error\":\"ESP8266 Arm Controller offline or unreachable\"}");
    }
}

void AresWebManager::handleApiArmLimits() {
    sendCorsHeaders();
    if (!_server.hasArg("id") || !_server.hasArg("min") || !_server.hasArg("max")) {
        _server.send(400, "application/json", "{\"error\":\"Missing 'id', 'min', or 'max' parameter\"}");
        return;
    }
    uint8_t id = (uint8_t)_server.arg("id").toInt();
    int16_t minVal = (int16_t)_server.arg("min").toInt();
    int16_t maxVal = (int16_t)_server.arg("max").toInt();

    bool sent = ArmGateway.sendLimitsCommand(id, minVal, maxVal);
    if (sent) {
        _server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Arm limits updated\"}");
    } else {
        _server.send(503, "application/json", "{\"error\":\"ESP8266 Arm Controller offline or unreachable\"}");
    }
}

// -----------------------------------------------------------------------------
// Legacy Route Adapters
// -----------------------------------------------------------------------------
void AresWebManager::handleLegacyCmd() {
    handleApiDrive();
}

void AresWebManager::handleLegacySpeed() {
    handleApiSpeed();
}

void AresWebManager::handleLegacyStatus() {
    handleApiStatus();
}

void AresWebManager::handleLegacyAux() {
    handleApiAux();
}

void AresWebManager::handleNotFound() {
    _server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
    _server.send(302, "text/plain", "");
}
