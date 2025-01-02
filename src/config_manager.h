#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Preferences.h>
#include <ArduinoLog.h>

void loadConfig();
void saveConfig(String ssid, String password, String broker, int port, String user, String pass);
void clearConfig();

#endif