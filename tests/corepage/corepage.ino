#include <lvgl.h>
#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"
#include "pin_config.h"
#include "lv_conf.h"
//#include <demos/lv_demos.h>
#include "HWCDC.h"
//#include "image.h"
#include "SensorQMI8658.hpp"

HWCDC USBSerial;

// --- Shared UI objects ---
static lv_obj_t *pages;        // global, not re-declared in build_ui()
static lv_obj_t *watch_label;  // on Page 1
static lv_obj_t *time_label;   // on Page 2
static lv_obj_t *gyro_label;   // on Page 3

// --- FreeRTOS sync ---
static SemaphoreHandle_t i2cMutex;
static SemaphoreHandle_t dataMutex;

struct SharedData {
  uint8_t hh, mm, ss;
  float gx, gy, gz;
  bool rtc_valid;
  bool imu_valid;
};
static SharedData gData = {};   // zero-init

// Device I2C addresses (adjust if your board differs)
#define PCF85063_ADDR 0x51
#define QMI8658_ADDR  0x6B  // some boards use 0x6A

extern "C" void rtcTask(void *arg);
extern "C" void imuTask(void *arg);

Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);

Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST /* RST */,
                                      0 /* rotation */, true /* IPS */, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);


std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
  std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

void Arduino_IIC_Touch_Interrupt(void);

std::unique_ptr<Arduino_IIC> CST816T(new Arduino_CST816x(IIC_Bus, CST816T_DEVICE_ADDRESS,
                                                         TP_RST, TP_INT, Arduino_IIC_Touch_Interrupt));

