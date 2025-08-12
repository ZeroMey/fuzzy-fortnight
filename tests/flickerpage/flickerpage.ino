#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "pin_config.h"


// --- Display (ST7789 240x280, 20px top offset set here) ---
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST, 0, true, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);
//Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
// If your RST wiring is unreliable or shared, force SW reset by passing -1:
//Arduino_GFX *gfx = new Arduino_ST7789(bus, /*rst=*/GFX_NOT_DEFINED, 0, true, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);

void setup() {
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);      // full on, no PWM during debug
  gfx->begin();                    // does SW reset if rst == -1
  gfx->fillScreen(BLACK);
}

void loop() {
  gfx->fillScreen(RED);   delay(3000);
  gfx->fillScreen(GREEN); delay(3000);
  gfx->fillScreen(BLUE);  delay(3000);
}
