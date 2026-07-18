#pragma once
#include <Arduino.h>

struct ClockTime {
  uint16_t year;
  uint8_t month, day, hour, minute, second;
};

// 時刻管理: GPS 時刻を正とし DS1307 (UTC 保持) に同期。GPS 断では RTC で継続。
namespace Clock {
void begin();
void task(); // loop から毎回呼ぶ (GPS 有効時、起動直後と 1 時間毎に RTC を書く)
bool getLocal(ClockTime &t); // タイムゾーン適用済み。時刻不明なら false
bool sourceIsGps();          // 直近の時刻が GPS 由来か
}
