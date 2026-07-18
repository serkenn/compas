#pragma once
#include <Arduino.h>

// 方位ソース選択 (GPS COG / 地磁気) と円環スムージング
namespace Heading {
void update(); // 200ms 間隔で呼ぶ
bool valid();
uint16_t deg();    // 表示用 0-359 (設定に応じ真方位 or 磁方位)
bool usingGps();   // 現在採用中のソース
}
