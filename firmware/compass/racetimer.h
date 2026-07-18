#pragma once
#include <Arduino.h>

// レーススタートタイマー: 5:00 カウントダウン → 0 で自動的に経過時間カウントアップ。
// 通常のストップウォッチとしても使える。
namespace RaceTimer {
void tick();   // loop から毎回呼ぶ
void toggle(); // 開始 / 一時停止 / 再開
void sync();   // カウントダウン中: 残りを最寄りの分に丸める (スタート信号同期)
void reset();  // 停止して 5:00 に戻す
bool idle();
bool inCountdown();
bool running();
int32_t seconds(); // カウントダウン中: 残り秒 / 経過中: 経過秒
}
