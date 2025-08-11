#include "screensaver_page.h"
#include <Adafruit_GFX.h>        // GFX library for display support
#include <Adafruit_ST7789.h>     // ST7789V2 display driver
#include <Arduino_TouchBus.h>    // Touch input for CST816T

// Global display object
extern Adafruit_ST7789 tft;   // Create display object for ST7789 display
extern Arduino_TouchBus touchBus;  // CST816T touch input object

// Global page object and label variables
lv_obj_t *screensaver_page;       // Current screensaver page object
lv_obj_t *time_label;             // Time label
lv_obj_t *status_label;           // Status label

// Function to create the Screensaver page UI elements
void createScreensaverPage() {
    // Create the screensaver page object
    screensaver_page = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_bg_color(screensaver_page, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);  // Set background color

    // Create label for time
    time_label = lv_label_create(screensaver_page, NULL);
    lv_label_set_text(time_label, "00:00:00");  // Initial placeholder time
    lv_obj_align(time_label, NULL, LV_ALIGN_CENTER, 0, -30);  // Center the label on the screen

    // Create label for status message (e.g., idle message)
    status_label = lv_label_create(screensaver_page, NULL);
    lv_label_set_text(status_label, "Swipe right to go to Health\nSwipe left to go to Watch");  // Initial message
    lv_obj_align(status_label, time_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  // Position below the time label
}

// Function to update the Screensaver page content (time)
void updateScreensaverPage() {
    // Update time dynamically
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        char timeStr[9];  // Format "HH:MM:SS"
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
        lv_label_set_text(time_label, timeStr);
    } else {
        lv_label_set_text(time_label, "Error");  // If the time fails to update
    }
}

// Function to handle touch input for the Screensaver page (using CST816T)
void handleScreensaverPageTouch() {
    TS_Point p = touchBus.getPoint();  // Get the current touch point

    if (p.x != -1 && p.y != -1) {  // Check if the screen was touched
        // Implement touch handling logic (e.g., navigate to other pages)
        Serial.print("Touch detected at X: ");
        Serial.print(p.x);
        Serial.print(", Y: ");
        Serial.println(p.y);

        // Example: Swipe right to navigate to health page
        if (p.x > 100 && p.x < 200 && p.y > 100 && p.y < 200) {
            lv_scr_load(health_page);  // Switch to health page
        }

        // Example: Swipe left to navigate to watch page
        if (p.x < 100 && p.y > 100 && p.y < 200) {
            lv_scr_load(watch_page);   // Switch to watch page
        }
    }
}

// Function to initialize the display and touchscreen for the Screensaver
void initScreensaverDisplay() {
    // Initialize the TFT display (if not already initialized)
    tft.begin();
    tft.setRotation(3);  // Set the display rotation (adjust as needed)
    tft.fillScreen(ST77XX_BLACK);  // Clear the display to black
}

void initScreensaverTouchscreen() {
    touchBus.begin();  // Initialize the touch input
    // You can adjust touch parameters if needed (e.g., calibrate, set rotation)
}

// Function to simulate screensaver updates (for testing purposes)
void simulateScreensaverPage() {
    updateScreensaverPage();
    handleScreensaverPageTouch();  // Handle touch input for navigation
}
