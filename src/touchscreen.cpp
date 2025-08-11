#include "touchscreen.h"
#include <Arduino_TouchBus.h>    // CST816T touch driver library

// Global touch input object
extern Arduino_TouchBus touchBus;  // CST816T touch input object

// Function to initialize the touchscreen (CST816T)
void initTouchscreen() {
    touchBus.begin();  // Initialize the touch input bus for CST816T touchscreen
    Serial.println("Touchscreen initialized.");
}

// Function to handle touch input on the screen
void handleTouchInput() {
    TS_Point p = touchBus.getPoint();  // Get the current touch point from the touchscreen

    // Check if the screen was touched (coordinates are valid)
    if (p.x != -1 && p.y != -1) {
        // Print touch coordinates (for debugging purposes)
        Serial.print("Touch detected at X: ");
        Serial.print(p.x);
        Serial.print(", Y: ");
        Serial.println(p.y);

        // Handle specific touch areas based on coordinates (e.g., for swipes)

        // Swipe Left (Navigate to Screensaver Page)
        if (p.x < 100 && p.y > 100 && p.y < 200) {
            lv_scr_load(screensaver_page);  // Switch to screensaver page
        }

        // Swipe Right (Navigate to Health Page)
        if (p.x > 200 && p.y > 100 && p.y < 200) {
            lv_scr_load(health_page);  // Switch to health page
        }
    }
}

// Function to detect swipe gestures (left or right)
void handleSwipeGestures() {
    static int startX = -1, startY = -1;  // Initial touch position
    static int endX = -1, endY = -1;      // Final touch position
    static bool isTouching = false;        // Flag to check if the screen is being touched

    if (touchBus.isTouched()) {
        // Get the current touch point
        TS_Point p = touchBus.getPoint();

        if (startX == -1 && startY == -1) {
            // First touch event, record the initial touch position
            startX = p.x;
            startY = p.y;
            isTouching = true;
        }

        // Update the end touch position
        endX = p.x;
        endY = p.y;

        // If the user has lifted the finger (touch release)
        if (!touchBus.isTouched() && isTouching) {
            int deltaX = endX - startX;
            int deltaY = endY - startY;

            // Check if the movement is mostly horizontal (for detecting swipe)
            if (abs(deltaY) < 50) {  // Allow some vertical tolerance for swipe
                if (deltaX > 100) {
                    // Swipe right: Navigate to health page
                    lv_scr_load(health_page);
                } else if (deltaX < -100) {
                    // Swipe left: Navigate to screensaver page
                    lv_scr_load(screensaver_page);
                }
            }

            // Reset touch positions after swipe
            startX = -1;
            startY = -1;
            isTouching = false;
        }
    }
}
