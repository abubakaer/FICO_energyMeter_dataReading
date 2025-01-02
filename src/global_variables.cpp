#include "global_variables.h"
#include <Preferences.h>

// Define global variables
WiFiClient espClient;
PubSubClient mqttClient(espClient);
ModbusMaster node;
Preferences preferences;
String wifiSSID, wifiPassword, mqttBroker, mqttUser, mqttPassword;
int mqttPort;
bool isHTTPServerRunning = false; // Tracks the server's state
bool newConfigFlag = false;
String newConfigPayload = "";


// Define modbus and publish intervals
unsigned long publishInterval = 15000;  // Default: 15 seconds
