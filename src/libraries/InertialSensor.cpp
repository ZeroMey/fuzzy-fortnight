#include "SensorQMI8658.hpp"

SensorQMI8658 qmi;

IMUdata acc;
IMUdata gyr;

void initInertialSensor() {
  // Initialize QMI8658 accelerometer
  // if (!qmi8658.begin()) {
  //   Serial.println("QMI8658 initialization failed!");
  //   while (1) { delay(10); }
  // }

    if (!qmi.begin(Wire, QMI8658_L_SLAVE_ADDRESS, IIC_SDA, IIC_SCL)) {

    while (1) {
      delay(1000);
    }
  }

  // Get chip ID
  //USBSerial.println(qmi.getChipID());

  qmi.configAccelerometer(
    SensorQMI8658::ACC_RANGE_4G,
    SensorQMI8658::ACC_ODR_1000Hz,
    SensorQMI8658::LPF_MODE_0,
    true);

  qmi.configGyroscope(
    SensorQMI8658::GYR_RANGE_64DPS,
    SensorQMI8658::GYR_ODR_896_8Hz,
    SensorQMI8658::LPF_MODE_3,
    true);

  qmi.enableGyroscope();
  qmi.enableAccelerometer();

  qmi.dumpCtrlRegister();
  
}

float pollInertialSensorAccX() {
  if (qmi.getDataReady()) {
    if (qmi.getAccelerometer(acc.x, acc.y, acc.z)) {
      return acc.x;
    }
  }

float pollInertialSensorAccY() {
  if (qmi.getDataReady()) {
    if (qmi.getAccelerometer(acc.x, acc.y, acc.z)) {
      return acc.y;
    }
  }

float pollInertialSensorAccZ() {
  if (qmi.getDataReady()) {
    if (qmi.getAccelerometer(acc.x, acc.y, acc.z)) {
      return acc.z;
    }
  }

float pollInertialSensorGyrX() {
  if (qmi.getDataReady()) {
    if (qmi.getGyroscope(gyr.x, gyr.y, gyr.z)) {
      return gyr.x;
    }
  }

float pollInertialSensorGyrY() {
  if (qmi.getDataReady()) {
    if (qmi.getGyroscope(gyr.x, gyr.y, gyr.z)) {
      return gyr.y;
    }
  }

float pollInertialSensorGyrZ() {
  if (qmi.getDataReady()) {
    if (qmi.getGyroscope(gyr.x, gyr.y, gyr.z)) {
      return gyr.z;
    }
  }
