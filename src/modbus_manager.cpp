#include "modbus_manager.h"
#include "global_variables.h"
#include <ArduinoLog.h>
#include <Arduino.h>
#include "hardware_config.h"


float voltageA = 0, voltageB = 0, voltageC = 0;
float currentA = 0, currentB = 0, currentC = 0;
float frequency = 0, energyEp = 0;

// Pre-transmission callback for RS485
void preTransmission() {
  digitalWrite(RS485_DE_RE, HIGH); // Enable transmission mode
}

// Post-transmission callback for RS485
void postTransmission() {
  digitalWrite(RS485_DE_RE, LOW); // Enable reception mode
}

void initializeModbus() {
  pinMode(4, OUTPUT); // RS485 DE/RE
  digitalWrite(4, LOW);
  Serial2.begin(9600);
  node.begin(1, Serial2);
}

void readModbusData() {
  uint8_t result;
  uint16_t data[2];  // Declare temporary buffer to store Modbus response

  // Reading Phase A Voltage
  result = node.readHoldingRegisters(0x0060, 2);  // Register address and length
  if (result == node.ku8MBSuccess) {
    data[0] = node.getResponseBuffer(0);
    data[1] = node.getResponseBuffer(1);
    uint32_t combined = (uint32_t(data[1]) << 16) | data[0];
    memcpy(&voltageA, &combined, sizeof(voltageA));
    Log.info("Phase A Voltage: %F V\n", voltageA);
  } else {
    Log.error("Error reading energy: 0x%02X\n", result);
  }

  // Reading Phase B Voltage
  result = node.readHoldingRegisters(0x0062, 2);
  if (result == node.ku8MBSuccess) {
    data[0] = node.getResponseBuffer(0);
    data[1] = node.getResponseBuffer(1);
    uint32_t combined = (uint32_t(data[1]) << 16) | data[0];
    memcpy(&voltageB, &combined, sizeof(voltageB));
    Log.info("Phase B Voltage: %F V\n", voltageB);
  } else {
    Log.error("Error reading energy: 0x%02X\n", result);
  }

  // Reading Phase C Voltage
  result = node.readHoldingRegisters(0x0064, 2);
  if (result == node.ku8MBSuccess) {
    data[0] = node.getResponseBuffer(0);
    data[1] = node.getResponseBuffer(1);
    uint32_t combined = (uint32_t(data[1]) << 16) | data[0];
    memcpy(&voltageC, &combined, sizeof(voltageC));
    Log.info("Phase C Voltage: %F V\n", voltageC);
  } else {
    Log.error("Error reading energy: 0x%02X\n", result);
  }

  // Reading Phase A Current
  result = node.readHoldingRegisters(0x006C, 2);
  if (result == node.ku8MBSuccess) {
    data[0] = node.getResponseBuffer(0);
    data[1] = node.getResponseBuffer(1);
    uint32_t combined = (uint32_t(data[1]) << 16) | data[0];
    memcpy(&currentA, &combined, sizeof(currentA));
    Log.info("Phase A Current: %F A\n", currentA);
  } else {
    Log.error("Error reading energy: 0x%02X\n", result);
  }

  // Reading Phase B Current
  result = node.readHoldingRegisters(0x006E, 2);
  if (result == node.ku8MBSuccess) {
    data[0] = node.getResponseBuffer(0);
    data[1] = node.getResponseBuffer(1);
    uint32_t combined = (uint32_t(data[1]) << 16) | data[0];
    memcpy(&currentB, &combined, sizeof(currentB));
    Log.info("Phase B Current: %F A\n", currentB);
  } else {
    Log.error("Error reading energy: 0x%02X\n", result);
  }

  // Reading Phase C Current
  result = node.readHoldingRegisters(0x0070, 2);
  if (result == node.ku8MBSuccess) {
    data[0] = node.getResponseBuffer(0);
    data[1] = node.getResponseBuffer(1);
    uint32_t combined = (uint32_t(data[1]) << 16) | data[0];
    memcpy(&currentC, &combined, sizeof(currentC));
    Log.info("Phase C Current: %F A\n", currentC);
  } else {
    Log.error("Error reading energy: 0x%02X\n", result);
  }

  // Reading Frequency
  result = node.readHoldingRegisters(0x0054, 1);
  if (result == node.ku8MBSuccess) {
    data[0] = node.getResponseBuffer(0);
    frequency = (float)data[0] * 0.01;  // Scale frequency
    Log.info("Frequency: %F Hz\n", frequency);
  } else {
    Log.error("Error reading energy: 0x%02X\n", result);
  }

  // Reading Energy Ep
  result = node.readHoldingRegisters(0x0058, 2);
  if (result == node.ku8MBSuccess) {
    data[0] = node.getResponseBuffer(0);
    data[1] = node.getResponseBuffer(1);
    uint32_t combined = (uint32_t(data[1]) << 16) | data[0];
    memcpy(&energyEp, &combined, sizeof(energyEp));
    Log.info("Energy Ep: %F kWh\n", energyEp);
  } else {
    Log.error("Error reading energy: 0x%02X\n", result);
  }
}