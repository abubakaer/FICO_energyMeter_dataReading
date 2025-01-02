#ifndef WATCHDOG_MANAGER_H
#define WATCHDOG_MANAGER_H

#include <esp_task_wdt.h>
#include <Arduino.h>

void initializeWatchdog(int timeoutSeconds);
void resetWatchdog();

#endif