#include <Arduino.h>
#include "ui.h"
#include "device_manager.h"
#include "page_time.h"
#include "page_gyro.h"

// Define the UI objects declared in ui.h
lv_obj_t *pages      = nullptr;
lv_obj_t *watch_label = nullptr;
lv_obj_t *time_label  = nullptr;
lv_obj_t *gyro_label  = nullptr;

extern SemaphoreHandle_t i2cMutex;
extern SemaphoreHandle_t dataMutex;

SharedData gData = {};   // zero-init

// --- Build 3 swipeable pages ---
void build_ui(void) {
  lv_obj_t *scr = lv_scr_act();
  lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

  pages = lv_obj_create(scr);
  lv_obj_set_size(pages, LV_PCT(100), LV_PCT(100));
  lv_obj_set_flex_flow(pages, LV_FLEX_FLOW_ROW);
  lv_obj_set_scroll_dir(pages, LV_DIR_HOR);
  lv_obj_set_scroll_snap_x(pages, LV_SCROLL_SNAP_START);
  lv_obj_clear_flag(pages, LV_OBJ_FLAG_SCROLL_ELASTIC);
  lv_obj_add_flag(pages, LV_OBJ_FLAG_SCROLL_ONE);
  lv_obj_set_scrollbar_mode(pages, LV_SCROLLBAR_MODE_OFF);

  // seamless look
  lv_obj_set_style_bg_color(pages, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(pages, LV_OPA_COVER, 0);
  lv_obj_set_style_pad_all(pages, 0, 0);
  lv_obj_set_style_pad_row(pages, 0, 0);
  lv_obj_set_style_pad_column(pages, 0, 0);
  lv_obj_set_style_border_width(pages, 0, 0);
  lv_obj_set_style_radius(pages, 0, 0);

  // Build pages via modules; capture their central labels
  (void)page_watch_create(pages, &watch_label);
  (void)page_time_create (pages, &time_label);
  (void)page_gyro_create (pages, &gyro_label);

  lv_obj_scroll_to_view(lv_obj_get_child(pages, 0), LV_ANIM_OFF);
}

void ui_timer_cb(lv_timer_t *t) {
  SharedData snap;
  if (xSemaphoreTake(dataMutex, 0) == pdTRUE) {
    snap = gData;
    xSemaphoreGive(dataMutex);
  } else {
    return; // skip this frame if data is busy
  }
  /*
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
  }*/
  page_time_update(snap);
  page_gyro_update(snap);
}