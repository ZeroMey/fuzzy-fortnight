#ifndef SCREENSAVER_PAGE_H
#define SCREENSAVER_PAGE_H

#include <lvgl.h>

void createScreensaverPage();      // Setup screensaver page layout
void updateScreensaverPage();      // Update content or handle events
void activateScreensaverPage();        // Activate screensaver (e.g., idle mode)

#endif
