#include "web_server_manager.h"
#include "dashboard_html.h"

// Global instance
AresWebManager WebManager;

AresWebManager::AresWebManager() 
    : _server(80), _webSpeed(SPEED_DEFAULT), _lightOn(false), _buzzerOn(false), _bootTimeSeconds(0) {}

void AresWebManager::begin() {
    Serial.println(F("\n[NET] Initializing Local ARES Wi-Fi Access Point..."));

    // 1. Configure Standalone Wi-Fi Access Point (No external router needed)
    WiFi.mode(WIFI_AP);
    
    IPAddress local_IP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    
    WiFi.softAPConfig(local_IP, gateway, subnet);
    bool apStarted = WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD, WIFI_AP_CHANNEL);

    if (apStarted) {
        Serial.printf("[NET] Wi-Fi AP Online! SSID: %s\n", WIFI_AP_SSID);
        Serial.printf("[NET] Password:      %s\n", WIFI_AP_PASSWORD);
        Serial.printf("[NET] Rover Web URL: http://%s\n", WiFi.softAPIP().toString().c_str());
    } else {
        Serial.println(F("[NET ERROR] Failed to start SoftAP!"));
    }

    // 2. Start DNS Captive Portal (Redirects any phone browser to 192.168.4.1)
    #if ENABLE_CAPTIVE_PORTAL
    _dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    _dnsServer.start(53, "*", local_IP);
    Serial.println(F("[NET] DNS Captive Portal active."));
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

    // 5. Register HTTP Web Server Endpoints
    registerHandlers();
    _server.begin();
    Serial.println(F("[NET] HTTP Web Teleoperation Server started on port 80.\n"));
}

void AresWebManager::registerHandlers() {
    // Serve Main High-Tech Dashboard
    _server.on("/", HTTP_GET, std::bind(&AresWebManager::handleRoot, this));

    // Real-time Motion Commands (/cmd?c=w&spd=180)
    _server.on("/cmd", HTTP_GET, std::bind(&AresWebManager::handleCommand, this));

    // Speed Slider Update (/speed?val=200)
    _server.on("/speed", HTTP_GET, std::bind(&AresWebManager::handleSpeed, this));

    // Live Telemetry Stream (/status)
    _server.on("/status", HTTP_GET, std::bind(&AresWebManager::handleStatus, this));

    // Auxiliary Systems (/aux?item=light&state=1)
    _server.on("/aux", HTTP_GET, std::bind(&AresWebManager::handleAux, this));

    // Captive Portal probes from Android / iOS / Windows
    _server.on("/generate_204", HTTP_GET, std::bind(&AresWebManager::handleRoot, this));
    _server.on("/hotspot-detect.html", HTTP_GET, std::bind(&AresWebManager::handleRoot, this));
    _server.on("/ncsi.txt", HTTP_GET, std::bind(&AresWebManager::handleRoot, this));

    // Fallback handler for all other paths
    _server.onNotFound(std::bind(&AresWebManager::handleNotFound, this));
}

void AresWebManager::update() {
    #if ENABLE_CAPTIVE_PORTAL
    _dnsServer.processNextRequest();
    #endif
    _server.handleClient();
}

void AresWebManager::handleRoot() {
    _server.sendHeader("Cache-Control", "max-age=86400"); // Cache static page on client
    _server.send_P(200, "text/html", INDEX_HTML);
}

void AresWebManager::handleCommand() {
    if (!_server.hasArg("c")) {
        _server.send(400, "application/json", "{\"error\":\"Missing cmd 'c' parameter\"}");
        return;
    }

    String cmd = _server.arg("c");
    cmd.toLowerCase();
    cmd.trim();

    // Optional speed parameter
    if (_server.hasArg("spd")) {
        _webSpeed = (uint8_t)constrain(_server.arg("spd").toInt(), SPEED_MIN, SPEED_MAX);
    }

    // Execute Rover Kinematics (Handles both single-char and descriptive string commands)
    if (cmd == "w" || cmd == "fwd" || cmd == "forward") {
        Rover.forward(_webSpeed);
    } else if (cmd == "s" || cmd == "rev" || cmd == "reverse" || cmd == "back") {
        Rover.reverse(_webSpeed);
    } else if (cmd == "a" || cmd == "rot_left" || cmd == "rotate_left" || cmd == "spin_left") {
        Rover.rotateLeft(_webSpeed);
    } else if (cmd == "d" || cmd == "rot_right" || cmd == "rotate_right" || cmd == "spin_right") {
        Rover.rotateRight(_webSpeed);
    } else if (cmd == "q" || cmd == "left" || cmd == "turn_left" || cmd == "pivot_left") {
        Rover.turnLeft(_webSpeed);
    } else if (cmd == "e" || cmd == "right" || cmd == "turn_right" || cmd == "pivot_right") {
        Rover.turnRight(_webSpeed);
    } else if (cmd == "b" || cmd == "brake") {
        Rover.brake();
    } else if (cmd == "t" || cmd == "test") {
        Rover.runSelfTestSequence();
    } else if (cmd == "x" || cmd == "stop") {
        Rover.stop();
    } else {
        Rover.stop();
    }

    // Return instant status JSON
    char resp[128];
    snprintf(resp, sizeof(resp), "{\"status\":\"ok\",\"state\":\"%s\",\"speed\":%d}", 
             Rover.getMotionStateString(), _webSpeed);
    
    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.send(200, "application/json", resp);
}

