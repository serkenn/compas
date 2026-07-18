#include "buttons.h"
#include "config.h"

namespace Buttons {

enum class Btn : uint8_t { None, Mode, A, B };

static const uint16_t DEBOUNCE_MS = 30;
static const uint16_t LONG_MS = 700;
static const uint16_t REPEAT_MS = 150;

static Btn stable = Btn::None;   // 確定状態
static Btn candidate = Btn::None;
static uint32_t candidateSince = 0;
static uint32_t pressStart = 0;
static uint32_t nextRepeat = 0;
static bool longFired = false;
static uint32_t lastSample = 0;

static Btn read() {
  int v = analogRead(PIN_BTN);
  if (v > BTN_TH_MODE) return Btn::Mode;
  if (v > BTN_TH_A_LO && v <= BTN_TH_A_HI) return Btn::A;
  if (v > BTN_TH_B_LO && v <= BTN_TH_B_HI) return Btn::B;
  return Btn::None;
}

void begin() {}

BtnEvent poll() {
  uint32_t now = millis();
  if (now - lastSample < 10) return BtnEvent::None;
  lastSample = now;

  Btn raw = read();
  if (raw != candidate) {
    candidate = raw;
    candidateSince = now;
    return BtnEvent::None;
  }
  if (candidate == stable) {
    // 押しっぱなし: 長押し / リピート
    if (stable != Btn::None) {
      if (!longFired && now - pressStart >= LONG_MS) {
        longFired = true;
        nextRepeat = now + REPEAT_MS;
        switch (stable) {
          case Btn::Mode: return BtnEvent::ModeLong;
          case Btn::A: return BtnEvent::ALong;
          case Btn::B: return BtnEvent::BLong;
          default: break;
        }
      } else if (longFired && now >= nextRepeat) {
        nextRepeat = now + REPEAT_MS;
        if (stable == Btn::A) return BtnEvent::ARepeat;
        if (stable == Btn::B) return BtnEvent::BRepeat;
      }
    }
    return BtnEvent::None;
  }
  if (now - candidateSince < DEBOUNCE_MS) return BtnEvent::None;

  // 状態確定
  Btn prev = stable;
  stable = candidate;
  if (stable != Btn::None) { // 押下
    pressStart = now;
    longFired = false;
    return BtnEvent::None;
  }
  // 離した: 長押し済みでなければ短押しイベント
  if (!longFired) {
    switch (prev) {
      case Btn::Mode: return BtnEvent::ModeShort;
      case Btn::A: return BtnEvent::AShort;
      case Btn::B: return BtnEvent::BShort;
      default: break;
    }
  }
  return BtnEvent::None;
}

} // namespace Buttons
