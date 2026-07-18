#pragma once
#include <Arduino.h>

// GPS (GT-502MGG-N, Serial1) の受信・NMEA パースと航程ログ
namespace Nav {
void begin();
void poll(); // loop から毎回呼ぶ

bool hasFix();
uint8_t sats();
bool sogValid();
float sogKt();
bool cogValid();
float cogDeg(); // 真方位
bool posValid();
double lat();
double lng();
bool timeValid(); // 日付+時刻とも有効
void getUtc(uint16_t &y, uint8_t &mo, uint8_t &d, uint8_t &h, uint8_t &mi, uint8_t &s);
uint32_t ppsAgeMs(); // 最後の 1PPS からの経過 (無ければ大きい値)

// 航程ログ (電源断でリセット)
float tripNm();
float maxKt();
float avgKt();
void resetLog();
}
