#include <Arduino.h>
#include <lvgl.h>

#include "display_manager.h"
#include "touchscreen.h"
#include "power_manager.h"
#include "watch_page.h"
#include "telemetry_page.h"
#include "health_page.h"
#include "screensaver_page.h"
#include "utils.h"

void app_setup() {
  Serial.begin(115200);
  delay(200);

  lv_init();

  initDisplay();
  initTouchscreen();
  initPowerManager();

  createWatchPage();
  createTelemetryPage();
  createHealthPage();
  createScreensaverPage();
}

void app_loop() {
  handleTouchInput();

  updateWatchPage();
  updateTelemetryPage();
  updateHealthPage();
  updateScreensaverPage();

  updateDisplay();
  lv_timer_handler();
  delay(5);
}
