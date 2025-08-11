#include "power_manager.h"
#include <Arduino.h>
#include <WiFi.h>  // Optional: If you use WiFi for wakeup (remove if not used)

// Function to initialize power management
void initPowerManager() {
    // Configure wakeup sources for deep sleep
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, ESP_EXT0_WAKEUP_ACTIVE_HIGH); // Example: Wakeup from GPIO 33 (you can change this pin)
    esp_sleep_enable_timer_wakeup(60 * 1000000);  // Set timer wakeup every 60 seconds (optional)

    Serial.println("Power manager initialized.");
}

// Function to put the ESP32 into deep sleep
void enterDeepSleep() {
    // If you're using a sensor or GPIO to wake up, you can configure those here.
    // For now, we configure it to wake up with an external GPIO (e.g., button press, accelerometer motion).
    Serial.println("Entering deep sleep...");

    // You can also enable the RTC memory for preserving variables between deep sleep sessions
    // RTC_DATA_ATTR int bootCount = 0;  // Example of persisting data (uncomment if needed)

    // Enter deep sleep mode
    esp_deep_sleep_start();
}

// Function to wake up from deep sleep (based on the wakeup source)
void wakeUpFromSleep() {
    // Check the reason for wakeup
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    switch (wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0:  // External pin (e.g., GPIO)
            Serial.println("Woken up by external GPIO.");
            break;
        case ESP_SLEEP_WAKEUP_TIMER:  // Timer wakeup
            Serial.println("Woken up by timer.");
            break;
        default:
            Serial.println("Woken up by an unknown cause.");
            break;
    }
}

// Function to enable a timer-based wakeup (e.g., every 10 seconds)
void enableTimerWakeup(uint64_t sleep_time_us) {
    // Set the timer wakeup period (in microseconds)
    esp_sleep_enable_timer_wakeup(sleep_time_us);
    Serial.print("Timer wakeup enabled for ");
    Serial.print(sleep_time_us / 1000000);
    Serial.println(" seconds.");
}

// Function to enable external interrupt wakeup (e.g., wake up on motion detection or button press)
void enableExternalWakeup(GPIO_NUM_ pin, bool active_high) {
    // Configure an external wakeup source from a GPIO pin
    esp_sleep_enable_ext0_wakeup(pin, active_high ? ESP_EXT0_WAKEUP_ACTIVE_HIGH : ESP_EXT0_WAKEUP_ACTIVE_LOW);
    Serial.print("External wakeup enabled on GPIO ");
    Serial.println(pin);
}

// Function to check if the device is in deep sleep
bool isInDeepSleep() {
    return (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER);
}
