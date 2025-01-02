#include "tcp_manager.h"
#include "config_manager.h"
#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "global_variables.h"
#include <esp_task_wdt.h>
#include <ArduinoJson.h>
#include "FS.h"
#include "http_manager.h"

WiFiServer tcpServer(90); // TCP server listening on port 90

void startTCPServer() {
    Log.info("Starting TCP server...\n");
    tcpServer.begin();
}

void handleTCPConnection(WiFiClient client) {
    String message = "";
    unsigned long timeoutStart = millis();

    while (client.connected() && millis() - timeoutStart < 5000) {
        while (client.available()) {
            char c = client.read();
            message += c;
            timeoutStart = millis();
        }
    }

    message.trim();
    Log.info("Received TCP message: %s\n", message.c_str());

    if (!message.startsWith("{") || !message.endsWith("}")) {
        Log.error("Invalid JSON format received.\n");
        client.println("{\"status\":\"error\",\"message\":\"Invalid JSON format.\"}");
        return;
    }

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        Log.error("JSON deserialization failed: %s\n", error.c_str());
        client.println("{\"status\":\"error\",\"message\":\"JSON deserialization failed.\"}");
        return;
    }

    String ssid = doc["ssid"] | "";
    String password = doc["password"] | "";
    String broker = doc["mqtt_broker"] | "";
    int port = doc["mqtt_port"] | 0;
    String user = doc["mqtt_user"] | "";
    String pass = doc["mqtt_password"] | "";

    if (reconnectWiFi(ssid, password) && validateMQTTCredentials(broker, port, user, pass)) {
        saveConfig(ssid, password, broker, port, user, pass);
        client.println("{\"status\":\"success\",\"message\":\"Configuration applied successfully.\"}");
        ESP.restart();
    } else {
        client.println("{\"status\":\"error\",\"message\":\"Failed to apply configuration.\"}");
    }
}

void checkForTCPClients() {
    WiFiClient client = tcpServer.available(); // Check for incoming clients
    if (client) {
        Log.info("TCP client connected. Handling request...\n");
        handleTCPConnection(client); // Process client request
        client.stop(); // Close connection after handling
        Log.info("TCP client disconnected.\n");
    }
}

void stopTCPServer() {
    tcpServer.stop();
    Log.notice("TCP server stopped.\n");
}