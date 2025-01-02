#include "wifi_manager.h"
#include "global_variables.h"
#include "watchdog_manager.h"

// Attempt to connect to Wi-Fi
void connectWiFi() {
    Log.info("Connecting to Wi-Fi...\n");
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
        delay(500);
        Log.info(".");
        resetWatchdog();
    }

    if (WiFi.status() == WL_CONNECTED) {
        Log.notice("Connected to Wi-Fi. IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Log.error("Failed to connect to Wi-Fi......\n");
    }
}

// Validate Wi-Fi credentials
bool validateWiFiCredentials(const String& ssid, const String& password) {
    Log.info("Validating Wi-Fi credentials...\n");
    WiFi.disconnect();
    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
        delay(500);
        resetWatchdog();
    }

    bool isConnected = WiFi.status() == WL_CONNECTED;
    if (isConnected) {
        Log.notice("Wi-Fi credentials validated successfully.\n");
    } else {
        Log.error("Wi-Fi credentials validation failed.\n");
    }
    return isConnected;
}

// Handle reconnection logic
void handleWiFiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        connectWiFi();
    }
}