#pragma once
#include "lvgl.h"
#include "page_watch.h"
#include "page_time.h"
#include "page_gyro.h"


// Exposed UI objects (built by build_ui)
extern lv_obj_t *pages;        // container with 3 pages
extern lv_obj_t *watch_label;  // Page 1
extern lv_obj_t *time_label;   // Page 2
extern lv_obj_t *gyro_label;   // Page 3

void build_ui(void);
void ui_timer_cb(lv_timer_t *t);
