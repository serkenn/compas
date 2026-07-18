#include "rtclock.h"
#include "gpsnav.h"
#include "settings.h"
#include <Wire.h>

namespace Clock {

static const uint8_t DS1307 = 0x68;
static uint32_t lastSyncMs = 0;
static bool synced = false;
static bool lastWasGps = false;

static uint8_t bcd2dec(uint8_t v) { return (v >> 4) * 10 + (v & 0x0F); }
static uint8_t dec2bcd(uint8_t v) { return ((v / 10) << 4) | (v % 10); }

// --- 暦 ⇔ 通算秒 (2000-01-01 起点) ---
static const uint8_t DAYS_IN_MONTH[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static bool isLeap(uint16_t y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }

static uint32_t toEpoch(const ClockTime &t) {
  uint32_t days = 0;
  for (uint16_t y = 2000; y < t.year; y++) days += isLeap(y) ? 366 : 365;
  for (uint8_t m = 1; m < t.month; m++) {
    days += DAYS_IN_MONTH[m - 1];
    if (m == 2 && isLeap(t.year)) days++;
  }
  days += t.day - 1;
  return ((days * 24UL + t.hour) * 60 + t.minute) * 60 + t.second;
}

static void fromEpoch(uint32_t e, ClockTime &t) {
  t.second = e % 60; e /= 60;
  t.minute = e % 60; e /= 60;
  t.hour = e % 24;
  uint32_t days = e / 24;
  uint16_t y = 2000;
  for (;;) {
    uint16_t len = isLeap(y) ? 366 : 365;
    if (days < len) break;
    days -= len;
    y++;
  }
  t.year = y;
  uint8_t m = 1;
  for (;;) {
    uint8_t len = DAYS_IN_MONTH[m - 1] + ((m == 2 && isLeap(y)) ? 1 : 0);
    if (days < len) break;
    days -= len;
    m++;
  }
  t.month = m;
  t.day = days + 1;
}

// --- DS1307 (UTC 保持) ---
static bool rtcRead(ClockTime &t) {
  Wire.beginTransmission(DS1307);
  Wire.write((uint8_t)0x00);
  if (Wire.endTransmission() != 0) return false;
  if (Wire.requestFrom(DS1307, (size_t)7) != 7) return false;
  uint8_t sec = Wire.read();
  if (sec & 0x80) return false; // 発振停止 = 未設定
  t.second = bcd2dec(sec & 0x7F);
  t.minute = bcd2dec(Wire.read());
  t.hour = bcd2dec(Wire.read() & 0x3F);
  Wire.read(); // 曜日は未使用
  t.day = bcd2dec(Wire.read());
  t.month = bcd2dec(Wire.read());
  t.year = 2000 + bcd2dec(Wire.read());
  return t.month >= 1 && t.month <= 12 && t.day >= 1 && t.day <= 31;
}

static void rtcWrite(const ClockTime &t) {
  Wire.beginTransmission(DS1307);
  Wire.write((uint8_t)0x00);
  Wire.write(dec2bcd(t.second)); // CH=0 で発振開始
  Wire.write(dec2bcd(t.minute));
  Wire.write(dec2bcd(t.hour));
  Wire.write(dec2bcd(1)); // 曜日 (未使用)
  Wire.write(dec2bcd(t.day));
  Wire.write(dec2bcd(t.month));
  Wire.write(dec2bcd(t.year % 100));
  Wire.endTransmission();
}

void begin() {}

void task() {
  if (!Nav::timeValid()) return;
  uint32_t now = millis();
  if (synced && now - lastSyncMs < 3600000UL) return;
  ClockTime utc;
  Nav::getUtc(utc.year, utc.month, utc.day, utc.hour, utc.minute, utc.second);
  rtcWrite(utc);
  synced = true;
  lastSyncMs = now;
}

bool getLocal(ClockTime &t) {
  ClockTime utc;
  if (Nav::timeValid()) {
    Nav::getUtc(utc.year, utc.month, utc.day, utc.hour, utc.minute, utc.second);
    lastWasGps = true;
  } else if (rtcRead(utc)) {
    lastWasGps = false;
  } else {
    return false;
  }
  fromEpoch(toEpoch(utc) + (int32_t)settings.tzMinutes * 60, t);
  return true;
}

bool sourceIsGps() { return lastWasGps; }

} // namespace Clock
