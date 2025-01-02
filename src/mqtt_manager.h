#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "global_variables.h"  // Import global variables
#include "modbus_manager.h"    // For access to Modbus data

extern PubSubClient mqttClient;

// MQTT-related functions
void initializeMQTT();
void connectMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);
bool validateMQTTCredentials(const String &broker, int port, const String &user, const String &password);
void publishReadingsToMQTT();

#endif
