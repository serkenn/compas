#include "seg7.h"
#include "config.h"

namespace Seg7 {

// bit0=a ... bit6=g
static const uint8_t FONT[10] = {
  0b0111111, 0b0000110, 0b1011011, 0b1001111, 0b1100110,
  0b1101101, 0b1111101, 0b0000111, 0b1111111, 0b1101111,
};
static const uint8_t MASK_DASH = 0b1000000; // g のみ

static volatile uint8_t frame[3];
static volatile bool blink = false;

void begin() {
  for (uint8_t i = 0; i < 3; i++) { pinMode(PIN_DIG[i], OUTPUT); digitalWrite(PIN_DIG[i], LOW); }
  for (uint8_t i = 0; i < 7; i++) { pinMode(PIN_SEG[i], OUTPUT); digitalWrite(PIN_SEG[i], LOW); }
  showDashes();
  // TCB1: 16MHz/2 = 8MHz, CCMP=8000 → 1ms 周期
  TCB1.CTRLB = TCB_CNTMODE_INT_gc;
  TCB1.CCMP = 8000 - 1;
  TCB1.INTCTRL = TCB_CAPT_bm;
  TCB1.CTRLA = TCB_CLKSEL_CLKDIV2_gc | TCB_ENABLE_bm;
}

void showNumber(uint16_t v) {
  if (v > 999) v = 999;
  uint8_t f0 = FONT[(v / 100) % 10];
  uint8_t f1 = FONT[(v / 10) % 10];
  uint8_t f2 = FONT[v % 10];
  noInterrupts();
  frame[0] = f0; frame[1] = f1; frame[2] = f2;
  interrupts();
}

void showDashes() {
  noInterrupts();
  frame[0] = frame[1] = frame[2] = MASK_DASH;
  interrupts();
}

void setBlink(bool on) { blink = on; }

} // namespace Seg7

ISR(TCB1_INT_vect) {
  using namespace Seg7;
  TCB1.INTFLAGS = TCB_CAPT_bm;
  static uint8_t cur = 0;
  digitalWrite(PIN_DIG[cur], LOW); // ゴースト防止: 先に桁を切る
  cur = (cur + 1) % 3;
  // MAG ソース時は 1.6 秒周期で 200ms 消灯して知らせる
  bool blank = blink && (millis() % 1600) < 200;
  uint8_t mask = blank ? 0 : frame[cur];
  for (uint8_t i = 0; i < 7; i++) digitalWrite(PIN_SEG[i], (mask >> i) & 1);
  digitalWrite(PIN_DIG[cur], HIGH);
}
