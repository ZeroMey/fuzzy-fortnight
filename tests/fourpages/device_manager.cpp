//#include <Arduino.h>
//#include <Wire.h>
#include <time.h>
#include <Preferences.h>
#include <sys/time.h>
#include <stdio.h>
#include "Arduino_DriveBus_Library.h"
#include "device_manager.h"
#include "SensorQMI8658.hpp"
#include "pin_config.h"
#include "HWCDC.h"
#include <sys/time.h> // Time over Serial
#include <ctype.h> // Time over Serial
/**/

#ifndef RTC_BUILD_FALLBACK
#define RTC_BUILD_FALLBACK 1   // set to 1 if you ever want build-time fallback again
#endif

// Globals owned by main (we reference them)
extern SemaphoreHandle_t i2cMutex;
extern SemaphoreHandle_t dataMutex;
extern SharedData gData;
extern HWCDC USBSerial;

// Local to this module
static SensorQMI8658 qmi;

static inline uint8_t bcd2bin(uint8_t v) { return (v & 0x0F) + 10 * (v >> 4); }

static inline uint8_t bin2bcd(uint8_t v) { return uint8_t(((v / 10) << 4) | (v % 10)); }

// Save the current/known epoch to NVS
static inline void save_last_epoch(time_t epoch) {
  Preferences p;
  if (p.begin("rtc", false)) { p.putULong64("epoch", (uint64_t)epoch); p.end(); }
}

// Restore epoch from NVS into the ESP32 system clock. Returns true if restored.
bool restore_last_epoch() {
  Preferences p;
  if (!p.begin("rtc", true)) return false;
  uint64_t raw = p.getULong64("epoch", 0ULL);
  p.end();
  if (raw < 946684800ULL) return false;              // sanity: < 2000-01-01
  struct timeval tv{ .tv_sec = (time_t)raw, .tv_usec = 0 };
  settimeofday(&tv, nullptr);
  return true;
}

// --- Minimal helpers ---
static uint8_t month_from_str(const char *mmm) {
  // __DATE__ month is "Jan".."Dec"
  switch ((mmm[0] << 16) | (mmm[1] << 8) | mmm[2]) {
    case ('J'<<16|'a'<<8|'n'): return 1;
    case ('F'<<16|'e'<<8|'b'): return 2;
    case ('M'<<16|'a'<<8|'r'): return 3;
    case ('A'<<16|'p'<<8|'r'): return 4;
    case ('M'<<16|'a'<<8|'y'): return 5;
    case ('J'<<16|'u'<<8|'n'): return 6;
    case ('J'<<16|'u'<<8|'l'): return 7;
    case ('A'<<16|'u'<<8|'g'): return 8;
    case ('S'<<16|'e'<<8|'p'): return 9;
    case ('O'<<16|'c'<<8|'t'): return 10;
    case ('N'<<16|'o'<<8|'v'): return 11;
    case ('D'<<16|'e'<<8|'c'): return 12;
  }
  return 1;
}

static void get_build_time(uint8_t &yy, uint8_t &mo, uint8_t &dd,
                           uint8_t &hh, uint8_t &mi, uint8_t &ss) {
  char mon[4] = {0};
  int d=1, y=2000, H=0, M=0, S=0;
  // __DATE__ = "Mmm dd yyyy", __TIME__ = "hh:mm:ss"
  sscanf(__DATE__, "%3s %d %d", mon, &d, &y);
  sscanf(__TIME__, "%d:%d:%d", &H, &M, &S);
  yy = uint8_t(y % 100);
  mo = month_from_str(mon);
  dd = uint8_t(d);
  hh = uint8_t(H);
  mi = uint8_t(M);
  ss = uint8_t(S);
}

// Safe take/give even if i2cMutex is null (early init)
static bool take_i2c(TickType_t to) {
  extern SemaphoreHandle_t i2cMutex;
  return (!i2cMutex) || (xSemaphoreTake(i2cMutex, to) == pdTRUE);
}
static void give_i2c() {
  extern SemaphoreHandle_t i2cMutex;
  if (i2cMutex) xSemaphoreGive(i2cMutex);
}

