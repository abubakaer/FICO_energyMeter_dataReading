#include "config_manager.h"
#include "global_variables.h"

extern String wifiSSID, wifiPassword, mqttBroker, mqttUser, mqttPassword;
extern int mqttPort;

void loadConfig() {
    preferences.begin("config", true);  // Open preferences in read-only mode
    wifiSSID = preferences.getString("wifiSSID", "");
    wifiPassword = preferences.getString("wifiPassword", "");
    mqttBroker = preferences.getString("mqttBroker", "");
    mqttPort = preferences.getInt("mqttPort", 1883);  // Default MQTT port is 1883
    mqttUser = preferences.getString("mqttUser", "");
    mqttPassword = preferences.getString("mqttPassword", "");
    preferences.end();

    Log.notice("Configuration loaded: Wi-Fi SSID=%s, MQTT Broker=%s, Port=%d\n", wifiSSID.c_str(), mqttBroker.c_str(), mqttPort);
}

void saveConfig(String ssid, String password, String broker, int port, String user, String pass) {
    preferences.begin("config", false);  // Open preferences in write mode
    preferences.putString("wifiSSID", ssid);
    preferences.putString("wifiPassword", password);
    preferences.putString("mqttBroker", broker);
    preferences.putInt("mqttPort", port);
    preferences.putString("mqttUser", user);
    preferences.putString("mqttPassword", pass);
    preferences.end();

    Log.notice("Configuration saved: Wi-Fi SSID=%s, MQTT Broker=%s, Port=%d\n", ssid.c_str(), broker.c_str(), port);
}

void clearConfig() {
  preferences.begin("config", false);
  preferences.clear();
  preferences.end();
}

