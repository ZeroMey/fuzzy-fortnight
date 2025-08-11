#include "display_manager.h"
#include <Adafruit_GFX.h>        // GFX library for display support
#include <Adafruit_ST7789.h>     // ST7789V2 display driver
#include <lvgl.h>                // LVGL library for UI management
#include "Arduino_GFX_Library.h"

// Global display object
extern Adafruit_ST7789 tft;   // Create display object for ST7789 display

// Function to initialize the display and prepare LVGL
void initDisplay() {
    // Initialize the TFT display
    tft.begin();
    tft.setRotation(3);  // Set the display rotation (adjust as needed)
    tft.fillScreen(ST77XX_BLACK);  // Clear the display to black

    // Initialize LVGL display buffer
    static lv_disp_buf_t disp_buf;
    static lv_color_t buf[LV_HOR_RES_MAX * 10];  // Buffer for the display (adjust if needed)
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

    // Register the display with LVGL
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = my_disp_flush;  // Use the custom flush callback
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);
}

// Function to handle display flushing (called by LVGL)
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    // This function sends the buffer data to the TFT display
    tft.pushRect(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (uint16_t *)color_p);
    lv_disp_flush_ready(disp_drv);  // Notify LVGL that the flush is done
}

// Function to clear the screen
void clearDisplay() {
    tft.fillScreen(ST77XX_BLACK);  // Clear the screen to black
}

// Function to update the display (called by LVGL)
void updateDisplay() {
    lv_task_handler();  // Handle LVGL tasks (UI updates, animations, etc.)
}

// Function to transition to a new page (screen transition)
void transitionToPage(lv_obj_t *new_page) {
    lv_scr_load_anim(new_page, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, true);  // Fade transition to the new page
}

// Function to load a page with a fade-in transition
void loadPageWithFade(lv_obj_t *new_page) {
    lv_scr_load_anim(new_page, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, true);  // Fade transition to the new page
}

// Function to load a page with a slide-in transition
void loadPageWithSlide(lv_obj_t *new_page) {
    lv_scr_load_anim(new_page, LV_SCR_LOAD_ANIM_SLIDE_RIGHT, 500, 0, true);  // Slide transition
}

// Function to navigate back to the previous page with a fade-out transition
void backToPreviousPage() {
    lv_scr_load_anim(lv_scr_act(), LV_SCR_LOAD_ANIM_FADE_OFF, 300, 0, true);  // Fade-out transition
}

// Function to handle page transitions (custom logic if needed)
void handlePageTransition() {
    // This function can handle any custom transition logic, e.g., checking conditions
    // and selecting the next page or animation type.
}

// Function to refresh the display after any updates (handles LVGL tasks)
void refreshDisplay() {
    lv_task_handler();  // Handle LVGL tasks
    delay(5);  // Short delay for task switching and responsiveness
}
