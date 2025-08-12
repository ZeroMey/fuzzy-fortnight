#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

const char* formatTemperature(float temp);     // Format temperature values
float averageData(float* data, int size);  // Calculate average of sensor data
float convertToFahrenheit(float celsius); // Convert Celsius to Fahrenheit

#endif