// --- Forced set from build time ---
void rtc_init() {
  extern HWCDC USBSerial;

  uint8_t yy, mo, dd, hh, mi, ss;
  get_build_time(yy, mo, dd, hh, mi, ss);

  if (!take_i2c(pdMS_TO_TICKS(50))) return;
  Wire.beginTransmission(PCF85063_ADDR);
  Wire.write(0x04);                        // seconds..year
  Wire.write(bin2bcd(ss) & 0x7F);          // seconds (clear VL)
  Wire.write(bin2bcd(mi) & 0x7F);          // minutes
  Wire.write(bin2bcd(hh) & 0x3F);          // hours (24h)
  Wire.write(bin2bcd(dd) & 0x3F);          // day
  Wire.write(0);                            // weekday (0=Sun) — minimal; not critical
  Wire.write(bin2bcd(mo) & 0x1F);          // month
  Wire.write(bin2bcd(yy));                 // year (00..99)
  uint8_t err = Wire.endTransmission();
  give_i2c();

  USBSerial.printf(err ? "[RTC] set failed (%u)\n"
                       : "[RTC] set to %02u-%02u-%02u %02u:%02u:%02u\n",
                   err, yy, mo, dd, hh, mi, ss);
}

// --- Set only if VL=1 (oscillator stopped) ---
void rtc_init_if_needed() {
  // Probe Seconds (bit7 = VL). On I2C error, do nothing.
  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) != pdTRUE) return;
  Wire.beginTransmission(PCF85063_ADDR);
  Wire.write(0x04);
  if (Wire.endTransmission(false) != 0 || Wire.requestFrom(PCF85063_ADDR, 1) != 1) {
    xSemaphoreGive(i2cMutex);
    return;  // don't treat I2C errors as VL=1
  }
  uint8_t sec = Wire.read();
  xSemaphoreGive(i2cMutex);

  if ((sec & 0x80) == 0) {
    // VL=0 → RTC valid, nothing to do.
    return;
  }

#if RTC_BUILD_FALLBACK
  // Optional, only if you explicitly enable it:
  rtc_init();  // set from __DATE__/__TIME__
#else
  // Prevent fallback to build time:
  // Leave time untouched; user can call rtc_sync_over_serial() or rtc_init() explicitly.
  USBSerial.println("[RTC] VL=1 — skipping build-time fallback (define RTC_BUILD_FALLBACK=1 to enable).");
#endif
}


// helper: write PCF85063 from a tm (local time)
static bool rtc_write_from_tm(const tm &t) {
  extern SemaphoreHandle_t i2cMutex;

  const uint8_t yy = uint8_t((t.tm_year + 1900) % 100);
  const uint8_t mo = uint8_t(t.tm_mon + 1);
  const uint8_t dd = uint8_t(t.tm_mday);
  const uint8_t wd = uint8_t(t.tm_wday);      // 0..6 (Sun..Sat)
  const uint8_t hh = uint8_t(t.tm_hour);
  const uint8_t mi = uint8_t(t.tm_min);
  const uint8_t ss = uint8_t(t.tm_sec);

  if (!i2cMutex || xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
    Wire.beginTransmission(PCF85063_ADDR);
    Wire.write(0x04);                         // seconds..year
    Wire.write(bin2bcd(ss) & 0x7F);           // clears VL
    Wire.write(bin2bcd(mi) & 0x7F);
    Wire.write(bin2bcd(hh) & 0x3F);           // 24h
    Wire.write(bin2bcd(dd) & 0x3F);
    Wire.write(bin2bcd(wd) & 0x07);
    Wire.write(bin2bcd(mo) & 0x1F);
    Wire.write(bin2bcd(yy));
    const uint8_t err = Wire.endTransmission();
    if (i2cMutex) xSemaphoreGive(i2cMutex);
    return err == 0;
  }
  return false;
}

bool rtc_sync_over_serial(uint32_t timeout_ms) {
  extern HWCDC USBSerial;

  USBSerial.println("[RTC] Send one line: 'EPOCH <secs>' or 'TIME YYYY-MM-DD HH:MM:SS'");

  // clear any stale input
  while (USBSerial.available()) USBSerial.read();

  // read one line with timeout
  char buf[80]; size_t n = 0;
  const uint32_t t0 = millis();
  while ((millis() - t0) < timeout_ms) {
    if (USBSerial.available()) {
      int c = USBSerial.read();
      if (c == '\r') continue;
      if (c == '\n') break;
      if (n < sizeof(buf) - 1) buf[n++] = char(c);
    } else {
      delay(1);
    }
  }
  buf[n] = '\0';
  if (n == 0) { USBSerial.println("[RTC] timeout / no input"); return false; }

  // parse input
  time_t epoch = 0;
  tm t{};
  unsigned long secs;
  int Y,M,D,H,Min,S;

  if (sscanf(buf, "EPOCH %lu", &secs) == 1) {
    epoch = (time_t)secs;
    localtime_r(&epoch, &t);                  // convert to local time for RTC
  } else if (sscanf(buf, "TIME %d-%d-%d %d:%d:%d", &Y, &M, &D, &H, &Min, &S) == 6) {
    t = tm{}; t.tm_year = Y - 1900; t.tm_mon = M - 1; t.tm_mday = D;
    t.tm_hour = H; t.tm_min = Min; t.tm_sec = S; t.tm_isdst = -1;
    epoch = mktime(&t);                       // assumes values are local time
  } else {
    USBSerial.printf("[RTC] bad format: '%s'\n", buf);
    return false;
  }


  // set ESP32 system time
  timeval tv{ .tv_sec = epoch, .tv_usec = 0 };
  settimeofday(&tv, nullptr);

  // refresh tm from epoch to ensure wday etc. are consistent
  localtime_r(&epoch, &t);

  // set external RTC
  const bool ok = rtc_write_from_tm(t);
  USBSerial.printf("[RTC] %s: %04d-%02d-%02d %02d:%02d:%02d (epoch=%ld)\n",
                   ok ? "synced" : "RTC write failed",
                   t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                   t.tm_hour, t.tm_min, t.tm_sec, (long)epoch);
  return ok;
}