void Arduino_IIC_Touch_Interrupt(void) {
  CST816T->IIC_Interrupt_Flag = true;
}

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

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  int32_t touchX = CST816T->IIC_Read_Device_Value(CST816T->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
  int32_t touchY = CST816T->IIC_Read_Device_Value(CST816T->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);

  if (CST816T->IIC_Interrupt_Flag == true) {
    CST816T->IIC_Interrupt_Flag = false;
    data->state = LV_INDEV_STATE_PR;

    /* Set the coordinates with some debounce */
    if (touchX >= 0 && touchY >= 0) {
      data->point.x = touchX;
      data->point.y = touchY;

      USBSerial.printf("Data x: %d, Data y: %d\n", touchX, touchY);
    }
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}

// --- Build 3 swipeable pages ---
static void build_ui(void) {
  lv_obj_t *scr = lv_scr_act();
  lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

  pages = lv_obj_create(scr); 
  lv_obj_set_size(pages, LV_PCT(100), LV_PCT(100));
  lv_obj_set_flex_flow(pages, LV_FLEX_FLOW_ROW);
  lv_obj_set_scroll_dir(pages, LV_DIR_HOR);
  lv_obj_set_scroll_snap_x(pages, LV_SCROLL_SNAP_START);      // page-aligned
  lv_obj_clear_flag(pages, LV_OBJ_FLAG_SCROLL_ELASTIC);
  lv_obj_add_flag(pages, LV_OBJ_FLAG_SCROLL_ONE);             // <= one page per swipe
  lv_obj_set_scrollbar_mode(pages, LV_SCROLLBAR_MODE_OFF);

  /* Remove any visual seams */
  lv_obj_set_style_bg_color(pages, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(pages, LV_OPA_COVER, 0);
  lv_obj_set_style_pad_all(pages, 0, 0);
  lv_obj_set_style_pad_row(pages, 0, 0);
  lv_obj_set_style_pad_column(pages, 0, 0);
  lv_obj_set_style_border_width(pages, 0, 0);
  lv_obj_set_style_radius(pages, 0, 0);

  const char *labels[3] = {"Page one", "Page two", "Page three"};
  for (int i = 0; i < 3; ++i) {
    lv_obj_t *page = lv_obj_create(pages);
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));          // exact width match
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

    /* Page visuals: solid black, no seams */
    lv_obj_set_style_bg_color(page, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_radius(page, 0, 0);
    lv_obj_set_style_pad_all(page, 0, 0);

    lv_obj_t *lab = lv_label_create(page);
    lv_label_set_text(lab, labels[i]);
    lv_obj_set_style_text_color(lab, lv_color_white(), 0);
    lv_obj_set_style_text_font(lab, &lv_font_montserrat_12, 0);
    //lv_obj_center(lab);
    lv_obj_align(lab, LV_ALIGN_TOP_MID, 0, 6);

    if (i == 0) {
      watch_label = lv_label_create(page);
      lv_obj_set_style_text_color(watch_label, lv_color_white(), 0);
      lv_obj_set_style_text_font(watch_label, &lv_font_montserrat_40, 0);
      lv_obj_center(watch_label);
      lv_label_set_text(watch_label, "WTCH");
    } else if (i == 1) {
      time_label = lv_label_create(page);
      lv_obj_set_style_text_color(time_label, lv_color_white(), 0);
      lv_obj_set_style_text_font(time_label, &lv_font_montserrat_30, 0);
      lv_obj_center(time_label);
      lv_label_set_text(time_label, "--:--:--");
    } else if (i == 2) {
      gyro_label = lv_label_create(page);
      lv_obj_set_style_text_color(gyro_label, lv_color_white(), 0);
      lv_obj_set_style_text_font(gyro_label, &lv_font_montserrat_20, 0);
      lv_obj_center(gyro_label);
      lv_label_set_text(gyro_label, "gx=-- dps\ngy=-- dps\ngz=-- dps");
    }
  }

  /* Ensure we start perfectly aligned to page 0 */
  lv_obj_scroll_to_view(lv_obj_get_child(pages, 0), LV_ANIM_OFF);
}

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

static inline uint8_t bcd2bin(uint8_t v) { return (v & 0x0F) + 10 * (v >> 4); }

void read_pcf85063_time(uint8_t &hh, uint8_t &mm, uint8_t &ss, bool &ok) {
  ok = false;
  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) != pdTRUE) return;

  Wire.beginTransmission(PCF85063_ADDR);
  Wire.write(0x04); // seconds register
  if (Wire.endTransmission(false) != 0) { xSemaphoreGive(i2cMutex); return; }

  if (Wire.requestFrom(PCF85063_ADDR, 7) != 7) { xSemaphoreGive(i2cMutex); return; }

  uint8_t sec = Wire.read();
  uint8_t min = Wire.read();
  uint8_t hour = Wire.read();
  Wire.read(); // day
  Wire.read(); // weekday
  Wire.read(); // month
  Wire.read(); // year

  xSemaphoreGive(i2cMutex);

  // VL flag in seconds MSB indicates low voltage/lost clock. Mask bit7.
  ss = bcd2bin(sec & 0x7F);
  mm = bcd2bin(min & 0x7F);
  hh = bcd2bin(hour & 0x3F);
  ok = true;
}

void rtcTask(void *arg) {
  // Optional: configure PCF85063 if needed (24h, etc.)
  const TickType_t period = pdMS_TO_TICKS(500);
  TickType_t last = xTaskGetTickCount();

  for (;;) {
    uint8_t hh, mm, ss; bool ok;
    read_pcf85063_time(hh, mm, ss, ok);

    if (ok && xSemaphoreTake(dataMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
      gData.hh = hh; gData.mm = mm; gData.ss = ss; gData.rtc_valid = true;
      xSemaphoreGive(dataMutex);
    }
    vTaskDelayUntil(&last, period);
  }
}

// If you have a library:
// #include "QMI8658.h"
SensorQMI8658 qmi;
IMUdata acc;

bool imu_begin() {
  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(200)) != pdTRUE) return false;

  // Use the address you defined (0x6B); avoids mismatch with *_L_* constants
  bool ok = qmi.begin(Wire, QMI8658_ADDR, IIC_SDA, IIC_SCL);
  if (ok) {
    qmi.configGyroscope(
      SensorQMI8658::GYR_RANGE_64DPS,
      SensorQMI8658::GYR_ODR_896_8Hz,
      SensorQMI8658::LPF_MODE_3,
      true);
    qmi.enableGyroscope();

    // Many QMI8658 libs assert DRDY only when BOTH accel+gyro are enabled.
    qmi.enableAccelerometer();

    qmi.dumpCtrlRegister();
  }
  xSemaphoreGive(i2cMutex);

  if (!ok) USBSerial.println("QMI8658 init failed (addr/wiring).");
  return ok;
}

