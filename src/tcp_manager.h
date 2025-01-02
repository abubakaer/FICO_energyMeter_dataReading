#ifndef TCP_MANAGER_H
#define TCP_MANAGER_H

#include <WiFi.h>
#include <ArduinoLog.h>
#include <WiFiServer.h>
#include <global_variables.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>

void startTCPServer();              // Initialize TCP server
void handleTCPConnection(WiFiClient client);          // Check and handle TCP clients
void checkForTCPClients();
void stopTCPServer();
#endif