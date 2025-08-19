#pragma once
#include <lvgl.h>

// Creates Page 1 under `parent`. Returns the page object and sets *out_label to the centered big label.
lv_obj_t* page_watch_create(lv_obj_t* parent, lv_obj_t** out_label);
