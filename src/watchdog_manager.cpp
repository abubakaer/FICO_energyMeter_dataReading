#include "watchdog_manager.h"
#include "global_variables.h"

void initializeWatchdog(int timeoutSeconds) {
  esp_task_wdt_init(timeoutSeconds, true); // Enable panic on timeout
  esp_task_wdt_add(NULL); // Add the current task to the watchdog
}

void resetWatchdog() {
  esp_task_wdt_reset();
}