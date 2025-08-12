#include "utils.h"
#include <Arduino.h>

// Function to format temperature readings (Celsius to Fahrenheit)
float convertToFahrenheit(float celsius) {
    return (celsius * 9.0 / 5.0) + 32.0;  // Convert Celsius to Fahrenheit
}

// Function to format temperature data for display
const char* formatTemperature(float tempCelsius) {
    static char tempStr[10];
    snprintf(tempStr, sizeof(tempStr), "%.1f°C", tempCelsius);  // Format temperature in Celsius
    return tempStr;
}

// Function to calculate the average of an array of floats
float averageData(float* data, int size) {
    if (size == 0) return 0.0;  // Avoid division by zero

    float sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum / size;
}

// Function to convert temperature from Fahrenheit to Celsius
float convertToCelsius(float fahrenheit) {
    return (fahrenheit - 32.0) * 5.0 / 9.0;  // Convert Fahrenheit to Celsius
}

// Function to format humidity readings for display
char* formatHumidity(float humidity) {
    static char humidityStr[10];
    snprintf(humidityStr, sizeof(humidityStr), "%.1f%%", humidity);  // Format humidity
    return humidityStr;
}

// Function to format light intensity readings (in lux)
char* formatLightIntensity(int lightIntensity) {
    static char lightStr[20];
    snprintf(lightStr, sizeof(lightStr), "%d lx", lightIntensity);  // Format light intensity
    return lightStr;
}

// Function to format battery percentage for display
char* formatBatteryPercentage(int batteryPercentage) {
    static char batteryStr[20];
    snprintf(batteryStr, sizeof(batteryStr), "Battery: %d%%", batteryPercentage);  // Format battery status
    return batteryStr;
}

// Function to calculate the difference between two time points (in milliseconds)
unsigned long timeDifference(unsigned long startTime, unsigned long endTime) {
    if (endTime >= startTime) {
        return endTime - startTime;  // Return the difference if end time is greater than start time
    } else {
        return (ULONG_MAX - startTime) + endTime;  // Handle overflow
    }
}

// Function to debounce sensor readings (to smooth out spikes in sensor data)
float debounceSensorData(float* data, int size) {
    float filteredData[size];
    float sum = 0.0;
    int threshold = 5;  // Define a threshold for filtering

    for (int i = 0; i < size; i++) {
        if (data[i] < threshold) {
            filteredData[i] = data[i];  // Keep data if it is below threshold
        } else {
            filteredData[i] = 0;  // Remove outliers by setting them to 0
        }
        sum += filteredData[i];  // Sum up the filtered data
    }

    return sum / size;  // Return the average of the filtered data
}