bool imu_read_gyro(float &gx, float &gy, float &gz) {
  if (!qmi.getDataReady()) return false;      // no new sample yet
  return qmi.getGyroscope(gx, gy, gz);        // true on success
}

void imuTask(void *arg) {
  (void)arg;

  // Initialize device once
  if (!imu_begin()) {
    // Suspend this task if init fails to avoid busy looping
    vTaskSuspend(nullptr);
  }

  // Poll fast enough relative to ODR; UI will downsample in its timer
  const TickType_t period = pdMS_TO_TICKS(10);  // ~100 Hz
  TickType_t last = xTaskGetTickCount();

  for (;;) {
    float gx, gy, gz;
    bool ok = false;

    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
      ok = imu_read_gyro(gx, gy, gz);
      xSemaphoreGive(i2cMutex);
    }

    if (ok && xSemaphoreTake(dataMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
      gData.gx = gx;
      gData.gy = gy;
      gData.gz = gz;
      gData.imu_valid = true;
      xSemaphoreGive(dataMutex);
    }

    vTaskDelayUntil(&last, period);
  }
}

void setup() {
  USBSerial.begin(115200); /* prepare for possible serial debug */

  while (CST816T->begin() == false) {
    USBSerial.println("CST816T initialization fail");
    delay(2000);
  }
  USBSerial.println("CST816T initialization successfully");

  CST816T->IIC_Write_Device_State(CST816T->Arduino_IIC_Touch::Device::TOUCH_DEVICE_INTERRUPT_MODE,
                                  CST816T->Arduino_IIC_Touch::Device_Mode::TOUCH_DEVICE_INTERRUPT_PERIODIC);

  ledcAttach(LCD_BL, 500, 8);         // 500Hz, 8-bit
  ledcWrite(LCD_BL, 32);              // 12.5% duty

  gfx->begin();

  //pinMode(LCD_BL, OUTPUT);
  //digitalWrite(LCD_BL, HIGH);

  screenWidth = gfx->width();
  screenHeight = gfx->height();

  lv_init();

  lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(screenWidth * screenHeight / 1 * sizeof(lv_color_t), MALLOC_CAP_DMA);

  lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(screenWidth * screenHeight / 1 * sizeof(lv_color_t), MALLOC_CAP_DMA);

  String LVGL_Arduino = "Hello Arduino! ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

  USBSerial.println(LVGL_Arduino);
  USBSerial.println("I am LVGL_Arduino");

#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, screenWidth * screenHeight / 1);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the (dummy) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  lv_obj_t *label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, "Hello Ardino and LVGL!");
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

  const esp_timer_create_args_t lvgl_tick_timer_args = {
    .callback = &example_increase_lvgl_tick,
    .name = "lvgl_tick"
  };

  const esp_timer_create_args_t reboot_timer_args = {
    .callback = &example_increase_reboot,
    .name = "reboot"
  };

  esp_timer_handle_t lvgl_tick_timer = NULL;
  esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
  esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000);

  // UI
  build_ui();

  // Create mutexes
  i2cMutex  = xSemaphoreCreateMutex();
  dataMutex = xSemaphoreCreateMutex();

  // UI refresh every 200 ms
  lv_timer_create(ui_timer_cb, 200, nullptr);

  // After timers are started and UI built:
  xTaskCreatePinnedToCore(rtcTask, "rtcTask", 4096, nullptr, 3, nullptr, 0);
  xTaskCreatePinnedToCore(imuTask, "imuTask", 4096, nullptr, 3, nullptr, 0);

}

void loop() {
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);
}
