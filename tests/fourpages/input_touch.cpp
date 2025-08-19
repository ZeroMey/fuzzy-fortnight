#include "input_touch.h"
#include <Arduino.h>
#include <Wire.h>
#include <memory>

#include "Arduino_DriveBus_Library.h"
#include "pin_config.h"
#include "HWCDC.h"

// External sync/log (defined in your main or another module)
extern SemaphoreHandle_t i2cMutex;
extern HWCDC USBSerial;

// ---- Local state ----
static std::shared_ptr<Arduino_IIC_DriveBus> s_i2c_bus;
static std::unique_ptr<Arduino_IIC>          s_cst816;
static volatile bool                          s_ready = false;

// ISR: marks a flag inside the driver object (library pattern)
void IRAM_ATTR Arduino_IIC_Touch_Interrupt(void) {
  if (s_cst816) s_cst816->IIC_Interrupt_Flag = true;
}

// ---- Public API ----
void touch_init() {
  if (!s_i2c_bus) {
    s_i2c_bus = std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);
  }
  if (!s_cst816) {
    s_cst816.reset(new Arduino_CST816x(
        s_i2c_bus, CST816T_DEVICE_ADDRESS, TP_RST, TP_INT, Arduino_IIC_Touch_Interrupt));
  }

  // Probe until success (keeps previous behavior)
  while (!s_cst816->begin()) {
    USBSerial.println("CST816T init failed; retrying...");
    delay(2000);
  }
  USBSerial.println("CST816T initialized");

  // Periodic interrupt mode (matches your original)
  s_cst816->IIC_Write_Device_State(
      s_cst816->Arduino_IIC_Touch::Device::TOUCH_DEVICE_INTERRUPT_MODE,
      s_cst816->Arduino_IIC_Touch::Device_Mode::TOUCH_DEVICE_INTERRUPT_PERIODIC);

  s_ready = true;
}

// LVGL read callback
void my_touchpad_read(lv_indev_drv_t * /*indev*/, lv_indev_data_t *data) {
  // Not ready yet -> no touch
  if (!s_ready || !s_cst816) {
    data->state = LV_INDEV_STATE_REL;
    return;
  }

  // No pending touch interrupt -> no touch
  if (!s_cst816->IIC_Interrupt_Flag) {
    data->state = LV_INDEV_STATE_REL;
    return;
  }

  // Short, non-blocking take to avoid UI jitter
  if (i2cMutex && xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(5)) != pdTRUE) {
    data->state = LV_INDEV_STATE_REL;
    return;
  }

  s_cst816->IIC_Interrupt_Flag = false;

  const int32_t touchX = s_cst816->IIC_Read_Device_Value(
      s_cst816->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
  const int32_t touchY = s_cst816->IIC_Read_Device_Value(
      s_cst816->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);

  if (i2cMutex) xSemaphoreGive(i2cMutex);

  if (touchX >= 0 && touchY >= 0) {
    data->state   = LV_INDEV_STATE_PR;
    data->point.x = touchX;
    data->point.y = touchY;
    // Debug (optional)
    // USBSerial.printf("Touch: x=%ld y=%ld\n", (long)touchX, (long)touchY);
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
}
