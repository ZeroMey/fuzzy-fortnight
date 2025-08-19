#include <Arduino.h>
#include "lv_port.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"

// --- module-local state ---
static lv_disp_draw_buf_t s_draw_buf;
static lv_color_t *s_buf1 = nullptr, *s_buf2 = nullptr;
static lv_disp_t  *s_disp  = nullptr;
static lv_indev_t *s_indev = nullptr;
static esp_timer_handle_t s_tick_timer = nullptr;
static uint32_t s_tick_ms = 2;

static void tick_cb(void *) { lv_tick_inc(s_tick_ms); }

void lv_port_init(
    uint16_t hor_res,
    uint16_t ver_res,
    void (*flush_cb)(lv_disp_drv_t*, const lv_area_t*, lv_color_t*),
    void (*indev_read_cb)(lv_indev_drv_t*, lv_indev_data_t*),
    uint32_t tick_period_ms)
{
  s_tick_ms = tick_period_ms;

  lv_init();

  const size_t pixels = (size_t)hor_res * ver_res;
  s_buf1 = (lv_color_t *)heap_caps_malloc(pixels * sizeof(lv_color_t), MALLOC_CAP_DMA);
  s_buf2 = (lv_color_t *)heap_caps_malloc(pixels * sizeof(lv_color_t), MALLOC_CAP_DMA);
  lv_disp_draw_buf_init(&s_draw_buf, s_buf1, s_buf2, pixels);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res  = hor_res;
  disp_drv.ver_res  = ver_res;
  disp_drv.flush_cb = flush_cb;
  disp_drv.draw_buf = &s_draw_buf;
  s_disp = lv_disp_drv_register(&disp_drv);

  if (indev_read_cb) {
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type    = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = indev_read_cb;
    s_indev = lv_indev_drv_register(&indev_drv);
  }

  const esp_timer_create_args_t args = {
    .callback = &tick_cb,
    .arg = nullptr,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "lvgl_tick"
  };
  esp_timer_create(&args, &s_tick_timer);
  esp_timer_start_periodic(s_tick_timer, s_tick_ms * 1000ULL);
}

lv_disp_t*  lv_port_disp()  { return s_disp; }
lv_indev_t* lv_port_indev() { return s_indev; }

void lvglTask(void *arg) {
  const TickType_t period = pdMS_TO_TICKS(5); // ~200 Hz
  TickType_t last = xTaskGetTickCount();
  for (;;) {
    lv_timer_handler();
    vTaskDelayUntil(&last, period);
  }
}
