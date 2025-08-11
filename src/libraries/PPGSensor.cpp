#include "DFRobot_BloodOxygen_S.h"

#define I2C_ADDRESS 0x57
DFRobot_BloodOxygen_S_I2C MAX30102(&Wire, I2C_ADDRESS);

void initPPGSensor() {
  while (false == MAX30102.begin())
  {
    //Serial.println("init fail!");
    delay(1000);
  }
  MAX30102.sensorStartCollect();
  // Initialize MAX30102 heart rate & SpO2 sensor
  // max30102.begin(); // Use your library’s init
}

int pollPPGSensorSP0() {
  MAX30102.getHeartbeatSPO2();
  return Serial.print(MAX30102._sHeartbeatSPO2.SPO2;
  MAX30102.sensorEndCollect();
}

int pollPPGSensorHeartbeat() {
  MAX30102.getHeartbeatSPO2();
  return MAX30102._sHeartbeatSPO2.Heartbeat;
  MAX30102.sensorEndCollect();
}

float pollPPGSensorTemperature() {
  MAX30102.getHeartbeatSPO2();
  return MAX30102.getTemperature_C());
  MAX30102.sensorEndCollect();
}

