#pragma once
#include <lvgl.h>

struct SharedData;

// Creates Page 2 under `parent`. Returns the page object and sets *out_label to the time label.
lv_obj_t* page_time_create(lv_obj_t* parent, lv_obj_t** out_label);
void page_time_update(const SharedData& snap);
