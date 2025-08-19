#include <lvgl.h>
#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"
#include "pin_config.h"
#include "lv_conf.h"
#include "HWCDC.h"

#include "ui.h"
#include "device_manager.h"
#include "lv_port.h"
#include "input_touch.h"

HWCDC USBSerial;

// --- FreeRTOS sync ---
SemaphoreHandle_t i2cMutex;
SemaphoreHandle_t dataMutex;

extern void rtcTask(void *arg);
extern void imuTask(void *arg);

Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);

Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST /* RST */,
                                      0 /* rotation */, true /* IPS */, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);

#define EXAMPLE_LVGL_TICK_PERIOD_MS 2

uint32_t screenWidth;
uint32_t screenHeight;

static lv_disp_draw_buf_t draw_buf;
// static lv_color_t buf[screenWidth * screenHeight / 10];

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char *buf) {
  USBSerial.printf("%s", buf);
  USBSerial.flush();
}
#endif

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif

  lv_disp_flush_ready(disp);
}

void example_increase_lvgl_tick(void *arg) {
  /* Tell LVGL how many milliseconds has elapsed */
  lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

static uint8_t count = 0;
void example_increase_reboot(void *arg) {
  count++;
  if (count == 30) {
    esp_restart();
  }
}
/*
static void ui_timer_cb(lv_timer_t *t) {
  SharedData snap;
  if (xSemaphoreTake(dataMutex, 0) == pdTRUE) {
    snap = gData;
    xSemaphoreGive(dataMutex);
  } else {
    return; // skip this frame if data is busy
  }

  if (time_label && snap.rtc_valid) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%02u:%02u:%02u", snap.hh, snap.mm, snap.ss);
    lv_label_set_text(time_label, buf);
  }

  if (gyro_label && snap.imu_valid) {
    char gbuf[48];
    // show one decimal
    snprintf(gbuf, sizeof(gbuf), "gx=%.1f dps\ngy=%.1f dps\ngz=%.1f dps", snap.gx, snap.gy, snap.gz);
    lv_label_set_text(gyro_label, gbuf);
  }
}
*/
/*
void lvglTask(void *arg) {
  const TickType_t period = pdMS_TO_TICKS(5); // ~200 Hz
  TickType_t last = xTaskGetTickCount();
  for (;;) {
    lv_timer_handler();
    vTaskDelayUntil(&last, period);
  }
}
*/

void setup() {
  USBSerial.begin(115200);

  ledcAttach(LCD_BL, 500, 8);         // 500Hz, 8-bit
  ledcWrite(LCD_BL, 32);              // 12.5% duty

  gfx->begin();

  //pinMode(LCD_BL, OUTPUT);
  //digitalWrite(LCD_BL, HIGH);

  screenWidth = gfx->width();
  screenHeight = gfx->height();

  touch_init();  // must be before lv_port_init so indev is ready
  lv_port_init(screenWidth, screenHeight, my_disp_flush, my_touchpad_read, EXAMPLE_LVGL_TICK_PERIOD_MS);

  String LVGL_Arduino = "Hello Arduino! ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

  USBSerial.println(LVGL_Arduino);
  USBSerial.println("I am LVGL_Arduino");

#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

  restore_last_epoch();

  // UI
  build_ui();

  // Create mutexes
  i2cMutex  = xSemaphoreCreateMutex();
  dataMutex = xSemaphoreCreateMutex();

  // UI refresh every 200 ms
  lv_timer_create(ui_timer_cb, 200, nullptr);

  // After timers are started and UI built:
  xTaskCreatePinnedToCore(lvglTask, "lvgl", 6144, nullptr, 5, nullptr, 1); // core 1
  xTaskCreatePinnedToCore(rtcTask, "rtcTask", 4096, nullptr, 3, nullptr, 0);
  xTaskCreatePinnedToCore(imuTask, "imuTask", 4096, nullptr, 3, nullptr, 0);

  for (int i = 0; i < 75; i++) {
    rtc_sync_over_serial(100);
    delay(100);
  }
  rtc_init_if_needed();

}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}