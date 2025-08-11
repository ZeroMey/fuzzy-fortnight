// Auto-generated from examples/fourpages/fourpages.ino

#include <Arduino.h>
#include <lvgl.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Wire.h>
#include "pin_config.h"
#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"
#include "HWCDC.h"
HWCDC USBSerial;

#include "watch_page.h"
#include "telemetry_page.h"
#include "screensaver_page.h"
#include "health_page.h"

#include "PPGSensor.h"
#include "SkinTemperatureSensor.h"
#include "AmbientLightSensor.h"
#include "AmbientTemperatureSensor.h"
#include "InertialSensor.h"
#include "RotaryEncoder.h"

#include "touchscreen.h"
#include "display_manager.h"
#include "utils.h"

#include "main.h"

// Global variables (mirrored from original sketch)
TaskHandle_t watchPageTaskHandle;
TaskHandle_t telemetryPageTaskHandle;
TaskHandle_t screensaverPageTaskHandle;
TaskHandle_t healthPageTaskHandle;

SEN0502 rotaryEncoder(33, 32);  // DFRobot Rotary Encoder connected to pins 33 and 32

// Forward declarations for helper initializers (from original sketch)
static void initPPGSensor();
static void initSkinTemperatureSensor();
static void initAmbientLightSensor();
static void initAmbientTemperatureSensor();
static void initInertialSensor();
static void initRotaryEncoder();

// Page task entry points (as in the original sketch)
static void watchPageTask(void *pvParameters);
static void telemetryPageTask(void *pvParameters);
static void screensaverPageTask(void *pvParameters);
static void healthPageTask(void *pvParameters);

void app_setup() {
  // Initialize Serial communication (optional for debugging)
  Serial.begin(115200);
  delay(2000);  // Give time for the serial monitor to connect

  // Initialize LVGL
  lv_init();

  // Initialize the display (Adafruit_ST7789 using GFX library)
  initDisplay();

  // Initialize the touchscreen
  initTouchscreen();

  // Initialize I2C communication
  Wire.begin();  // Initialize I2C (SDA/SCL default pins based on board)
  // Initialize SPI if required for certain sensors
  SPI.begin();

  // Initialize the rotary encoder
  rotaryEncoder.begin();

  // Initialize sensors (as in original sketch; comment out those you don’t have wired)
  initPPGSensor();                 // Heart rate / SpO2
  initSkinTemperatureSensor();     // IR temperature
  initAmbientLightSensor();        // Ambient light
  initAmbientTemperatureSensor();  // Temp/Humidity
  initInertialSensor();            // IMU (accelerometer/gyro)
  initRotaryEncoder();             // Rotary Encoder

  // Create LVGL UI pages
  createWatchPage();
  createTelemetryPage();
  createHealthPage();
  createScreensaverPage();

  // Create FreeRTOS tasks for each page (priorities/sizes per original)
  xTaskCreatePinnedToCore(watchPageTask, "Watch Page Task", 4096, NULL, 1, &watchPageTaskHandle, 0);
  xTaskCreatePinnedToCore(telemetryPageTask, "Telemetry Page Task", 4096, NULL, 2, &telemetryPageTaskHandle, 0);
  xTaskCreatePinnedToCore(screensaverPageTask, "Screensaver Page Task", 4096, NULL, 1, &screensaverPageTaskHandle, 0);
  xTaskCreatePinnedToCore(healthPageTask, "Health Page Task", 4096, NULL, 3, &healthPageTaskHandle, 0);
}

void app_loop() {
  // Handle LVGL tasks (display updates, animations, etc.)
  // For LVGL v8+, prefer lv_timer_handler(); if you used lv_task_handler() originally, keep it.
  lv_timer_handler();

  // Allow FreeRTOS to schedule tasks (like reading sensors or handling input)
  delay(5);  // Small delay for better task switching

  // Periodically update rotary encoder state for navigation
  rotaryEncoder.update();

  // Example: Use rotary encoder to switch between pages
  if (rotaryEncoder.readDirection() == SEN0502::Direction::RIGHT) {
    // Switch to next page
    // transitionToPage(next_page_obj);
  } else if (rotaryEncoder.readDirection() == SEN0502::Direction::LEFT) {
    // Switch to previous page
    // transitionToPage(prev_page_obj);
  }

  // Update display (if your display manager does buffered/scheduled updates)
  updateDisplay();
}

// ---------------------- Page task stubs (from original sketch) ----------------------

static void watchPageTask(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    updateWatchPage();
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

static void telemetryPageTask(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    updateTelemetryPage();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

static void screensaverPageTask(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    updateScreensaverPage();
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

static void healthPageTask(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    updateHealthPage();
    vTaskDelay(pdMS_TO_TICKS(150));
  }
}
