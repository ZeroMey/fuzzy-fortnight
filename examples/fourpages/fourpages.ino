#include <Arduino.h>             // Standard Arduino includes
#include <lvgl.h>                // LVGL library for display management
#include <Adafruit_GFX.h>        // GFX library for display support
#include <Adafruit_ILI9341.h>    // Example display (change to your display driver)
#include <Wire.h>                // I2C library (for sensors)
#include <SPI.h>                 // SPI library (if using SPI-based sensors)
#include <XPT2046_Touchscreen.h> // Touchscreen driver (if using a resistive touchscreen)

#include "watch_page.h"          // Include the header for the watch page
#include "telemetry_page.h"      // Include the header for the telemetry page
#include "screensaver_page.h"    // Include the header for the screensaver page
#include "health_page.h"         // Include the header for the health page

#include "max30102_sensor.h"     // Include MAX30102 sensor header
#include "mlx90614-dss_sensor.h" // Include MLX90614 sensor header
#include "temt6000_sensor.h"     // Include TEMT6000 ambient light sensor header
#include "dht22_sensor.h"        // Include DHT22 sensor header
#include "QMI8658.h"             // Include QMI8658 accelerometer header
#include "SEN0502.h"             // Include DFRobot Rotary Encoder (SEN0502) header

#include "touchscreen.h"         // Include touchscreen input handler
#include "display_manager.h"     // Include display manager functions
#include "utils.h"               // Include utility functions
#include "power_manager.h"       // Include power management functions

// Global variables for the display and touchscreen
#define TFT_CS     5     // Chip select pin for TFT display
#define TFT_RST    22    // Reset pin for TFT display
#define TFT_DC     21    // Data/Command pin for TFT display
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST); // Create display object

XPT2046_Touchscreen ts(6, 7); // Define touchscreen (example pins, change as needed)

// FreeRTOS task handles
TaskHandle_t watchPageTaskHandle;
TaskHandle_t telemetryPageTaskHandle;
TaskHandle_t screensaverPageTaskHandle;
TaskHandle_t healthPageTaskHandle;

SEN0502 rotaryEncoder(33, 32);  // DFRobot Rotary Encoder connected to pins 33 and 32

void setup() {
  // Initialize Serial communication (optional for debugging)
  Serial.begin(115200);
  delay(2000);  // Give time for the serial monitor to connect

  // Initialize LVGL
  lv_init();

  // Initialize the display (Adafruit_ILI9341 using GFX library)
  initDisplay();

  // Initialize the touchscreen
  initTouchscreen();

  // Initialize I2C communication
  Wire.begin();  // Initialize the I2C bus (default pins are used for ESP32)

  // Initialize sensors (MAX30102, MLX90614, TEMT6000, DHT22, etc.)
  initMAX30102();
  initMLX90614();
  initTEMT6000();
  initDHT22();
  initQMI8658();

  // Initialize power management (e.g., sleep modes, wakeup triggers)
  initPowerManager();

  // Initialize the rotary encoder
  rotaryEncoder.begin();

  // Create FreeRTOS tasks for each page
  xTaskCreatePinnedToCore(watchPageTask, "Watch Page Task", 4096, NULL, 1, &watchPageTaskHandle, 0);
  xTaskCreatePinnedToCore(telemetryPageTask, "Telemetry Page Task", 4096, NULL, 2, &telemetryPageTaskHandle, 0);
  xTaskCreatePinnedToCore(screensaverPageTask, "Screensaver Page Task", 4096, NULL, 1, &screensaverPageTaskHandle, 0);
  xTaskCreatePinnedToCore(healthPageTask, "Health Page Task", 4096, NULL, 3, &healthPageTaskHandle, 0);
}

void loop() {
  // Handle LVGL tasks (display updates, animations, etc.)
  lv_task_handler();
  
  // Allow FreeRTOS to schedule tasks (like reading sensors or handling input)
  delay(5);  // Small delay for better task switching
  
  // Periodically update rotary encoder state for navigation
  rotaryEncoder.update();
  
  // Example: Use rotary encoder to switch between pages
  if (rotaryEncoder.readDirection() == 1) {
    // Rotate clockwise -> Next page
    switchPageNext();
  } else if (rotaryEncoder.readDirection() == -1) {
    // Rotate counterclockwise -> Previous page
    switchPagePrevious();
  }
}

// Function to switch to the next page (e.g., from watch to telemetry)
void switchPageNext() {
  lv_scr_load(telemetry_page);
}

// Function to switch to the previous page (e.g., from telemetry to watch)
void switchPagePrevious() {
  lv_scr_load(watch_page);
}

// Initialize the display using GFX and Adafruit_ILI9341
void initDisplay() {
  // Initialize the TFT display
  tft.begin();
  tft.setRotation(3); // Set the display rotation (adjust as needed)
  tft.fillScreen(ILI9341_BLACK); // Clear the display to black
}

// Initialize the touchscreen (XPT2046)
void initTouchscreen() {
  ts.begin();
  ts.setRotation(3); // Set the touchscreen rotation to match display
}

// Touch input handler function
void handleTouchInput() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint(); // Get touch coordinates
    // Handle touch gestures (e.g., swipe, tap) and update LVGL accordingly
    // For example, use touch input to navigate between pages
  }
}

// Initialize all I2C sensors
void initMAX30102() {
  // Initialize MAX30102 sensor for heart rate and SpO2
  if (!max30102.begin()) {
    Serial.println("MAX30102 initialization failed!");
    while (1);
  }
}

void initMLX90614() {
  // Initialize MLX90614 IR temperature sensor
  if (!mlx.begin()) {
    Serial.println("MLX90614 initialization failed!");
    while (1);
  }
}

void initTEMT6000() {
  // Initialize TEMT6000 ambient light sensor (if needed)
}

void initDHT22() {
  // Initialize DHT22 sensor for temperature and humidity
  dht.begin();
}

void initQMI8658() {
  // Initialize QMI8658 accelerometer
  if (!qmi8658.begin()) {
    Serial.println("QMI8658 initialization failed!");
    while (1);
  }
}
