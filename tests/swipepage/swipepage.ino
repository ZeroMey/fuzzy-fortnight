#include <Arduino.h>
#include <Wire.h>
#include <memory>
#include "pin_config.h"
#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"

// Display
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST, 0, true, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);

// Touch (CST816)
std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus = std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);
void Arduino_IIC_Touch_Interrupt(void);
std::unique_ptr<Arduino_IIC> CST816T(new Arduino_CST816x(IIC_Bus, CST816T_DEVICE_ADDRESS, TP_RST, TP_INT, Arduino_IIC_Touch_Interrupt));
void Arduino_IIC_Touch_Interrupt(void) { CST816T->IIC_Interrupt_Flag = true; }

// Pager state
static int page = 0;
static bool tracking = false;
static int startX = 0, startY = 0;
static uint32_t lastTouchMs = 0;

void drawPage() {
  gfx->fillScreen(BLACK);
  gfx->setTextColor(WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(24, 130);
  if (page == 0)      gfx->print("My Label");
  else if (page == 1) gfx->print("New Label");
  else                gfx->print("Third Label");
}

void setup() {
  Wire.begin(IIC_SDA, IIC_SCL);
  while (!CST816T->begin()) delay(200);
  CST816T->IIC_Write_Device_State(
      CST816T->Arduino_IIC_Touch::Device::TOUCH_DEVICE_INTERRUPT_MODE,
      CST816T->Arduino_IIC_Touch::Device_Mode::TOUCH_DEVICE_INTERRUPT_PERIODIC);

  ledcAttach(LCD_BL, 500, 8);
  ledcWrite(LCD_BL, 64);

  gfx->begin();
  drawPage();
}

void loop() {
  if (CST816T->IIC_Interrupt_Flag) {
    CST816T->IIC_Interrupt_Flag = false;

    int32_t x = CST816T->IIC_Read_Device_Value(
        CST816T->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
    int32_t y = CST816T->IIC_Read_Device_Value(
        CST816T->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);

    uint32_t now = millis();
    if (x >= 0 && y >= 0 && x < LCD_WIDTH && y < LCD_HEIGHT) {
      if (!tracking) { tracking = true; startX = x; startY = y; }
      else {
        int dx = x - startX;
        int dy = y - startY;
        const int th = 60;           // swipe threshold (px)
        if (abs(dy) <= 40) {
          if (dx <= -th) {           // left swipe -> next page
            page = (page + 1) % 3;
            drawPage();
            tracking = false;
            delay(120);
          } else if (dx >= th) {     // right swipe -> prev page
            page = (page + 2) % 3;   // (page - 1 + 3) % 3
            drawPage();
            tracking = false;
            delay(120);
          }
        }
      }
      lastTouchMs = now;
    }
  }
  if (tracking && (millis() - lastTouchMs > 200)) tracking = false; // gesture timeout
}
