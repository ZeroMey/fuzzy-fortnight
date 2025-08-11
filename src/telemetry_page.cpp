#include "telemetry_page.h"
#include <Wire.h>                // I2C library for sensor communication

// Global page object and label variables
lv_obj_t *telemetry_page;       // Current telemetry page object
lv_obj_t *temp_label;           // Temperature label
lv_obj_t *humidity_label;       // Humidity label
lv_obj_t *light_label;          // Light intensity label
lv_obj_t *ir_temp_label;        // IR temperature label
lv_obj_t *status_label;         // Status label

// DHT22 sensor object (for temperature and humidity)
DHT dht(2, DHT22);  // DHT sensor on pin 2 (adjust as needed)

// TEMT6000 ambient light sensor object
TEMT6000 lightSensor(A0);  // Analog input pin for the TEMT6000

// MLX90614 IR temperature sensor object
MLX90614 mlx;

// Function to create the Telemetry page UI elements
void createTelemetryPage() {
    // Create the telemetry page object
    telemetry_page = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_bg_color(telemetry_page, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);  // Set background color

    // Create label for temperature
    temp_label = lv_label_create(telemetry_page, NULL);
    lv_label_set_text(temp_label, "Temp: -- C");  // Initial placeholder for temperature
    lv_obj_align(temp_label, NULL, LV_ALIGN_CENTER, 0, -30);  // Center the label on the screen

    // Create label for humidity
    humidity_label = lv_label_create(telemetry_page, NULL);
    lv_label_set_text(humidity_label, "Humidity: -- %");  // Initial placeholder for humidity
    lv_obj_align(humidity_label, temp_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  // Position below the temperature label

    // Create label for light intensity
    light_label = lv_label_create(telemetry_page, NULL);
    lv_label_set_text(light_label, "Light: -- lx");  // Initial placeholder for light intensity
    lv_obj_align(light_label, humidity_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  // Position below the humidity label

    // Create label for IR temperature
    ir_temp_label = lv_label_create(telemetry_page, NULL);
    lv_label_set_text(ir_temp_label, "IR Temp: -- C");  // Initial placeholder for IR temperature
    lv_obj_align(ir_temp_label, light_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  // Position below the light label

    // Create label for status (e.g., data fetching or error message)
    status_label = lv_label_create(telemetry_page, NULL);
    lv_label_set_text(status_label, "Initializing...");  // Initial status message
    lv_obj_align(status_label, ir_temp_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  // Position below the IR temperature label
}

// Function to update the Telemetry page content (sensor data)
void updateTelemetryPage() {
    // Fetch the temperature and humidity values from the DHT22 sensor
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    bool sensorDataReady = false;

    if (isnan(temperature) || isnan(humidity)) {
        lv_label_set_text(status_label, "DHT Error");
    } else {
        // Update the temperature label
        char tempStr[20];
        snprintf(tempStr, sizeof(tempStr), "Temp: %.2f C", temperature);
        lv_label_set_text(temp_label, tempStr);

        // Update the humidity label
        char humidityStr[20];
        snprintf(humidityStr, sizeof(humidityStr), "Humidity: %.2f %%", humidity);
        lv_label_set_text(humidity_label, humidityStr);

        sensorDataReady = true;
    }

    // Fetch the light intensity value from the TEMT6000 sensor
    int lightIntensity = lightSensor.readLight();
    if (lightIntensity < 0) {
        lv_label_set_text(status_label, "Light Sensor Error");
    } else {
        // Update the light intensity label
        char lightStr[20];
        snprintf(lightStr, sizeof(lightStr), "Light: %d lx", lightIntensity);
        lv_label_set_text(light_label, lightStr);
        sensorDataReady = true;
    }

    // Fetch the IR temperature value from the MLX90614 sensor
    float irTemp = mlx.readObjectTempC();
    if (isnan(irTemp)) {
        lv_label_set_text(status_label, "IR Temp Error");
    } else {
        // Update the IR temperature label
        char irTempStr[20];
        snprintf(irTempStr, sizeof(irTempStr), "IR Temp: %.2f C", irTemp);
        lv_label_set_text(ir_temp_label, irTempStr);
        sensorDataReady = true;
    }

    // If all sensor data is fetched successfully, update the status label
    if (sensorDataReady) {
        lv_label_set_text(status_label, "Data updated");
    }
}

// Function to initialize the Telemetry page and sensors
void initTelemetryPage() {
    // Initialize the DHT22 sensor
    dht.begin();

    // Initialize the TEMT6000 sensor
    lightSensor.begin();

    // Initialize the MLX90614 IR temperature sensor
    if (!mlx.begin()) {
        Serial.println("MLX90614 initialization failed!");
        while (1);  // Halt the program if the sensor fails to initialize
    }
}

// Function to simulate telemetry updates (for testing purposes)
void simulateTelemetryPage() {
    updateTelemetryPage();
    handleTelemetryPageTouch();  // Handle touch input for navigation
}
