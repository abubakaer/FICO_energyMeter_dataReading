#ifndef MODBUS_MANAGER_H
#define MODBUS_MANAGER_H

#include <ModbusMaster.h>

// Declare external variables
extern ModbusMaster node;
extern float voltageA, voltageB, voltageC;
extern float currentA, currentB, currentC;
extern float frequency, energyEp;

// Function declarations
void initializeModbus();
void readModbusData();
void preTransmission();
void postTransmission();

#endif // MODBUS_MANAGER_H