void AresWebManager::handleSpeed() {
    if (_server.hasArg("val")) {
        _webSpeed = (uint8_t)constrain(_server.arg("val").toInt(), SPEED_MIN, SPEED_MAX);
        
        // If rover is actively moving, update current movement speed immediately
        RoverMotionState state = Rover.getMotionState();
        if (state == STATE_FORWARD) Rover.forward(_webSpeed);
        else if (state == STATE_REVERSE) Rover.reverse(_webSpeed);
        else if (state == STATE_TURN_LEFT) Rover.turnLeft(_webSpeed);
        else if (state == STATE_TURN_RIGHT) Rover.turnRight(_webSpeed);
        else if (state == STATE_ROTATE_LEFT) Rover.rotateLeft(_webSpeed);
        else if (state == STATE_ROTATE_RIGHT) Rover.rotateRight(_webSpeed);
    }

    char resp[64];
    snprintf(resp, sizeof(resp), "{\"speed\":%d}", _webSpeed);
    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.send(200, "application/json", resp);
}

void AresWebManager::handleStatus() {
    uint32_t uptimeSec = millis() / 1000;
    float batteryVoltage = 11.8f; // SIMULATED placeholder until Stage 4 ADC circuit is connected

    char json[320];
    snprintf(json, sizeof(json),
             "{\"state\":\"%s\",\"speed\":%d,\"watchdog\":%s,\"uptime\":%u,"
             "\"batt\":%.1f,\"batt_sim\":true,"
             "\"light\":%s,\"light_hw\":%s,"
             "\"beacon\":%s,\"beacon_hw\":%s}",
             Rover.getMotionStateString(),
             _webSpeed,
             Rover.isWatchdogTriggered() ? "false" : "true",
             uptimeSec,
             batteryVoltage,
             _lightOn ? "true" : "false",
             (PIN_HEADLIGHTS != -1) ? "true" : "false",
             _buzzerOn ? "true" : "false",
             (PIN_RESCUE_BEACON != -1) ? "true" : "false");

    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    _server.send(200, "application/json", json);
}

void AresWebManager::handleAux() {
    if (_server.hasArg("item")) {
        String item = _server.arg("item");
        item.toLowerCase();
        bool state = (_server.arg("state") == "1");

        if (item == "light" || item == "lights") {
            _lightOn = state;
            if (PIN_HEADLIGHTS != -1) {
                digitalWrite(PIN_HEADLIGHTS, _lightOn ? HIGH : LOW);
                Serial.printf("[AUX HARDWARE] Headlight set to %s on GPIO %d\n", _lightOn ? "ON" : "OFF", PIN_HEADLIGHTS);
            } else {
                Serial.printf("[AUX SIMULATED] Headlight state set to %s (Hardware pin unassigned)\n", _lightOn ? "ON" : "OFF");
            }
        } else if (item == "beacon" || item == "siren" || item == "buzzer") {
            _buzzerOn = state;
            if (PIN_RESCUE_BEACON != -1) {
                digitalWrite(PIN_RESCUE_BEACON, _buzzerOn ? HIGH : LOW);
                Serial.printf("[AUX HARDWARE] Rescue Beacon set to %s on GPIO %d\n", _buzzerOn ? "ON" : "OFF", PIN_RESCUE_BEACON);
            } else {
                Serial.printf("[AUX SIMULATED] Rescue Beacon state set to %s (Hardware pin unassigned)\n", _buzzerOn ? "ON" : "OFF");
            }
        }
    }

    _server.sendHeader("Access-Control-Allow-Origin", "*");
    _server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void AresWebManager::handleNotFound() {
    // If unknown URL requested on Captive Portal, redirect straight to Root
    _server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
    _server.send(302, "text/plain", "");
}
