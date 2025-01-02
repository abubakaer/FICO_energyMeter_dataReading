#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoLog.h>

void connectWiFi();
void handleWiFiConnection();
bool validateWiFiCredentials(String ssid, String password);

#endif