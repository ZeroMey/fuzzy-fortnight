#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "pin_config.h"

#include "DFRobot_BloodOxygen_S.h"

#define I2C_COMMUNICATION  //use I2C for communication, but use the serial port for communication if the line of codes were masked

#ifdef  I2C_COMMUNICATION
#define I2C_ADDRESS    0x57
  DFRobot_BloodOxygen_S_I2C MAX30102(&Wire ,I2C_ADDRESS);
#endif

// SPI bus/display (pins and size from pin_config.h)
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST /* RST */, 0 /* rotation */, true /* IPS */, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);

int sp0 = 0;  

void setup() {
  // 50% backlight using only ledcAttach/ledcWrite (Arduino-ESP32 v3.x API)
  ledcAttach(LCD_BL, 500, 8);         // 20 kHz, 12-bit
  ledcWrite(LCD_BL, 64);               // 50% duty

  gfx->begin();
  gfx->fillScreen(BLACK);
  gfx->setTextColor(WHITE);
  gfx->setTextSize(2); 
  gfx->setCursor(24, 130);
  gfx->print("Beat");

  Serial.begin(115200);
  while (false == MAX30102.begin())
  {
    Serial.println("init fail!");
    delay(1000);
  }
  delay(3000);
  Serial.println("init success!");
  Serial.println("start measuring...");
  MAX30102.sensorStartCollect();
}

void loop() {

    MAX30102.getHeartbeatSPO2();
    //sp0 = MAX30102._sHeartbeatSPO2.Heartbeat;
    // Overwrite previous text by drawing with background color
    gfx->setCursor(24, 130);
    gfx->setTextColor(WHITE, BLACK);      // fg=WHITE, bg=BLACK erases old text
    gfx->print("Beat: ");
    gfx->print(MAX30102._sHeartbeatSPO2.Heartbeat);                   // 2 decimals; change as needed
    delay(4000);
}