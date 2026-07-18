// t05: BM1422AGMV 地磁気センサー単体確認 (アドレス 0x0E)
// 生値 X/Y/Z と、水平前提の簡易方位(オフセット補正なし)を表示する。
// レジスタ構成は ROHM 公式 Arduino ライブラリ準拠。初回動作時にデータシートと突き合わせること。
#include <Wire.h>

const uint8_t MAG_ADDR = 0x0E;
const uint8_t REG_WIA = 0x0F;   // WHO_AM_I = 0x41
const uint8_t REG_DATAX = 0x10; // X/Y/Z 各 16bit LE
const uint8_t REG_CNTL1 = 0x1B;
const uint8_t REG_CNTL2 = 0x1C;
const uint8_t REG_CNTL3 = 0x1D;
const uint8_t REG_CNTL4 = 0x5C; // 16bit, 0x0000 を書いてリセット解除

void writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MAG_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

bool readRegs(uint8_t reg, uint8_t *buf, uint8_t len) {
  Wire.beginTransmission(MAG_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  Wire.requestFrom(MAG_ADDR, len);
  for (uint8_t i = 0; i < len; i++) buf[i] = Wire.read();
  return true;
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  uint8_t wia = 0;
  readRegs(REG_WIA, &wia, 1);
  Serial.print(F("WHO_AM_I = 0x"));
  Serial.println(wia, HEX);
  if (wia != 0x41) Serial.println(F("!! BM1422AGMV not found"));

  writeReg(REG_CNTL1, 0xC0); // PC1=1(動作) | OUT_BIT=14bit | ODR=10Hz | 連続測定
  delay(2);
  writeReg(REG_CNTL4, 0x00); // CNTL4 = 0x0000 (LSB, MSB の順)
  writeReg(REG_CNTL4 + 1, 0x00);
  writeReg(REG_CNTL2, 0x08); // DRDY 有効
  writeReg(REG_CNTL3, 0x40); // FORCE: 測定開始
}

void loop() {
  uint8_t raw[6];
  if (readRegs(REG_DATAX, raw, 6)) {
    int16_t x = (int16_t)(raw[0] | (raw[1] << 8));
    int16_t y = (int16_t)(raw[2] | (raw[3] << 8));
    int16_t z = (int16_t)(raw[4] | (raw[5] << 8));
    float heading = atan2((float)-y, (float)x) * 180.0 / PI; // 取付向きで要調整
    if (heading < 0) heading += 360.0;
    char buf[48];
    snprintf(buf, sizeof(buf), "X=%6d Y=%6d Z=%6d  HDG=%3d", x, y, z, (int)heading);
    Serial.println(buf);
  } else {
    Serial.println(F("read error"));
  }
  delay(200);
}
