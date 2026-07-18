#pragma once
#include <Arduino.h>

// 7セグ 3 桁ダイナミック点灯 (TCB1 割り込み, 1ms/桁)
namespace Seg7 {
void begin();
void showNumber(uint16_t v); // 0-999 をゼロ埋め 3 桁で表示
void showDashes();           // "---" (方位不明)
void setBlink(bool on);      // 地磁気ソース時の周期ブリンク
}
