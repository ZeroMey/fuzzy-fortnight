#include "health_page.h"
#include <Adafruit_GFX.h>        // GFX library for display support
#include <Adafruit_ST7789.h>     // ST7789V2 display driver
#include <Wire.h>                // I2C library for sensor communication
#include <MAX30105.h>            // MAX30102 sensor library

// Global display object
extern Adafruit_ST7789 tft;   // Create display object for ST7789 display

// Global page object and label variables
lv_obj_t *health_page;         // Current health page object
lv_obj_t *heart_rate_label;    // Heart rate label
lv_obj_t *spo2_label;          // SpO2 label
lv_obj_t *status_label;        // Status label

// MAX30102 sensor object
MAX30105 particleSensor;

// Function to create the Health page UI elements
void createHealthPage() {
    // Create the health page object
    health_page = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_bg_color(health_page, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);  // Set background color

    // Create label for heart rate
    heart_rate_label = lv_label_create(health_page, NULL);
    lv_label_set_text(heart_rate_label, "Heart Rate: -- bpm");  // Initial placeholder for heart rate
    lv_obj_align(heart_rate_label, NULL, LV_ALIGN_CENTER, 0, -30);  // Center the label on the screen

    // Create label for SpO2
    spo2_label = lv_label_create(health_page, NULL);
    lv_label_set_text(spo2_label, "SpO2: -- %");  // Initial placeholder for SpO2
    lv_obj_align(spo2_label, heart_rate_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  // Position below the heart rate label

    // Create label for status (e.g., data fetching or error message)
    status_label = lv_label_create(health_page, NULL);
    lv_label_set_text(status_label, "Initializing...");  // Initial status message
    lv_obj_align(status_label, spo2_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  // Position below the SpO2 label
}

// Function to update the Health page content (heart rate, SpO2)
void updateHealthPage() {
    // Fetch the heart rate and SpO2 values from the MAX30102 sensor
    uint8_t heartRate = 0;
    uint8_t spo2 = 0;
    bool sensorDataReady = false;

    // Check if the sensor is ready
    if (particleSensor.available()) {
        // Read the heart rate and SpO2 values
        heartRate = particleSensor.getHeartRate();
        spo2 = particleSensor.getSpO2();
        sensorDataReady = true;
        particleSensor.clear();  // Clear the sensor data for the next read
    }

    // Update the health data on the screen
    if (sensorDataReady) {
        // Update the heart rate label
        char heartRateStr[20];
        snprintf(heartRateStr, sizeof(heartRateStr), "Heart Rate: %d bpm", heartRate);
        lv_label_set_text(heart_rate_label, heartRateStr);

        // Update the SpO2 label
        char spo2Str[20];
        snprintf(spo2Str, sizeof(spo2Str), "SpO2: %d %%", spo2);
        lv_label_set_text(spo2_label, spo2Str);

        // Update the status label
        lv_label_set_text(status_label, "Data updated");
    } else {
        // Display an error message if sensor data is not available
        lv_label_set_text(status_label, "Sensor Error");
    }
}

// Function to initialize the Health page and MAX30102 sensor
void initHealthPage() {
    // Initialize the MAX30102 sensor
    if (!particleSensor.begin()) {
        Serial.println("MAX30102 initialization failed!");
        while (1);  // Halt the program if the sensor fails to initialize
    }

    // Set the sensor's parameters (adjust these for your specific application)
    particleSensor.setup();  // Configure sensor
    particleSensor.setMode(MAX30105_MODE_HEART_RATE);  // Set the mode for heart rate and SpO2 detection
    particleSensor.setSampleRate(400);  // Sample rate for heart rate detection
    particleSensor.setPulseWidth(160);  // Pulse width setting for sensor
    particleSensor.setADCRange(4096);  // Set ADC range for better accuracy
}

// Function to handle touch input for the Health page (using CST816T)
void handleHealthPageTouch() {
    TS_Point p = touchBus.getPoint();  // Get the current touch point

    if (p.x != -1 && p.y != -1) {  // Check if the screen was touched
        // Implement touch handling logic (e.g., navigate to other pages)
        Serial.print("Touch detected at X: ");
        Serial.print(p.x);
        Serial.print(", Y: ");
        Serial.println(p.y);

        // Example: Swipe left to navigate to watch page
        if (p.x < 100 && p.y > 100 && p.y < 200) {
            lv_scr_load(watch_page);   // Switch to watch page
        }

        // Example: Swipe right to navigate to screensaver page
        if (p.x > 200 && p.y > 100 && p.y < 200) {
            lv_scr_load(screensaver_page);  // Switch to screensaver page
        }
    }
}

// Function to simulate health data updates (for testing purposes)
void simulateHealthPage() {
    updateHealthPage();
    handleHealthPageTouch();  // Handle touch input for navigation
}
