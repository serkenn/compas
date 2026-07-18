// t04: DS1307 RTC 単体確認 (レジスタ直叩き, アドレス 0x68)
// 停止していれば 2026-01-01 00:00:00 をセットして走らせ、毎秒時刻を表示する。
#include <Wire.h>

const uint8_t DS1307 = 0x68;

uint8_t bcd2dec(uint8_t v) { return (v >> 4) * 10 + (v & 0x0F); }
uint8_t dec2bcd(uint8_t v) { return ((v / 10) << 4) | (v % 10); }

void writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(DS1307);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  // 秒レジスタの CH ビット(bit7)=1 なら発振停止中 → 初期時刻を書いて始動
  Wire.beginTransmission(DS1307);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(DS1307, (size_t)1);
  uint8_t sec = Wire.read();
  if (sec & 0x80) {
    Serial.println(F("RTC stopped -> set 2026-01-01 00:00:00"));
    Wire.beginTransmission(DS1307);
    Wire.write(0x00);
    Wire.write(dec2bcd(0));  // sec (CH=0)
    Wire.write(dec2bcd(0));  // min
    Wire.write(dec2bcd(0));  // hour (24h)
    Wire.write(dec2bcd(4));  // 曜日 (任意)
    Wire.write(dec2bcd(1));  // day
    Wire.write(dec2bcd(1));  // month
    Wire.write(dec2bcd(26)); // year
    Wire.endTransmission();
  }
}

void loop() {
  Wire.beginTransmission(DS1307);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(DS1307, (size_t)7);
  uint8_t s = bcd2dec(Wire.read() & 0x7F);
  uint8_t mi = bcd2dec(Wire.read());
  uint8_t h = bcd2dec(Wire.read() & 0x3F);
  Wire.read(); // 曜日
  uint8_t d = bcd2dec(Wire.read());
  uint8_t mo = bcd2dec(Wire.read());
  uint8_t y = bcd2dec(Wire.read());
  char buf[24];
  snprintf(buf, sizeof(buf), "20%02u-%02u-%02u %02u:%02u:%02u", y, mo, d, h, mi, s);
  Serial.println(buf);
  delay(1000);
}
