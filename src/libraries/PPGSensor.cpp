#include "DFRobot_BloodOxygen_S.h"

#define I2C_ADDRESS 0x57
DFRobot_BloodOxygen_S_I2C MAX30102(&Wire, I2C_ADDRESS);

static void initPPGSensor() {

  while (false == MAX30102.begin())
  {
    Serial.println("init fail!");
    delay(1000);
  }
  // Initialize MAX30102 heart rate & SpO2 sensor
  // max30102.begin(); // Use your library’s init
}
