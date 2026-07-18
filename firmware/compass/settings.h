#pragma once
#include <Arduino.h>

// EEPROM に保存するユーザー設定
struct Settings {
  uint8_t magic;   // 0x5A
  uint8_t version; // 1
  uint8_t source;            // 方位ソース: 0=AUTO 1=GPS 2=MAG
  uint8_t sogThTenths;       // AUTO 時に COG を採用する最低 SOG (kt*10)
  int16_t variationTenths;   // 磁気偏差 deg*10 (東+ / 西-)
  uint8_t dispMagnetic;      // 表示基準: 0=真方位 1=磁方位
  int16_t tzMinutes;         // UTC からのオフセット (分)
  int16_t magOffX, magOffY;  // ハードアイアン補正 (生値 LSB)
};

enum : uint8_t { SRC_AUTO = 0, SRC_GPS = 1, SRC_MAG = 2 };

extern Settings settings;

namespace SettingsStore {
void load(); // 不正なら既定値 (AUTO / 1.0kt / 西偏7.5° / 真方位 / JST)
void save();
}
