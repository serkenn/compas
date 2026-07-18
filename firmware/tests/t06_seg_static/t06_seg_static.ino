// t06: 7セグ静的点灯確認 (まず 1 桁で配線確認)
// DIG1(D9) だけ ON にして、a→g を順に点灯 → その後 0〜9 をカウント表示。
// セグメント電流・電圧の実測はこのスケッチで行う (期待値 ≈12mA/セグ)。

const uint8_t PIN_DIG[3] = {9, 10, 11};
const uint8_t PIN_SEG[7] = {12, 13, A0, A1, A2, A3, A6}; // a,b,c,d,e,f,g

// bit0=a ... bit6=g
const uint8_t FONT[10] = {
  0b0111111, 0b0000110, 0b1011011, 0b1001111, 0b1100110,
  0b1101101, 0b1111101, 0b0000111, 0b1111111, 0b1101111,
};

void showMask(uint8_t mask) {
  for (uint8_t i = 0; i < 7; i++) digitalWrite(PIN_SEG[i], (mask >> i) & 1);
}

void setup() {
  for (uint8_t i = 0; i < 3; i++) { pinMode(PIN_DIG[i], OUTPUT); digitalWrite(PIN_DIG[i], LOW); }
  for (uint8_t i = 0; i < 7; i++) { pinMode(PIN_SEG[i], OUTPUT); digitalWrite(PIN_SEG[i], LOW); }
  digitalWrite(PIN_DIG[0], HIGH); // DIG1 のみ ON

  // セグメント単体を 1 秒ずつ (a,b,c,...,g)
  for (uint8_t i = 0; i < 7; i++) {
    showMask(1 << i);
    delay(1000);
  }
}

void loop() {
  for (uint8_t n = 0; n < 10; n++) {
    showMask(FONT[n]);
    delay(700);
  }
}
