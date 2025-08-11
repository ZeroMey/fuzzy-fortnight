#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <lvgl.h>

void initDisplay();              // Initialize display (TFT/OLED)
void clearDisplay();             // Clear the display
void updateDisplay();            // Refresh the display with updated content
void transitionToPage(lv_obj_t* page);  // Transition smoothly between pages

#endif
