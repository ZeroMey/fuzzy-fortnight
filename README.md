# fuzzy-fortnight

Skeleton firmware for an LVGL-based watch/GUI: display and input managers plus UI pages (watch, telemetry, health, screensaver).

## Repository layout
```
.
├─ src/
│ ├─ screensaver_page.[h|cpp]
│ ├─ watch_page.[h|cpp]
│ ├─ health_page.[h|cpp]
│ ├─ telemetry_page.[h|cpp]
│ ├─ display_manager.[h|cpp]
│ ├─ touchscreen.[h|cpp]
│ ├─ power_manager.[h|cpp]
│ └─ utils.[h|cpp]
├─ examples/
│ └─ fourpages/
│ └─ fourpages.ino
└─ README.md
```

## Modules

### `screensaver_page`
**Purpose:** Screensaver/idle page.  
**Public API:**
- `void createScreensaverPage();` — Setup screensaver page layout
- `void updateScreensaverPage();` — Update content or handle events
- `void activateScreensaver();` — Activate screensaver (e.g., idle mode)

### `watch_page`
**Purpose:** UI page showing time/date/battery.  
**Public API:**
- `void createWatchPage();` — Setup watch page layout (time display, etc.)
- `void updateWatchPage();` — Update content like time, date, and battery

### `health_page`
**Purpose:** UI page for health metrics (HR/SpO₂).  
**Public API:**
- `void createHealthPage();` — Setup health telemetry UI (heart rate, SpO₂)
- `void updateHealthPage();` — Update telemetry page with new sensor data

### `telemetry_page`
**Purpose:** UI page for general sensor telemetry.  
**Public API:**
- `void createTelemetryPage();` — Setup telemetry page layout (sensor data display)
- `void updateTelemetryPage();` — Update sensor data on telemetry page

### `display_manager`
**Purpose:** LVGL + TFT glue; display init, refresh, page transitions.  
**Public API:**
- `void initDisplay();` — Initialize display (TFT/OLED)
- `void clearDisplay();` — Clear the display
- `void updateDisplay();` — Refresh the display with updated content
- `void transitionToPage(lv_obj_t* page);` — Transition smoothly between pages

### `touchscreen`
**Purpose:** Touch input init and event handling.  
**Public API:**
- `void initTouchscreen();` — Initialize touchscreen input
- `void handleTouchInput();` — Handle user interaction (swipe, tap, etc.)

 ### `power_manager`
**Purpose:** Power modes; init, deep sleep, wake.  
**Public API:**
- `void initPowerManager();` — Initialize power management (sleep, wakeup)
- `void enterDeepSleep();` — Enter deep sleep mode to save power
- `void wakeUpFromSleep();` — Wake up from sleep when triggered (e.g., touch, accelerometer)

### `utils`
**Purpose:** Small numeric/format helpers.  
**Public API:**
- `float formatTemperature(float temp);` — Format temperature values
- `float averageData(float* data, int size);` — Calculate average of sensor data
- `float convertToFahrenheit(float celsius);` — Convert Celsius to Fahrenheit

## Example
- `examples/fourpages/fourpages.ino` demonstrates wiring pages with LVGL, a TFT driver, and a touchscreen driver.

## Build
- Arduino IDE or PlatformIO.
- Install LVGL and your concrete display/touch drivers (e.g., Adafruit ST77xx/ILI9341; CST816T touch).
- Provide concrete driver instances where the code expects `extern` display/touch objects.

## Notes
- Headers expose page creation/update entry points and system managers; hardware specifics and event routing live in your sketch.
