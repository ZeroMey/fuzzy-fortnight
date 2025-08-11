#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>

void initPowerManager();        // Initialize power management (sleep, wakeup)
void enterDeepSleep();          // Enter deep sleep mode to save power
void wakeUpFromSleep();         // Wake up from sleep when triggered (e.g., touch, accelerometer)

#endif
