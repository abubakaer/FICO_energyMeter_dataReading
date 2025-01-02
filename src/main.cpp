#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "modbus_manager.h"
#include "config_manager.h"
#include "tcp_manager.h"
#include "watchdog_manager.h"
#include "global_variables.h"
#include "global_constants.h"
#include "http_manager.h"

//code starts from here 

enum OperationState { NORMAL_OPERATION, CONFIG_MODE_INIT, CONFIG_MODE_ACTIVE, CONFIG_MODE_DONE };
OperationState currentState = NORMAL_OPERATION;

void setup() {
    Serial.begin(115200);
    Log.begin(LOG_LEVEL_VERBOSE, &Serial);
    Serial2.begin(9600, SERIAL_8N1, 16, 17);  // Modbus communication
    node.begin(1, Serial2);
    node.preTransmission(preTransmission);
    node.postTransmission(postTransmission);

    initializeWatchdog(10);  // Initialize watchdog with a 10-second timeout
    initializeModbus();      // Initialize Modbus communication
    loadConfig();            // Load saved configurations
    connectWiFi();           // Connect to Wi-Fi
    initializeMQTT();
    connectMQTT();           // Connect to MQTT Broker
    Log.notice("HTTP server initialized and started.\n");
}


void loop() {
    resetWatchdog();  // Reset watchdog regularly

    // Handle Modbus data reading
    static unsigned long lastModbusTime = 0;
    if (millis() - lastModbusTime >= 1000) {
        readModbusData();
        lastModbusTime = millis();
    }

    mqttClient.loop();  // Handle MQTT client loop

    static unsigned long lastPublishTime = 0;
    if (millis() - lastPublishTime >= publishInterval) {
        publishReadingsToMQTT();
        lastPublishTime = millis();
    }

    // Dynamically start/stop HTTP server based on newConfigFlag
    if (newConfigFlag && !isHTTPServerRunning) {
        Log.notice("Starting HTTP server due to newConfigFlag.\n");
        startHTTPServer();
        isHTTPServerRunning = true;
    } else if (!newConfigFlag && isHTTPServerRunning) {
        Log.notice("Stopping HTTP server as newConfigFlag is cleared.\n");
        stopHTTPServer();
        isHTTPServerRunning = false;
    }

    // Process HTTP requests only if the server is running
    if (isHTTPServerRunning) {
        httpServer.handleClient();
    }

    switch (currentState) {
        case NORMAL_OPERATION:
            handleWiFiConnection();  // Handle Wi-Fi reconnection logic
            if (WiFi.status() != WL_CONNECTED) {
                reconnectWiFi(wifiSSID, wifiPassword);
            }
            if (!mqttClient.connected() && WiFi.status() == WL_CONNECTED) {
                connectMQTT();
            }
            if (newConfigFlag && currentState != CONFIG_MODE_INIT) {
                Log.notice("Switching to CONFIG_MODE_INIT.\n");
                currentState = CONFIG_MODE_INIT;
            }
            break;

        case CONFIG_MODE_INIT:
            Log.notice("Entering configuration mode...\n");
            if (!isHTTPServerRunning) {
                startHTTPServer();
                isHTTPServerRunning = true;
            }
            currentState = CONFIG_MODE_ACTIVE;  // Move to active mode immediately
            break;

        case CONFIG_MODE_ACTIVE:
            static unsigned long configStartTime = millis();
            resetWatchdog();

            // Exit if config is done or timeout occurs
            if (!newConfigFlag || millis() - configStartTime > 300000) {
                Log.warning("Exiting configuration mode. Either timeout or config complete.\n");
                currentState = CONFIG_MODE_DONE;
            }
            break;

        case CONFIG_MODE_DONE:
            stopHTTPServer();
            if (WiFi.status() == WL_CONNECTED && mqttClient.connected()) {
                Log.notice("Configuration complete. Returning to NORMAL_OPERATION.\n");
                newConfigFlag = false;  // Ensure the flag is reset
                currentState = NORMAL_OPERATION;
            } else {
                Log.warning("Configuration incomplete. Entering Hotspot mode.\n");
                enterHotspotMode();
            }
            break;
    }
}