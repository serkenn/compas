// t07: 7セグ 3 桁ダイナミック点灯確認
// TCB1 タイマ割り込み (1ms/桁 = 全体 333Hz) で走査し、000→359 を回して表示する。
// チラつき・ゴースト(前桁の残像)が無いことを確認する。

const uint8_t PIN_DIG[3] = {9, 10, 11};
const uint8_t PIN_SEG[7] = {12, 13, A0, A1, A2, A3, A6}; // a..g

const uint8_t FONT[10] = {
  0b0111111, 0b0000110, 0b1011011, 0b1001111, 0b1100110,
  0b1101101, 0b1111101, 0b0000111, 0b1111111, 0b1101111,
};

volatile uint8_t frame[3]; // 各桁のセグメントマスク

ISR(TCB1_INT_vect) {
  TCB1.INTFLAGS = TCB_CAPT_bm;
  static uint8_t cur = 0;
  digitalWrite(PIN_DIG[cur], LOW);          // 現桁 OFF (ゴースト防止のため先に桁を切る)
  cur = (cur + 1) % 3;
  uint8_t mask = frame[cur];
  for (uint8_t i = 0; i < 7; i++) digitalWrite(PIN_SEG[i], (mask >> i) & 1);
  digitalWrite(PIN_DIG[cur], HIGH);
}

void show3(uint16_t v) {
  uint8_t d0 = (v / 100) % 10, d1 = (v / 10) % 10, d2 = v % 10;
  noInterrupts();
  frame[0] = FONT[d0];
  frame[1] = FONT[d1];
  frame[2] = FONT[d2];
  interrupts();
}

void setup() {
  for (uint8_t i = 0; i < 3; i++) { pinMode(PIN_DIG[i], OUTPUT); digitalWrite(PIN_DIG[i], LOW); }
  for (uint8_t i = 0; i < 7; i++) { pinMode(PIN_SEG[i], OUTPUT); digitalWrite(PIN_SEG[i], LOW); }
  // TCB1: 16MHz/2 = 8MHz, CCMP=8000 → 1ms 周期
  TCB1.CTRLB = TCB_CNTMODE_INT_gc;
  TCB1.CCMP = 8000 - 1;
  TCB1.INTCTRL = TCB_CAPT_bm;
  TCB1.CTRLA = TCB_CLKSEL_CLKDIV2_gc | TCB_ENABLE_bm;
}

void loop() {
  static uint16_t deg = 0;
  show3(deg);
  deg = (deg + 1) % 360;
  delay(100);
}
