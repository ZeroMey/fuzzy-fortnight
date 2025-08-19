#pragma once
#include <stdint.h>
#include <lvgl.h>

// Initialize LVGL: alloc double buffers, register display (required) and touch (optional),
// and start a periodic lv_tick_inc.
// Signatures use explicit function pointers to avoid typedef/version issues.
void lv_port_init(
    uint16_t hor_res,
    uint16_t ver_res,
    void (*flush_cb)(lv_disp_drv_t*, const lv_area_t*, lv_color_t*),
    void (*indev_read_cb)(lv_indev_drv_t*, lv_indev_data_t*) = nullptr,
    uint32_t tick_period_ms = 2);

lv_disp_t*  lv_port_disp();
lv_indev_t* lv_port_indev();
void lvglTask(void *arg);
