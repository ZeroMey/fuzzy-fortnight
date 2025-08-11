#ifndef HEALTH_PAGE_H
#define HEALTH_PAGE_H

#include <lvgl.h>

void createHealthPage();         // Setup health telemetry UI (heart rate, SpO2)
void updateHealthPage();         // Update telemetry page with new sensor data

#endif
