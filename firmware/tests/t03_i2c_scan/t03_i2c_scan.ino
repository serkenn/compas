// t03: I2C バススキャン
// 期待値: 0x0E = BM1422AGMV(地磁気), 0x68 = DS1307(RTC)
// DS1307 モジュールに EEPROM が載っている場合は 0x50 台も見える。
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin();
}

void loop() {
  uint8_t found = 0;
  Serial.println(F("--- I2C scan ---"));
  for (uint8_t addr = 0x08; addr < 0x78; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("  found: 0x"));
      Serial.println(addr, HEX);
      found++;
    }
  }
  if (!found) Serial.println(F("  (none)"));
  delay(3000);
}
