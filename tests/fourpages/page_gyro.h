#pragma once
#include <lvgl.h>

struct SharedData;

// Creates Page 3 under `parent`. Returns the page object and sets *out_label to the gyro text label.
lv_obj_t* page_gyro_create(lv_obj_t* parent, lv_obj_t** out_label);
void page_gyro_update(const SharedData& snap);
