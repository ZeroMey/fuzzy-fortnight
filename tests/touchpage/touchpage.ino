#include <Arduino.h>
//#include <Wire.h>
#include <memory>
#include "pin_config.h"
#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"

Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST, 0, true, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);

std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus = std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

void Arduino_IIC_Touch_Interrupt(void);
std::unique_ptr<Arduino_IIC> CST816T(new Arduino_CST816x(IIC_Bus, CST816T_DEVICE_ADDRESS, TP_RST, TP_INT, Arduino_IIC_Touch_Interrupt));
void Arduino_IIC_Touch_Interrupt(void) { CST816T->IIC_Interrupt_Flag = true; }

void setup() {
  //Wire.begin(IIC_SDA, IIC_SCL);
  while (!CST816T->begin()) delay(200);
  CST816T->IIC_Write_Device_State(
      CST816T->Arduino_IIC_Touch::Device::TOUCH_DEVICE_INTERRUPT_MODE,
      CST816T->Arduino_IIC_Touch::Device_Mode::TOUCH_DEVICE_INTERRUPT_PERIODIC);

  ledcAttach(LCD_BL, 500, 8);   // 20 kHz, 8-bit
  ledcWrite(LCD_BL, 64);         // 50%

  gfx->begin();
  gfx->fillScreen(BLACK);
  gfx->setTextColor(WHITE);
  gfx->setCursor(24, 130);
  gfx->print("My Label");
}

void loop() {
  if (CST816T->IIC_Interrupt_Flag) {
    CST816T->IIC_Interrupt_Flag = false;
    int32_t x = CST816T->IIC_Read_Device_Value(
        CST816T->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
    int32_t y = CST816T->IIC_Read_Device_Value(
        CST816T->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);
    if (x >= 0 && y >= 0) gfx->fillCircle(x, y, 3, BLUE);
  }
}
