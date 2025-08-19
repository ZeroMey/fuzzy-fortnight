#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include <stdint.h>
/**/
// Shared I²C device addresses
#define PCF85063_ADDR 0x51
#define QMI8658_ADDR  0x6B  // some boards use 0x6A

// Shared data snapshot (used by your UI timer)
struct SharedData {
  uint8_t hh, mm, ss;
  float gx, gy, gz;
  bool rtc_valid;
  bool imu_valid;
};

// I²C/IMU/RTC API
bool restore_last_epoch();
void rtc_init();
void rtc_init_if_needed();  // sets RTC once if oscillator-stop (VL) is detected
bool rtc_sync_over_serial(uint32_t timeout_ms = 15000);
void rtc_read_time(uint8_t &hh, uint8_t &mm, uint8_t &ss, bool &ok);
bool imu_begin();
bool imu_read_gyro(float &gx, float &gy, float &gz);

// Tasks (same names as before)
extern void rtcTask(void *arg);
extern void imuTask(void *arg);

#endif // DEVICE_MANAGER_H