void rtc_read_time(uint8_t &hh, uint8_t &mm, uint8_t &ss, bool &ok) {
  ok = false;
  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(50)) != pdTRUE) return;

  Wire.beginTransmission(PCF85063_ADDR);
  Wire.write(0x04); // seconds register
  if (Wire.endTransmission(false) != 0) { xSemaphoreGive(i2cMutex); return; }

  if (Wire.requestFrom(PCF85063_ADDR, 7) != 7) { xSemaphoreGive(i2cMutex); return; }

  uint8_t sec  = Wire.read();
  uint8_t min  = Wire.read();
  uint8_t hour = Wire.read();
  Wire.read(); // day
  Wire.read(); // weekday
  Wire.read(); // month
  Wire.read(); // year

  xSemaphoreGive(i2cMutex);

  ss = bcd2bin(sec & 0x7F);
  mm = bcd2bin(min & 0x7F);
  hh = bcd2bin(hour & 0x3F);
  ok = true;
}

bool imu_begin() {
  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(200)) != pdTRUE) return false;

  const bool ok = qmi.begin(Wire, QMI8658_ADDR, IIC_SDA, IIC_SCL);
  if (ok) {
    qmi.configGyroscope(
      SensorQMI8658::GYR_RANGE_64DPS,
      SensorQMI8658::GYR_ODR_896_8Hz,
      SensorQMI8658::LPF_MODE_3,
      true);
    qmi.enableGyroscope();
    qmi.enableAccelerometer();
    qmi.dumpCtrlRegister();
  }
  xSemaphoreGive(i2cMutex);

  if (!ok) USBSerial.println("QMI8658 init failed (addr/wiring).");
  return ok;
}

bool imu_read_gyro(float &gx, float &gy, float &gz) {
  if (!qmi.getDataReady()) return false;
  return qmi.getGyroscope(gx, gy, gz);
}

extern void rtcTask(void *arg) {
  const TickType_t period = pdMS_TO_TICKS(500);
  TickType_t last = xTaskGetTickCount();

  TickType_t next_save = xTaskGetTickCount() + pdMS_TO_TICKS(15000);
  for (;;) {
    uint8_t hh, mm, ss; bool ok;
    rtc_read_time(hh, mm, ss, ok);

    if (ok && xSemaphoreTake(dataMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
      gData.hh = hh; gData.mm = mm; gData.ss = ss; gData.rtc_valid = true;
      xSemaphoreGive(dataMutex);
    }
    if ((int32_t)(xTaskGetTickCount() - next_save) >= 0) {
      time_t now = time(nullptr);
      save_last_epoch(now);
      next_save = xTaskGetTickCount() + pdMS_TO_TICKS(15000);
    }
    vTaskDelayUntil(&last, period);
  }
}

extern void imuTask(void *arg) {
  (void)arg;

  if (!imu_begin()) {
    vTaskSuspend(nullptr);
  }

  const TickType_t period = pdMS_TO_TICKS(10);  // ~100 Hz
  TickType_t last = xTaskGetTickCount();

  for (;;) {
    float gx, gy, gz;
    bool ok = false;

    if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
      ok = imu_read_gyro(gx, gy, gz);
      xSemaphoreGive(i2cMutex);
    }

    if (ok && xSemaphoreTake(dataMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
      gData.gx = gx; gData.gy = gy; gData.gz = gz; gData.imu_valid = true;
      xSemaphoreGive(dataMutex);
    }

    vTaskDelayUntil(&last, period);
  }
}
