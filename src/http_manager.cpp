#include "http_manager.h"
#include "watchdog_manager.h"
#include <ArduinoJson.h>
#include "global_variables.h"
#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "tcp_manager.h"

WebServer httpServer(80);

void startHTTPServer() {
    Log.notice("Starting HTTP server...\n");
    httpServer.on("/", handleRoot);  
    httpServer.on("/update-config", HTTP_POST, handleConfigPost);
    httpServer.begin();
    Log.notice("HTTP server started at IP: %s\n", WiFi.localIP().toString().c_str());
}

void stopHTTPServer() {
    resetWatchdog();
    httpServer.close();
    Log.notice("HTTP server stopped.\n");
}

void handleRoot() {
    httpServer.send(200, "text/html", "<html><body><h1>ESP32 Web Server</h1></body></html>");
}

// Centralized function to handle reconnection logic
bool reconnectWiFi(const String& ssid, const String& password) {
    if (ssid.isEmpty() || ssid.length() > 32) {
        Log.error("Invalid SSID: '%s'. Must be non-empty and <= 32 characters.\n", ssid.c_str());
        return false;
    }

    WiFi.disconnect();
    delay(1000);
    WiFi.begin(ssid.c_str(), password.c_str());
    Log.notice("Attempting to connect to Wi-Fi: SSID='%s'\n", ssid.c_str());

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
        delay(500);
        Log.info(".");
        resetWatchdog();  // Prevent watchdog timeout
    }

    if (WiFi.status() == WL_CONNECTED) {
        Log.notice("\nConnected to Wi-Fi. IP: %s\n", WiFi.localIP().toString().c_str());
        return true;
    } else {
        Log.error("\nFailed to connect to Wi-Fi with SSID='%s'.\n", ssid.c_str());
        return false;
    }
}

// Validate and apply Wi-Fi and MQTT configuration
bool validateAndApplyConfig(const String& ssid, const String& password,
                            const String& broker, int port,
                            const String& user, const String& pass) {
                                
    if (!reconnectWiFi(ssid, password)) {
        Log.error("Wi-Fi validation failed.\n");
        return false;
    }

    if (!validateMQTTCredentials(broker, port, user, pass)) {
        Log.error("MQTT validation failed.\n");
        return false;
    }

    return true;
}

void handleConfigGet() {
    if (httpServer.method() != HTTP_GET) {
        httpServer.send(405);
        return;
    }

    StaticJsonDocument <512>response;
    response["status"] = "success";
    response["message"] = "Current configuration";
    response["publishInterval"] = publishInterval;
    response["WiFi"]["SSID"] = WiFi.SSID();
    response["WiFi"]["IP"] = WiFi.localIP().toString();

    String jsonResponse;
    serializeJson(response, jsonResponse);
    httpServer.send(200, "application/json", jsonResponse);
}

void handleConfigPost() {
    Log.notice("handle configuration post triggered.\n");
    if (httpServer.method() != HTTP_POST || !httpServer.hasArg("plain")) {
        httpServer.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid request.\"}");
        return;
    }

    String payload = httpServer.arg("plain");
    Log.notice("Received payload: %s\n", payload.c_str());

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
        Log.error("Failed to parse JSON: %s. Payload: %s\n", error.c_str(), payload.c_str());
        httpServer.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON format.\"}");
        return;
    }

    // Extract new credentials
    String newSSID = doc["ssid"] | "";
    String newPassword = doc["password"] | "";
    String newBroker = doc["mqtt_broker"] | "";
    int newPort = doc["mqtt_port"] | 0;
    String newUser = doc["mqtt_user"] | "";
    String newPass = doc["mqtt_password"] | "";

    if (newSSID.isEmpty() || newPassword.isEmpty() || newBroker.isEmpty() || newPort <= 0 || newUser.isEmpty() || newPass.isEmpty()) {
        httpServer.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing required fields.\"}");
        Log.error("Configuration missing required fields.\n");
        return;
    }

    if (newSSID == wifiSSID && newPassword == wifiPassword &&
        newBroker == mqttBroker && newPort == mqttPort &&
        newUser == mqttUser && newPass == mqttPassword) {
        httpServer.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Configuration unchanged.\"}");
        Log.notice("Configuration unchanged. No action taken.\n");
        return;
    }

    esp_task_wdt_reset();

    // Validate and apply configuration without stopping the server
    bool configApplied = validateAndApplyConfig(newSSID, newPassword, newBroker, newPort, newUser, newPass);
    if (configApplied) {
        saveConfig(newSSID, newPassword, newBroker, newPort, newUser, newPass);
        httpServer.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Configuration updated.\"}");
        Log.notice("Configuration updated successfully.\n");
    } else {
        httpServer.send(500, "application/json", "{\"status\":\"error\",\"message\":\"Failed to apply configuration.\"}");
        Log.error("Failed to apply configuration.\n");
    }
}

void revertToSavedCredentials(const String& savedSSID, const String& savedPassword,
                              const String& savedBroker, int savedPort,
                              const String& savedUser, const String& savedPass) {
    Log.notice("Reverting to saved Wi-Fi and MQTT credentials...\n");
    reconnectWiFi(savedSSID, savedPassword);
    mqttClient.setServer(savedBroker.c_str(), savedPort);

    unsigned long startTime = millis();
    while (!mqttClient.connected() && millis() - startTime < 10000) {
        if (mqttClient.connect("ESP32Client", savedUser.c_str(), savedPass.c_str())) {
            Log.notice("Reconnected to saved MQTT Broker.\n");
            return;
        }
        delay(500);
        resetWatchdog();
    }
    Log.error("Failed to reconnect to saved MQTT Broker.\n");
}

void enterHotspotMode() {
    Log.warning("Entering Hotspot Mode. Awaiting configuration...\n");
    WiFi.softAP("ESP32_Config", "12345678");
    Log.notice("Hotspot started. SSID: ESP32_Config, IP: %s\n", WiFi.softAPIP().toString().c_str());

    startTCPServer(); // Start TCP server

    unsigned long startTime = millis();
    while (millis() - startTime < 300000) { // 5-minute timeout
        checkForTCPClients(); // Handle TCP clients
        resetWatchdog();
        delay(10);

        if (newConfigFlag) break; // Exit if new config received
    }

    stopTCPServer(); // Stop TCP server
    WiFi.softAPdisconnect();
    Log.notice("Exiting Hotspot Mode.\n");
}