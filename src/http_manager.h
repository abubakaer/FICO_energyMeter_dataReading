#ifndef HTTP_MANAGER_H
#define HTTP_MANAGER_H

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoLog.h>
#include "config_manager.h"
#include "global_variables.h"
#include <Preferences.h>

extern WebServer httpServer;  // HTTP server on port 80

void handleRoot();
void handleConfigGet();
void handleConfigPost();
bool reconnectWiFi(const String& ssidb ="", const String& password = "");

// HTTP Server management
void startHTTPServer();
void stopHTTPServer();

// Configuration and Validation
bool validateAndApplyConfig(const String& ssid, const String& password,
                            const String& broker, int port,
                            const String& user, const String& pass);
void revertToSavedCredentials(const String& savedSSID, const String& savedPassword,
                               const String& savedBroker, int savedPort,
                               const String& savedUser, const String& savedPass);

// Hotspot Mode
void enterHotspotMode();


#endif  // HTTP_MANAGER_H
