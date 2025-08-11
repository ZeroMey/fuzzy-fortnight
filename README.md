/src
  /main.ino

    Setup:
      Initialize sensors, FreeRTOS tasks, LVGL display, and touchscreen.
      Manage FreeRTOS scheduling and task prioritization.

    Loop:
      Handle LVGL task handler (lv_task_handler()).
      Manage low-power modes using FreeRTOS.

  // Pages
  // Purpose: Each page manages a distinct UI state and content.

    /screensaver_page.h
      Handles the idle screen. It could be activated by accelerometer motion and may show
        a simple clock or a sleeping indicator. This page should be lightweight, using minimal resources
        and triggered by inactivity or motion.
    /watch_page.h
      watch_page.h: The main UI page where the time, date, and possibly 
        some additional information (like battery level) are displayed.
    /health_page.h
      health_page.h: Displays health-related data like heart rate (from MAX30102), SpO2,
        and potentially trends over time.
    /telemetry_page.h
      telemetry_page.h: Displays all sensor data such as temperature (from MLX90614), 
        ambient light (from TEMT6000), temperature and humidity (from DHT22), etc. 
        This page could have dynamic graphs or data updates.

  //Sensor Modules (max30102_sensor.h, mlx90614-dss_sensor.h, etc.)
  //Each sensor will have its own module to manage initialization, data reading, and sensor-specific functions.

    /max30102_sensor.h
    /mlx90614-dss_sensor.h
    /temt6000_sensor.h
    /dht22_sensor.h
    /QMI8658.h
    /SEN0502.h

  // Other 
  // Purpose: Manages touchscreen, display, utilities and power

    /touchscreen.h
    Purpose: Manage touchscreen input for detecting gestures such as swipes and taps.
      Handle event detection (e.g., swipe left to go to the next page, tap to select data on telemetry).
      Interface with the LVGL touchscreen driver to detect touch events and trigger UI updates.
    /display_manager.h
    Purpose: Manage the LVGL display rendering and transitions between pages.
      Update the screen when a new page is shown.
      Handle UI elements like buttons, text, and animations.
      Manage smooth transitions between screens (e.g., swipe effects, fading).
    /utils.h
      Data Formatting: Format data like temperature, humidity, heart rate for display (e.g., 
      formatting numbers with a specific precision).
      Averaging: For smoothing sensor readings, calculate averages over a time period.
      Conversions: Converting sensor units (e.g., temperature in Celsius to Fahrenheit).
    /power_manager.h
    Purpose: Manage power states, low-power sleep modes, and wakeup triggers.
      Deep Sleep: Use ESP32's low-power modes (like deep sleep) when the watch is idle or 
      when the screensaver page is active.
      Wakeup Triggers: Set up wakeup from sleep on specific events such as accelerometer motion, 
      touch input, or sensor polling intervals.



