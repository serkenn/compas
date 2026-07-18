#pragma once
#include <Arduino.h>

// ---- ピンアサイン (docs/wiring.md §7 参照) ----
constexpr uint8_t PIN_PPS = 2; // 1PPS (反転バッファ経由, FALLING が秒頭)
constexpr uint8_t PIN_LCD_RS = 3, PIN_LCD_E = 4;
constexpr uint8_t PIN_LCD_D4 = 5, PIN_LCD_D5 = 6, PIN_LCD_D6 = 7, PIN_LCD_D7 = 8;
constexpr uint8_t PIN_DIG[3] = {9, 10, 11};                  // 桁選択 (H=ON)
constexpr uint8_t PIN_SEG[7] = {12, 13, A0, A1, A2, A3, A6}; // セグ a..g (H=点灯)
constexpr uint8_t PIN_BTN = A7;

// ---- ボタンラダーしきい値 (t08 の実測値で調整する) ----
constexpr int BTN_TH_MODE = 800;
constexpr int BTN_TH_A_LO = 430, BTN_TH_A_HI = 650;
constexpr int BTN_TH_B_LO = 250, BTN_TH_B_HI = 430;

// ---- 地磁気センサー取付方位オフセット deg*10 (艤装後に実測して調整) ----
constexpr int16_t MAG_MOUNT_OFFSET_TENTHS = 0;

// ---- GPS ----
constexpr uint32_t GPS_BAUD = 9600;
constexpr uint16_t GPS_STALE_MS = 3000; // これより古いデータは無効扱い
constexpr float LOG_MIN_SOG_KT = 0.5;   // 航程を積算する最低速度
