// Auto-generated from examples/fourpages/fourpages.ino

#include <Arduino.h>
#include <lvgl.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Wire.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include "watch_page.h"
#include "telemetry_page.h"
#include "screensaver_page.h"
#include "health_page.h"
#include "max30102_sensor.h"
#include "mlx90614-dss_sensor.h"
#include "temt6000_sensor.h"
#include "dht22_sensor.h"
#include "QMI8658.h"
#include "SEN0502.h"
#include "touchscreen.h"
#include "display_manager.h"
#include "utils.h"

#include "main.h"

// Global variables (mirrored from original sketch)
// NOTE: these depend on your actual wiring/libraries; keep as in your original sketch.
Adafruit_ILI9341 tft(/*CS=*/5, /*DC=*/2, /*RST=*/4);
XPT2046_Touchscreen ts(/*CS=*/15, /*IRQ=*/27);
TaskHandle_t watchPageTaskHandle;
TaskHandle_t telemetryPageTaskHandle;
TaskHandle_t screensaverPageTaskHandle;
TaskHandle_t healthPageTaskHandle;

SEN0502 rotaryEncoder(33, 32);  // DFRobot Rotary Encoder connected to pins 33 and 32

// Forward declarations for helper initializers (from original sketch)
static void initMAX30102();
static void initMLX90614();
static void initTEMT6000();
static void initDHT22();
static void initQMI8658();

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

  // Initialize the display (Adafruit_ILI9341 using GFX library)
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
  initMAX30102();   // Heart rate / SpO2
  initMLX90614();   // IR temperature
  initTEMT6000();   // Ambient light
  initDHT22();      // Temp/Humidity
  initQMI8658();    // IMU (accelerometer/gyro)

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

// ---------------------- Sensor init helpers (from original sketch) ------------------

static void initMAX30102() {
  // Initialize MAX30102 heart rate & SpO2 sensor
  // max30102.begin(); // Use your library’s init
}

static void initMLX90614() {
  // Initialize MLX90614 IR temperature sensor
  // mlx.begin();
}

static void initTEMT6000() {
  // Initialize TEMT6000 ambient light sensor (if needed)
}

void initDHT22() {
  // Initialize DHT22 sensor for temperature and humidity
  // dht.begin();
}

void initQMI8658() {
  // Initialize QMI8658 accelerometer
  // if (!qmi8658.begin()) {
  //   Serial.println("QMI8658 initialization failed!");
  //   while (1) { delay(10); }
  // }
}

// ---------------------- Touch helpers (from original sketch) ------------------------

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

// ---------------------- Display helpers (from original sketch) ----------------------

// Initialize the display (Adafruit ILI9341)
void initDisplay() {
  tft.begin();
  tft.setRotation(1); // Landscape orientation
  tft.fillScreen(ILI9341_BLACK); // Clear the display to black
}
