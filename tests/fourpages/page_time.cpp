#include <Arduino.h>
#include <cstdio>
//#include <stdio.h>
#include "device_manager.h"
#include "page_time.h"


static lv_obj_t* s_time_label = nullptr;

static void style_page(lv_obj_t* page) {
  lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
  lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(page, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(page, 0, 0);
  lv_obj_set_style_radius(page, 0, 0);
  lv_obj_set_style_pad_all(page, 0, 0);
}

lv_obj_t* page_time_create(lv_obj_t* parent, lv_obj_t** out_label) {
  lv_obj_t* page = lv_obj_create(parent);
  style_page(page);

  lv_obj_t* title = lv_label_create(page);
  lv_label_set_text(title, "Page two");
  lv_obj_set_style_text_color(title, lv_color_white(), 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_12, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 6);

  lv_obj_t* lab = lv_label_create(page);
  lv_obj_set_style_text_color(lab, lv_color_white(), 0);
  lv_obj_set_style_text_font(lab, &lv_font_montserrat_30, 0);
  lv_obj_center(lab);
  lv_label_set_text(lab, "--:--:--");

  s_time_label = lab;
  if (out_label) *out_label = lab;
  return page;
}

void page_time_update(const SharedData& snap) {
  if (!s_time_label || !snap.rtc_valid) return;
  char buf[16];
  std::snprintf(buf, sizeof(buf), "%02u:%02u:%02u", snap.hh, snap.mm, snap.ss);
  lv_label_set_text(s_time_label, buf);
}
