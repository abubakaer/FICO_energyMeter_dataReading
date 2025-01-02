#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <ModbusMaster.h>
#include <Arduino.h>

// Global Variables
extern String wifiSSID, wifiPassword, mqttBroker, mqttUser, mqttPassword;
extern int mqttPort;
extern bool isHTTPServerRunning; // Tracks the server's state
extern Preferences preferences;
extern WiFiClient espClient;
extern PubSubClient mqttClient;
extern ModbusMaster node;
extern String newConfigPayload;  // Stores new configuration payload
extern bool newConfigFlag;
extern bool isHotspotMode;

// Timing variables
extern unsigned long publishInterval;

#endif  // GLOBAL_VARIABLES_H
