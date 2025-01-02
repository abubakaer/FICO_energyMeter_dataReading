#include "mqtt_manager.h"
#include <ArduinoLog.h>
#include "watchdog_manager.h"
#include "http_manager.h"

const char* publish_topic = "meter/reading";     // MQTT topic to publish readings
const char* subscribe_topic = "meter/interval";  // Topic to change publish interval dynamically

// Function to initialize MQTT
void initializeMQTT() {
  mqttClient.setServer(mqttBroker.c_str(), mqttPort);
  mqttClient.setCallback(mqttCallback);  // Set the callback function
}

bool validateMQTTCredentials(const String& broker, int port, const String& user, const String& password) {
    mqttClient.setServer(broker.c_str(), port);
    Log.notice("Validating MQTT Broker: %s\n", broker.c_str());

    unsigned long mqttStartTime = millis();
    while (!mqttClient.connected() && millis() - mqttStartTime < 5000) {
        if (mqttClient.connect("ESP32Client", user.c_str(), password.c_str())) {
            Log.notice("MQTT Broker validation successful.\n");
            return true;
        }
        delay(500);
        resetWatchdog();
    }

    Log.error("Failed to validate MQTT Broker credentials.\n");
    return false;
}

// Function to connect to MQTT Broker
void connectMQTT() {
    int retryCount = 0;
    while (!mqttClient.connected() && retryCount < 5) {
        Log.info("Connecting to MQTT Broker: %s, Attempt %d...\n", mqttBroker.c_str(), retryCount + 1);
        if (mqttClient.connect("ESP32Client", mqttUser.c_str(), mqttPassword.c_str())) {
            Log.notice("Connected to MQTT Broker.\n");
            mqttClient.subscribe("meter/interval");
            mqttClient.subscribe("config/update");
            return;
        } else {
            Log.error("Failed to connect to MQTT. State: %d\n", mqttClient.state());
            retryCount++;
            delay(2000);
            resetWatchdog();
        }
    }

    if (!mqttClient.connected()) {
        Log.error("Failed to connect to MQTT after retries.\n");
    }
}

// Callback function for subscribed topics
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Log.info("Message received on topic: %s\n", topic);

  // Convert payload to a string
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  message.trim(); // Remove any unwanted whitespace or newline characters
  Log.info("Received message: %s", message.c_str());

  // Check if the topic matches and handle interval updates
  if (String(topic).equals(subscribe_topic)) {
    int newInterval = message.toInt(); // Convert to integer
    if (newInterval >= 5000 && newInterval <= 60000) { // Validate range
      publishInterval = newInterval;
      Log.notice("Publish interval updated to %d ms.\n", publishInterval);
    } else {
      Log.error("Invalid interval: %d. Must be between 5000 and 60000 ms.\n", newInterval);
    }
  }
  
  else if (String(topic).equals("config/update")) {  // Add a new topic for flag
    if (message.equalsIgnoreCase("true") && !newConfigFlag) {
      Log.notice("Configuration update flag set to true.\n");
      newConfigFlag = true;  // Set flag to true
    }
    else{
      message.equalsIgnoreCase("false");
      Log.notice("Configuration update flag set to false.\n");
      newConfigFlag = false;  // Set flag to false
    }
  }
}

// Function to publish JSON data to MQTT
void publishReadingsToMQTT() {
  Log.verbose("Preparing to publish readings to MQTT.\n");

  StaticJsonDocument<512> doc;
  doc["PhaseA_Voltage"] = voltageA;
  doc["PhaseB_Voltage"] = voltageB;
  doc["PhaseC_Voltage"] = voltageC;
  doc["PhaseA_Current"] = currentA;
  doc["PhaseB_Current"] = currentB;
  doc["PhaseC_Current"] = currentC;
  doc["Frequency"] = frequency;
  doc["EnergyEp"] = energyEp;

  char jsonOutput[512];
  if (serializeJson(doc, jsonOutput) == 0) {
    Log.error("Failed to serialize JSON.\n");
    return;
  }

  if (mqttClient.connected()) {
    if (mqttClient.publish(publish_topic, jsonOutput)) {
      Log.notice("Published data to MQTT topic '%s': %s\n", publish_topic, jsonOutput);
    } else {
      Log.error("Failed to publish JSON data to MQTT.\n");
    }
  } else {
    Log.warning("MQTT not connected. Attempting to reconnect...\n");
    connectMQTT();
  }
}

