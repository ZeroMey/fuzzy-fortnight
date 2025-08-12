#include <Arduino.h>
#include <Wire.h>
#include "pin_config.h"
#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"
#include "lvgl.h"

#include "page1.h"
#include "page2.h"
#include "page3.h"

// Display
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST, 0, true, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);

// Touch (CST816)
std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus = std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);
void tp_irq_cb(void);
std::unique_ptr<Arduino_IIC> CST816T(new Arduino_CST816x(IIC_Bus, CST816T_DEVICE_ADDRESS, TP_RST, TP_INT, tp_irq_cb));
void tp_irq_cb(void) { CST816T->IIC_Interrupt_Flag = true; }

// LVGL display buffer/driver
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[LCD_WIDTH * 40];

static void disp_flush(lv_disp_drv_t *drv, const lv_area_t *a, lv_color_t *px) {
  int w = a->x2 - a->x1 + 1, h = a->y2 - a->y1 + 1;
  gfx->draw16bitRGBBitmap(a->x1, a->y1, (uint16_t *)px, w, h);
  lv_disp_flush_ready(drv);
}

static void indev_read(lv_indev_drv_t *, lv_indev_data_t *data) {
  int32_t fingers = CST816T->IIC_Read_Device_Value(
      CST816T->Arduino_IIC_Touch::Value_Information::TOUCH_FINGER_NUMBER);
  int32_t x = CST816T->IIC_Read_Device_Value(
      CST816T->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
  int32_t y = CST816T->IIC_Read_Device_Value(
      CST816T->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);

  bool pressed = (fingers > 0) || (x >= 0 && y >= 0);
  if (pressed) {
    data->point.x = (lv_coord_t)constrain(x, 0, (int)LCD_WIDTH  - 1);
    data->point.y = (lv_coord_t)constrain(y, 0, (int)LCD_HEIGHT - 1);
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

static lv_obj_t *pages;

void setup() {
  // Backlight 50%
  ledcAttach(LCD_BL, 20000, 12);
  ledcWrite(LCD_BL, 2048);

  Wire.begin(IIC_SDA, IIC_SCL);
  gfx->begin();

  while (!CST816T->begin()) delay(200);
  CST816T->IIC_Write_Device_State(
      CST816T->Arduino_IIC_Touch::Device::TOUCH_DEVICE_INTERRUPT_MODE,
      CST816T->Arduino_IIC_Touch::Device_Mode::TOUCH_DEVICE_INTERRUPT_PERIODIC);

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf1, NULL, sizeof(buf1)/sizeof(buf1[0]));
  lv_disp_drv_t dd; lv_disp_drv_init(&dd);
  dd.hor_res = LCD_WIDTH; dd.ver_res = LCD_HEIGHT;
  dd.flush_cb = disp_flush; dd.draw_buf = &draw_buf;
  lv_disp_drv_register(&dd);

  lv_indev_drv_t id; lv_indev_drv_init(&id);
  id.type = LV_INDEV_TYPE_POINTER; id.read_cb = indev_read;
  lv_indev_drv_register(&id);

  // Horizontal page container
  pages = lv_obj_create(lv_scr_act());
  lv_obj_set_size(pages, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_pad_all(pages, 0, 0);
  lv_obj_clear_flag(pages, LV_OBJ_FLAG_SCROLL_ELASTIC);
  lv_obj_set_scroll_dir(pages, LV_DIR_HOR);
  lv_obj_set_scroll_snap_x(pages, LV_SCROLL_SNAP_CENTER);
  lv_obj_set_flex_flow(pages, LV_FLEX_FLOW_ROW);

  // Add pages
  page1_create(pages);
  page2_create(pages);
  page3_create(pages);
}

void loop() {
  static uint32_t last = millis();
  uint32_t now = millis();
  lv_tick_inc(now - last);
  last = now;
  lv_timer_handler();
  delay(5);
}
