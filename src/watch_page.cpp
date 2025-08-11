#include "watch_page.h"
#include <Adafruit_GFX.h>        // GFX library for display support
#include <Adafruit_ST7789.h>     // ST7789V2 display driver
#include <Arduino_TouchBus.h>    // Touch input for CST816T

// Global display object
extern Adafruit_ST7789 tft;   // Create display object for ST7789 display
extern Arduino_TouchBus touchBus;  // CST816T touch input object

// Global page object and label variables
lv_obj_t *watch_page;          // Current watch page object
lv_obj_t *time_label;
lv_obj_t *date_label;
lv_obj_t *battery_label;

// Swipe gesture tracking
int startX = -1, startY = -1;  // Initial touch position
int endX = -1, endY = -1;      // Final touch position
bool isTouching = false;        // Flag to check if the screen is being touched

int batteryPercentage = 100;    // Placeholder for battery percentage (you can implement actual battery reading)

// Function to create the Watch page UI elements
void createWatchPage() {
    // Create the watch page object
    watch_page = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_bg_color(watch_page, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);  // Set background color

    // Create label for time
    time_label = lv_label_create(watch_page, NULL);
    lv_label_set_text(time_label, "00:00:00");  // Initial placeholder time
    lv_obj_align(time_label, NULL, LV_ALIGN_CENTER, 0, -30);  // Center the label on the screen

    // Create label for date
    date_label = lv_label_create(watch_page, NULL);
    lv_label_set_text(date_label, "Jan 1, 2025");  // Initial placeholder date
    lv_obj_align(date_label, time_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  // Position below the time label

    // Create label for battery percentage
    battery_label = lv_label_create(watch_page, NULL);
    lv_label_set_text(battery_label, "Battery: 100%");  // Placeholder battery status
    lv_obj_align(battery_label, date_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  // Position below the date label
}

// Function to update the Watch page content (time, date, battery)
void updateWatchPage() {
    // Update time dynamically
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        char timeStr[9];  // Format "HH:MM:SS"
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
        lv_label_set_text(time_label, timeStr);
    } else {
        lv_label_set_text(time_label, "Error");  // If the time fails to update
    }

    // Update date dynamically
    char dateStr[20];  // Format "Mon DD, YYYY"
    if (getLocalTime(&timeinfo)) {
        strftime(dateStr, sizeof(dateStr), "%b %d, %Y", &timeinfo);
        lv_label_set_text(date_label, dateStr);
    } else {
        lv_label_set_text(date_label, "Error");
    }

    // Update battery percentage (simulate decrease)
    batteryPercentage = (batteryPercentage - 1) < 0 ? 100 : batteryPercentage - 1;  // Simulate battery decrease
    char batteryStr[20];
    snprintf(batteryStr, sizeof(batteryStr), "Battery: %d%%", batteryPercentage);
    lv_label_set_text(battery_label, batteryStr);
}

// Function to handle swipe gestures for page switching
void handleSwipeGesture() {
    // Check if the screen is touched
    if (touchBus.isTouched()) {
        TS_Point p = touchBus.getPoint(); // Get the current touch point

        if (startX == -1 && startY == -1) {
            // First touch event, set initial touch position
            startX = p.x;
            startY = p.y;
            isTouching = true;
        }

        endX = p.x;
        endY = p.y;

        // If the user has lifted the finger
        if (!touchBus.isTouched() && isTouching) {
            int deltaX = endX - startX;
            int deltaY = endY - startY;

            // Check if the movement is mostly horizontal (detect swipe)
            if (abs(deltaY) < 50) { // Allow some vertical tolerance for swipe
                if (deltaX > 100) {
                    // Swipe right: Navigate to health page
                    lv_scr_load(health_page);
                } else if (deltaX < -100) {
                    // Swipe left: Navigate to screensaver page
                    lv_scr_load(screensaver_page);
                }
            }

            // Reset touch positions
            startX = -1;
            startY = -1;
            isTouching = false;
        }
    }
}

// Function to simulate time and battery decrease (for testing purposes)
void simulateWatchPage() {
    updateWatchPage();
    handleSwipeGesture();  // Handle swipe gesture input
}
