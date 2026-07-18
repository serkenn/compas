#pragma once
#include <Arduino.h>

// 抵抗ラダー (A7) の 3 ボタン。短押し/長押し/長押しリピートをイベントで返す。
enum class BtnEvent : uint8_t {
  None,
  ModeShort, ModeLong,
  AShort, ALong, ARepeat,
  BShort, BLong, BRepeat,
};

namespace Buttons {
void begin();
BtnEvent poll(); // loop から毎回呼ぶ (内部で 10ms 間隔サンプリング)
}
