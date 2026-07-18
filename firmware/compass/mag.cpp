#include "mag.h"
#include "config.h"
#include "settings.h"
#include <Wire.h>

namespace Mag {

// レジスタ構成は ROHM 公式 Arduino ライブラリ準拠
static const uint8_t ADDR = 0x0E;
static const uint8_t REG_WIA = 0x0F;   // = 0x41
static const uint8_t REG_DATAX = 0x10; // X/Y/Z 各 16bit LE
static const uint8_t REG_CNTL1 = 0x1B;
static const uint8_t REG_CNTL2 = 0x1C;
static const uint8_t REG_CNTL3 = 0x1D;
static const uint8_t REG_CNTL4 = 0x5C;

static int16_t rawX = 0, rawY = 0;
static uint32_t lastOkMs = 0;
static bool present = false;
static uint32_t lastPollMs = 0;

static bool calibrating = false;
static uint16_t calN = 0;
static int16_t minX, maxX, minY, maxY;

static void writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

static bool readRegs(uint8_t reg, uint8_t *buf, uint8_t len) {
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(ADDR, len) != len) return false;
  for (uint8_t i = 0; i < len; i++) buf[i] = Wire.read();
  return true;
}

bool begin() {
  uint8_t wia = 0;
  if (!readRegs(REG_WIA, &wia, 1) || wia != 0x41) return false;
  writeReg(REG_CNTL1, 0xC0); // PC1=1 | OUT_BIT=14bit | ODR=10Hz | 連続測定
  delay(2);
  writeReg(REG_CNTL4, 0x00); // CNTL4=0x0000 でリセット解除
  writeReg(REG_CNTL4 + 1, 0x00);
  writeReg(REG_CNTL2, 0x08); // DRDY 有効
  writeReg(REG_CNTL3, 0x40); // FORCE: 測定開始
  present = true;
  return true;
}

void poll() {
  if (!present) return;
  uint32_t now = millis();
  if (now - lastPollMs < 50) return;
  lastPollMs = now;

  uint8_t raw[6];
  if (!readRegs(REG_DATAX, raw, 6)) return;
  rawX = (int16_t)(raw[0] | (raw[1] << 8));
  rawY = (int16_t)(raw[2] | (raw[3] << 8));
  lastOkMs = now;

  if (calibrating) {
    if (calN == 0) { minX = maxX = rawX; minY = maxY = rawY; }
    if (rawX < minX) minX = rawX;
    if (rawX > maxX) maxX = rawX;
    if (rawY < minY) minY = rawY;
    if (rawY > maxY) maxY = rawY;
    if (calN < UINT16_MAX) calN++;
  }
}

bool healthy() { return present && (millis() - lastOkMs) < 1000; }

float magneticHeading() {
  float x = rawX - settings.magOffX;
  float y = rawY - settings.magOffY;
  // 軸の対応は取付向きに依存する。実機で北を向けて確認し、必要なら符号・軸を入れ替える。
  float h = atan2(-y, x) * 180.0f / PI;
  h += MAG_MOUNT_OFFSET_TENTHS / 10.0f;
  while (h < 0) h += 360.0f;
  while (h >= 360.0f) h -= 360.0f;
  return h;
}

void calStart() {
  calibrating = true;
  calN = 0;
}

void calStop(bool save) {
  calibrating = false;
  if (save && calN > 50) {
    settings.magOffX = (int16_t)(((int32_t)minX + maxX) / 2);
    settings.magOffY = (int16_t)(((int32_t)minY + maxY) / 2);
    SettingsStore::save();
  }
}

bool calActive() { return calibrating; }
uint16_t calCount() { return calN; }

} // namespace Mag
