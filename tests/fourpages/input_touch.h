#pragma once
#include <lvgl.h>

// Initialize the CST816T touch controller and enable its interrupt mode.
// Safe to call multiple times; returns only when the device is ready.
void touch_init();

// LVGL input-device read callback to pass into lv_port_init(...)
void my_touchpad_read(lv_indev_drv_t *indev, lv_indev_data_t *data);
