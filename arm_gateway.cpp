#include "arm_gateway.h"

AresArmGateway ArmGateway;

AresArmGateway::AresArmGateway()
    : _baseUrl("http://192.168.4.20/api"),
      _lastSuccessfulContactMs(0),
      _timeoutMs(450), // Fast timeout prevents blocking the Rover motion loop
      _armOnline(false)
{
}

void AresArmGateway::begin(const char* armBaseUrl) {
    if (armBaseUrl && strlen(armBaseUrl) > 0) {
        _baseUrl = String(armBaseUrl);
    }
    Serial.printf("[GATEWAY] Inter-controller arm bridge initialized target: %s\n", _baseUrl.c_str());
}

void AresArmGateway::setArmBaseUrl(const char* armBaseUrl) {
    if (armBaseUrl && strlen(armBaseUrl) > 0) {
        _baseUrl = String(armBaseUrl);
    }
}

bool AresArmGateway::isArmConnected() const {
    if (!_armOnline) return false;
    return (millis() - _lastSuccessfulContactMs < 4000); // 4 seconds liveness threshold
}

String AresArmGateway::forwardGet(const String& endpoint) {
    WiFiClient client;
    HTTPClient http;
    http.setTimeout(_timeoutMs);

    String fullUrl = _baseUrl + endpoint;
    if (!http.begin(client, fullUrl)) {
        _armOnline = false;
        return "{\"status\":\"error\",\"connected\":false,\"error\":\"HTTP client init failed\"}";
    }

    int httpCode = http.GET();
    String payload = "";

    if (httpCode == HTTP_CODE_OK) {
        payload = http.getString();
        _armOnline = true;
        _lastSuccessfulContactMs = millis();
    } else {
        _armOnline = false;
        payload = "{\"status\":\"error\",\"connected\":false,\"error\":\"ESP8266 Arm Unreachable\",\"http_code\":" + String(httpCode) + "}";
    }

    http.end();
    return payload;
}

String AresArmGateway::forwardPost(const String& endpoint, const String& postData) {
    WiFiClient client;
    HTTPClient http;
    http.setTimeout(_timeoutMs);

    String fullUrl = _baseUrl + endpoint;
    if (!http.begin(client, fullUrl)) {
        _armOnline = false;
        return "{\"status\":\"error\",\"connected\":false,\"error\":\"HTTP client init failed\"}";
    }

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpCode = http.POST(postData);
    String payload = "";

    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_NO_CONTENT) {
        payload = http.getString();
        _armOnline = true;
        _lastSuccessfulContactMs = millis();
    } else {
        _armOnline = false;
        payload = "{\"status\":\"error\",\"connected\":false,\"error\":\"ESP8266 Arm Unreachable\",\"http_code\":" + String(httpCode) + "}";
    }

    http.end();
    return payload;
}

String AresArmGateway::getArmStatus() {
    return forwardGet("/status");
}

bool AresArmGateway::sendServoCommand(uint8_t servoId, float angleDeg, float speedDps) {
    String endpoint = "/servo?id=" + String(servoId) + "&angle=" + String(angleDeg, 1);
    if (speedDps > 0) {
        endpoint += "&spd=" + String(speedDps, 1);
    }
    String resp = forwardPost(endpoint, "");
    return _armOnline;
}

bool AresArmGateway::sendBatchServoCommand(const String& queryParams) {
    String endpoint = "/servos";
    if (queryParams.length() > 0) {
        endpoint += (queryParams.startsWith("?") ? queryParams : ("?" + queryParams));
    }
    String resp = forwardPost(endpoint, "");
    return _armOnline;
}

bool AresArmGateway::sendHomeCommand() {
    String resp = forwardPost("/home", "");
    return _armOnline;
}

bool AresArmGateway::sendStopCommand() {
    String resp = forwardPost("/stop", "");
    return _armOnline;
}

bool AresArmGateway::sendLimitsCommand(uint8_t servoId, int16_t minLimit, int16_t maxLimit) {
    String endpoint = "/limits?id=" + String(servoId) + "&min=" + String(minLimit) + "&max=" + String(maxLimit);
    String resp = forwardPost(endpoint, "");
    return _armOnline;
}
