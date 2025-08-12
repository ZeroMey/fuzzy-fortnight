#include <lvgl.h>
//#include <Arduino.h>
//#include <memory>
#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"
#include "pin_config.h"
#include "lv_conf.h"
#include <demos/lv_demos.h>
#include "image.h"

// --- Display (ST7789 240x280, 20px top offset set here) ---
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST, 0, true, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);

// --- Touch (CST816x over I2C via Arduino_DriveBus) ---
/*std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus = std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);
void Arduino_IIC_Touch_Interrupt(void);
std::unique_ptr<Arduino_IIC> CST816T(new Arduino_CST816x(IIC_Bus, CST816T_DEVICE_ADDRESS, TP_RST, TP_INT, Arduino_IIC_Touch_Interrupt));
void Arduino_IIC_Touch_Interrupt(void) { CST816T->IIC_Interrupt_Flag = true; }
*/

// --- LVGL draw buffer/driver ---
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[LCD_WIDTH * LCD_HEIGHT / 4]; // ~19 KB

static void my_disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
  int w = area->x2 - area->x1 + 1;
  int h = area->y2 - area->y1 + 1;
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)color_p->full, w, h);
  lv_disp_flush_ready(drv);
}

/*static void touch_read(lv_indev_drv_t *indev, lv_indev_data_t *data) {
  // Use finger count if available; fall back to coords >=0
  int32_t fingers = CST816T->IIC_Read_Device_Value(
      CST816T->Arduino_IIC_Touch::Value_Information::TOUCH_FINGER_NUMBER);
  int32_t x = CST816T->IIC_Read_Device_Value(
      CST816T->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
  int32_t y = CST816T->IIC_Read_Device_Value(
      CST816T->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);

  bool pressed = (fingers > 0) || (x >= 0 && y >= 0);
  if (pressed) {
    x = constrain(x, 0, (int)LCD_WIDTH  - 1);
    y = constrain(y, 0, (int)LCD_HEIGHT - 1);
    data->point.x = (lv_coord_t)x;
    data->point.y = (lv_coord_t)y;
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}
*/

// --- Build 3 swipeable pages ---
static lv_obj_t *pages;

static void build_ui() {
  pages = lv_obj_create(lv_scr_act());
  lv_obj_set_size(pages, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_pad_all(pages, 0, 0);
  lv_obj_set_style_border_width(pages, 0, 0);
  lv_obj_clear_flag(pages, LV_OBJ_FLAG_SCROLL_ELASTIC);
  lv_obj_set_scroll_dir(pages, LV_DIR_HOR);
  lv_obj_set_scroll_snap_x(pages, LV_SCROLL_SNAP_CENTER);
  lv_obj_set_flex_flow(pages, LV_FLEX_FLOW_ROW);

  const char *labels[3] = {"Page one", "Page two", "Page three"};
  for (int i = 0; i < 3; ++i) {
    lv_obj_t *page = lv_obj_create(pages);
    lv_obj_set_size(page, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_set_style_pad_all(page, 0, 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *lab = lv_label_create(page);
    lv_label_set_text(lab, labels[i]);
    lv_obj_set_style_text_color(lab, lv_color_white(), 0);
    lv_obj_center(lab);
  }
}

void setup() {
  /*while (!CST816T->begin()) delay(200);
  CST816T->IIC_Write_Device_State(
      CST816T->Arduino_IIC_Touch::Device::TOUCH_DEVICE_INTERRUPT_MODE,
      CST816T->Arduino_IIC_Touch::Device_Mode::TOUCH_DEVICE_INTERRUPT_PERIODIC);
      */

  gfx->begin();
  delay(200);
  gfx->fillScreen(BLACK);
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);

  //pinMode(TP_RST, OUTPUT);
  //digitalWrite(TP_RST, LOW);      // keep touch controller in reset during debug
  //pinMode(TP_INT, INPUT_PULLUP);  // don't let INT float


  // LVGL init
  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf1, NULL, LCD_WIDTH * LCD_HEIGHT / 4);
  lv_disp_drv_t disp_drv; 
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = LCD_WIDTH;
  disp_drv.ver_res = LCD_HEIGHT;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
  
  /*lv_indev_drv_t indev_drv; lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touch_read;
  lv_indev_drv_register(&indev_drv);
*/

  lv_obj_t *label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, "Hello Ardino and LVGL!");
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

  // UI
  //build_ui();
}

void loop() {
  static uint32_t last = millis();
  uint32_t now = millis();
  lv_tick_inc(now - last);
  last = now;
  lv_timer_handler();
  delay(5);
}
