#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "pin_config.h"

// SPI bus/display (pins and size from pin_config.h)
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST /* RST */, 0 /* rotation */, true /* IPS */, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);

void setup() {
  // 50% backlight using only ledcAttach/ledcWrite (Arduino-ESP32 v3.x API)
  ledcAttach(LCD_BL, 500, 8);         // 20 kHz, 12-bit
  ledcWrite(LCD_BL, 64);               // 50% duty

  gfx->begin();
  gfx->fillScreen(BLACK);
  gfx->setTextColor(WHITE);
  gfx->setCursor(24, 130);
  gfx->print("My Label");
}

void loop() {}