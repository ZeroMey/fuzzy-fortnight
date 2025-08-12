#include "page1.h"
#include "pin_config.h"

lv_obj_t* page1_create(lv_obj_t* parent) {
  lv_obj_t* p = lv_obj_create(parent);
  lv_obj_set_size(p, LCD_WIDTH, LCD_HEIGHT);
  lv_obj_clear_flag(p, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_t* l = lv_label_create(p);
  lv_label_set_text(l, "My Label");
  lv_obj_center(l);
  return p;
}
