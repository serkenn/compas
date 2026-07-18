#pragma once
#include <Arduino.h>

// BM1422AGMV 地磁気センサー (I2C 0x0E)。水平取付前提 (傾斜補正なし)。
namespace Mag {
bool begin();   // WHO_AM_I 不一致なら false
void poll();    // loop から毎回呼ぶ (内部 50ms 間隔)
bool healthy(); // 最近の読み取りが成功しているか
float magneticHeading(); // ハードアイアン・取付オフセット補正済みの磁方位 0-360

// キャリブレーション (水平でゆっくり 1〜2 回転させ min/max からオフセットを求める)
void calStart();
void calStop(bool save); // save=true でオフセット確定 + EEPROM 保存
bool calActive();
uint16_t calCount(); // 収集サンプル数 (UI 表示用)
}
