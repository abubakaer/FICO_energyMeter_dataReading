#ifndef GLOBAL_CONSTANTS_H
#define GLOBAL_CONSTANTS_H

#define WATCHDOG_TIMEOUT 10  // seconds
#define AP_SSID "ESP32_Config"
#define AP_PASSWORD "12345678"

// Constant Modbus and MQTT publish intervals
extern const unsigned long modbusInterval = 1000;  // Default: 1 second


#endif  // GLOBAL_CONSTANTS_H
